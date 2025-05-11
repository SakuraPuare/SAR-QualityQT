#include "snr.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace SAR {
namespace Analysis {

SNR::SNR() : lastSNR(0.0), lastNoiseLevel(0.0) {
}

double SNR::calculateSNR(const cv::Mat& image) {
    if (!validateImage(image)) {
        lastResult.isSuccess = false;
        lastResult.errorMessage = "输入图像无效";
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage = convertToFloat(image);

    // 估计信号和噪声成分
    cv::Mat signalComponent = estimateSignalComponent(floatImage);
    cv::Mat noiseComponent = estimateNoiseComponent(floatImage);

    // 计算信号能量
    cv::Scalar meanSignal = cv::mean(signalComponent);
    cv::Scalar stdSignal;
    cv::meanStdDev(signalComponent, cv::Scalar(), stdSignal);
    double signalPower = meanSignal[0] * meanSignal[0] + stdSignal[0] * stdSignal[0];

    // 计算噪声能量
    cv::Scalar stdNoise;
    cv::meanStdDev(noiseComponent, cv::Scalar(), stdNoise);
    double noisePower = stdNoise[0] * stdNoise[0];

    // 防止除以零
    if (noisePower < 1e-10) {
        noisePower = 1e-10;
    }

    // 计算信噪比并转换为分贝
    lastSNR = 10.0 * std::log10(signalPower / noisePower);
    
    // 更新结果
    lastResult.isSuccess = true;
    lastResult.numericValue = lastSNR;
    lastResult.unit = "dB";
    lastResult.additionalValues["信号功率"] = signalPower;
    lastResult.additionalValues["噪声功率"] = noisePower;
    lastResult.additionalInfo["分析方法"] = "高斯平滑差值法";
    
    log(QString("SNR计算完成：%1 dB").arg(lastSNR));
    return lastSNR;
}

double SNR::calculateSNRWithROI(const cv::Mat& image, const cv::Rect& signalROI, const cv::Rect& noiseROI) {
    // 验证图像和ROI
    if (!validateImage(image) || 
        !validateROI(image, signalROI) || 
        !validateROI(image, noiseROI)) {
        lastResult.isSuccess = false;
        lastResult.errorMessage = "输入图像或ROI无效";
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage = convertToFloat(image);

    // 提取信号和噪声区域
    cv::Mat signalRegion = floatImage(signalROI);
    cv::Mat noiseRegion = floatImage(noiseROI);

    // 计算信号能量
    cv::Scalar meanSignal = cv::mean(signalRegion);
    cv::Scalar stdSignal;
    cv::meanStdDev(signalRegion, cv::Scalar(), stdSignal);
    double signalPower = meanSignal[0] * meanSignal[0] + stdSignal[0] * stdSignal[0];

    // 计算噪声能量
    cv::Scalar meanNoise, stdNoise;
    cv::meanStdDev(noiseRegion, meanNoise, stdNoise);
    double noisePower = stdNoise[0] * stdNoise[0];

    // 防止除以零
    if (noisePower < 1e-10) {
        noisePower = 1e-10;
    }

    // 计算信噪比并转换为分贝
    lastSNR = 10.0 * std::log10(signalPower / noisePower);
    
    // 更新结果
    lastResult.isSuccess = true;
    lastResult.numericValue = lastSNR;
    lastResult.unit = "dB";
    lastResult.additionalValues["信号功率"] = signalPower;
    lastResult.additionalValues["噪声功率"] = noisePower;
    lastResult.additionalInfo["分析方法"] = "手动ROI法";
    lastResult.additionalInfo["信号ROI"] = QString("(%1,%2,%3,%4)").arg(signalROI.x).arg(signalROI.y).arg(signalROI.width).arg(signalROI.height);
    lastResult.additionalInfo["噪声ROI"] = QString("(%1,%2,%3,%4)").arg(noiseROI.x).arg(noiseROI.y).arg(noiseROI.width).arg(noiseROI.height);
    
    log(QString("ROI区域SNR计算完成：%1 dB").arg(lastSNR));
    return lastSNR;
}

double SNR::estimateNoiseLevel(const cv::Mat& image) {
    if (!validateImage(image)) {
        lastResult.isSuccess = false;
        lastResult.errorMessage = "输入图像无效";
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage = convertToFloat(image);

    // 使用高斯滤波得到平滑图像
    cv::Mat smoothed;
    cv::GaussianBlur(floatImage, smoothed, cv::Size(5, 5), 1.5);

    // 噪声是原图与平滑图像的差异
    cv::Mat noise = floatImage - smoothed;

    // 计算噪声标准差
    cv::Scalar mean, stdDev;
    cv::meanStdDev(noise, mean, stdDev);

    lastNoiseLevel = stdDev[0];
    
    // 更新结果
    lastResult.additionalValues["噪声水平"] = lastNoiseLevel;
    
    log(QString("噪声水平估计完成：%1").arg(lastNoiseLevel));
    return lastNoiseLevel;
}

AnalysisBase::Result SNR::getResult() const {
    return lastResult;
}

QString SNR::getResultDescription() const {
    if (!lastResult.isSuccess) {
        return "分析失败：" + lastResult.errorMessage;
    }
    
    return QString("信噪比：%1 %2, 噪声水平：%3")
        .arg(lastResult.numericValue, 0, 'f', 2)
        .arg(lastResult.unit)
        .arg(lastResult.additionalValues.count("噪声水平") ? 
             QString::number(lastResult.additionalValues.at("噪声水平"), 'f', 2) : "未计算");
}

AnalysisBase::Result SNR::analyze(const cv::Mat& image) {
    calculateSNR(image);
    return lastResult;
}

AnalysisBase::Result SNR::analyzeWithROI(const cv::Mat& image, const cv::Rect& roi) {
    // 使用ROI内部区域作为信号，ROI外部区域作为噪声
    cv::Rect imageRect(0, 0, image.cols, image.rows);
    cv::Rect noiseROI(0, 0, image.cols/4, image.rows/4); // 使用图像左上角1/4区域作为噪声区域
    
    // 确保ROI不与噪声区域重叠
    if (roi.x < noiseROI.x + noiseROI.width && roi.x + roi.width > noiseROI.x &&
        roi.y < noiseROI.y + noiseROI.height && roi.y + roi.height > noiseROI.y) {
        // 如果有重叠，使用右上角区域作为噪声区域
        noiseROI = cv::Rect(image.cols*3/4, 0, image.cols/4, image.rows/4);
    }
    
    calculateSNRWithROI(image, roi, noiseROI);
    return lastResult;
}

QString SNR::getMethodName() const {
    return "SNR";
}

QString SNR::getDescription() const {
    return "信噪比分析，评估图像中信号与噪声的比例，单位为dB";
}

cv::Mat SNR::estimateSignalComponent(const cv::Mat& image) {
    cv::Mat result;
    cv::GaussianBlur(image, result, cv::Size(5, 5), 1.5);
    return result;
}

cv::Mat SNR::estimateNoiseComponent(const cv::Mat& image) {
    cv::Mat smoothed;
    cv::GaussianBlur(image, smoothed, cv::Size(5, 5), 1.5);
    return image - smoothed;
}

} // namespace Analysis
} // namespace SAR 