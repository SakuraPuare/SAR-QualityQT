#include "snr.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace SAR {
namespace Analysis {

SNR::SNR() : lastSNR(0.0), lastNoiseLevel(0.0) {
}

double SNR::calculateSNR(const cv::Mat& image) {
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
    return lastSNR;
}

double SNR::calculateSNRWithROI(const cv::Mat& image, const cv::Rect& signalROI, const cv::Rect& noiseROI) {
    if (image.empty() || 
        signalROI.x < 0 || signalROI.y < 0 || 
        signalROI.x + signalROI.width > image.cols || 
        signalROI.y + signalROI.height > image.rows || 
        noiseROI.x < 0 || noiseROI.y < 0 || 
        noiseROI.x + noiseROI.width > image.cols || 
        noiseROI.y + noiseROI.height > image.rows) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

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
    return lastSNR;
}

double SNR::estimateNoiseLevel(const cv::Mat& image) {
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

    // 使用高斯滤波得到平滑图像
    cv::Mat smoothed;
    cv::GaussianBlur(floatImage, smoothed, cv::Size(5, 5), 1.5);

    // 噪声是原图与平滑图像的差异
    cv::Mat noise = floatImage - smoothed;

    // 计算噪声标准差
    cv::Scalar mean, stdDev;
    cv::meanStdDev(noise, mean, stdDev);

    lastNoiseLevel = stdDev[0];
    return lastNoiseLevel;
}

QString SNR::getResultDescription() const {
    return QString("信噪比：%1 dB, 噪声水平：%2")
        .arg(lastSNR, 0, 'f', 2)
        .arg(lastNoiseLevel, 0, 'f', 2);
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