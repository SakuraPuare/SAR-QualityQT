#include "aasr.h"
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <numeric>

namespace SAR {
namespace Analysis {

AASR::AASR() : lastAASR(0.0) {
}

double AASR::calculateAASR(const cv::Mat& image, double dopplerCenterFreq, double processingBandwidth,
                          const std::vector<double>& antennaGain) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算AASR
    lastAASR = calculateAASRValue(floatImage, dopplerCenterFreq, processingBandwidth, antennaGain);

    return lastAASR;
}

double AASR::calculateAASRInROI(const cv::Mat& image, const cv::Rect& roi, 
                               double dopplerCenterFreq, double processingBandwidth,
                               const std::vector<double>& antennaGain) {
    if (image.empty() || 
        roi.x < 0 || roi.y < 0 || 
        roi.x + roi.width > image.cols || 
        roi.y + roi.height > image.rows) {
        return 0.0;
    }

    // 将 ROI 区域提取出来
    cv::Mat roiImage = image(roi).clone();
    
    // 计算 ROI 区域的 AASR
    return calculateAASR(roiImage, dopplerCenterFreq, processingBandwidth, antennaGain);
}

QString AASR::getResultDescription() const {
    return QString("方位模糊度 (AASR): %1")
        .arg(lastAASR, 0, 'f', 4);
}

double AASR::calculateAASRValue(const cv::Mat& image, double dopplerCenterFreq, double processingBandwidth,
                               const std::vector<double>& antennaGain) {
    // 实现公式AASR计算
    // AASR = ∑ ∫[fd-BP/2+n*PRF, fd+BP/2+n*PRF] G^2(f) df / ∫[fd-BP/2, fd+BP/2] G^2(f) df
    // n ≠ 0
    
    // 由于实际的方位频谱和增益通常无法直接从单张图像获取，
    // 这里提供一个简化的实现，可根据实际情况调整
    
    // 估计方位频谱
    cv::Mat azimuthSpectrum = estimateAzimuthSpectrum(image);
    
    // 计算方位模糊度
    const int numAmbiguities = 3; // 考虑3个模糊区
    const int spectrumSize = azimuthSpectrum.cols;
    
    // 频率采样间隔
    double freqStep = 2.0 * processingBandwidth / spectrumSize;
    
    // 计算处理带宽内的能量（分母）
    double mainLobeEnergy = 0.0;
    int startIdx = static_cast<int>((dopplerCenterFreq - 0.5 * processingBandwidth) / freqStep) + spectrumSize / 2;
    int endIdx = static_cast<int>((dopplerCenterFreq + 0.5 * processingBandwidth) / freqStep) + spectrumSize / 2;
    
    startIdx = std::max(0, std::min(startIdx, spectrumSize - 1));
    endIdx = std::max(0, std::min(endIdx, spectrumSize - 1));
    
    for (int i = startIdx; i <= endIdx; i++) {
        double freq = (i - spectrumSize / 2) * freqStep;
        double gain = simulateAntennaAzimuthGain(freq, antennaGain);
        mainLobeEnergy += azimuthSpectrum.at<float>(0, i) * gain * gain;
    }
    
    // 计算带宽外的模糊能量（分子）
    double ambiguousEnergy = 0.0;
    for (int n = -numAmbiguities; n <= numAmbiguities; n++) {
        if (n == 0) continue; // 跳过主瓣
        
        int ambStartIdx = startIdx + n * static_cast<int>(processingBandwidth / freqStep);
        int ambEndIdx = endIdx + n * static_cast<int>(processingBandwidth / freqStep);
        
        if (ambEndIdx < 0 || ambStartIdx >= spectrumSize) continue;
        
        ambStartIdx = std::max(0, ambStartIdx);
        ambEndIdx = std::min(ambEndIdx, spectrumSize - 1);
        
        for (int i = ambStartIdx; i <= ambEndIdx; i++) {
            double freq = (i - spectrumSize / 2) * freqStep;
            double gain = simulateAntennaAzimuthGain(freq, antennaGain);
            ambiguousEnergy += azimuthSpectrum.at<float>(0, i) * gain * gain;
        }
    }
    
    // 防止除零
    if (mainLobeEnergy < 1e-10) {
        return 0.0;
    }
    
    return ambiguousEnergy / mainLobeEnergy;
}

double AASR::simulateAntennaAzimuthGain(double dopplerFreq, const std::vector<double>& antennaGain) {
    // 如果提供了天线增益数据，则使用插值
    if (!antennaGain.empty()) {
        // 简单的线性插值
        int n = antennaGain.size();
        double freqMax = 1000.0; // 假设最大频率是1000Hz
        double freqStep = 2.0 * freqMax / (n - 1);
        int index = static_cast<int>((dopplerFreq + freqMax) / freqStep);
        
        if (index < 0) return antennaGain.front();
        if (index >= n - 1) return antennaGain.back();
        
        double t = (dopplerFreq + freqMax - index * freqStep) / freqStep;
        return antennaGain[index] * (1.0 - t) + antennaGain[index + 1] * t;
    }
    
    // 否则使用简化的高斯模型模拟天线增益模式
    // 假设方位增益随多普勒频率的偏移减小
    const double bandwidth = 200.0; // Hz
    return std::exp(-0.5 * std::pow(dopplerFreq, 2) / std::pow(bandwidth, 2));
}

cv::Mat AASR::estimateAzimuthSpectrum(const cv::Mat& image) {
    // 提取图像的方位向谱（沿行方向的一维FFT）
    int rows = image.rows;
    int cols = image.cols;
    
    // 输出频谱
    cv::Mat azimuthSpectrum = cv::Mat::zeros(1, cols, CV_32F);
    
    // 对每一行进行FFT
    for (int y = 0; y < rows; y++) {
        cv::Mat row = image.row(y).clone();
        
        // 添加零填充（可选）
        cv::Mat padded;
        int paddedSize = cv::getOptimalDFTSize(cols);
        cv::copyMakeBorder(row, padded, 0, 0, 0, paddedSize - cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
        
        // 创建复数矩阵
        cv::Mat planes[] = {padded, cv::Mat::zeros(padded.size(), CV_32F)};
        cv::Mat complexRow;
        cv::merge(planes, 2, complexRow);
        
        // 执行FFT
        cv::dft(complexRow, complexRow);
        
        // 计算幅度谱
        cv::split(complexRow, planes);
        cv::magnitude(planes[0], planes[1], planes[0]);
        cv::Mat rowSpectrum = planes[0];
        
        // 累加到总体谱中
        for (int x = 0; x < cols; x++) {
            azimuthSpectrum.at<float>(0, x) += rowSpectrum.at<float>(0, x);
        }
    }
    
    // 归一化
    cv::normalize(azimuthSpectrum, azimuthSpectrum, 0, 1, cv::NORM_MINMAX);
    
    return azimuthSpectrum;
}

} // namespace Analysis
} // namespace SAR 