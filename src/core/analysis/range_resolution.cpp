#include "range_resolution.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace SAR {
namespace Analysis {

// 光速常量（米/秒）
constexpr double SPEED_OF_LIGHT = 299792458.0;

RangeResolution::RangeResolution() 
    : lastRangeResolution(0.0), wavelength(0.03), bandwidth(100e6), hasParameters(false) {
}

double RangeResolution::calculateRangeResolution(const cv::Mat& image) {
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

    if (hasParameters) {
        // 如果已经设置了波长和带宽参数，使用理论公式计算
        lastRangeResolution = computeTheoreticalRangeResolution();
    } else {
        // 否则使用图像估计分辨率
        lastRangeResolution = estimateResolutionFromImage(floatImage);
    }

    return lastRangeResolution;
}

double RangeResolution::calculateRangeResolutionInROI(const cv::Mat& image, const cv::Rect& roi) {
    if (image.empty() || 
        roi.x < 0 || roi.y < 0 || 
        roi.x + roi.width > image.cols || 
        roi.y + roi.height > image.rows) {
        return 0.0;
    }

    // 将 ROI 区域提取出来
    cv::Mat roiImage = image(roi).clone();
    
    // 计算 ROI 区域的距离分辨率
    return calculateRangeResolution(roiImage);
}

void RangeResolution::setWavelength(double wavelength) {
    if (wavelength > 0) {
        this->wavelength = wavelength;
        hasParameters = (this->bandwidth > 0);
    }
}

void RangeResolution::setBandwidth(double bandwidth) {
    if (bandwidth > 0) {
        this->bandwidth = bandwidth;
        hasParameters = (this->wavelength > 0);
    }
}

QString RangeResolution::getResultDescription() const {
    return QString("距离分辨率：%1 米")
        .arg(lastRangeResolution, 0, 'f', 3);
}

double RangeResolution::computeTheoreticalRangeResolution() {
    // 距离分辨率计算公式：δr = c / (2 * B)
    // 其中 c 是光速，B 是带宽
    return SPEED_OF_LIGHT / (2.0 * bandwidth);
}

double RangeResolution::estimateResolutionFromImage(const cv::Mat& image) {
    // 计算距离配置文件
    cv::Mat rangeProfile = computeRangeProfile(image);
    
    // 找到主瓣的半功率点
    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(rangeProfile, nullptr, &maxVal, nullptr, &maxLoc);
    
    double halfPowerLevel = maxVal / 2.0;
    
    // 在主瓣左侧寻找最接近半功率的点
    int leftIndex = maxLoc.x;
    while (leftIndex > 0 && rangeProfile.at<float>(leftIndex) > halfPowerLevel) {
        leftIndex--;
    }
    
    // 在主瓣右侧寻找最接近半功率的点
    int rightIndex = maxLoc.x;
    while (rightIndex < rangeProfile.cols - 1 && rangeProfile.at<float>(rightIndex) > halfPowerLevel) {
        rightIndex++;
    }
    
    // 计算 3dB 带宽（像素数）
    double resolution3dB = rightIndex - leftIndex;
    
    // 假设图像的一个像素对应的距离为 0.5 米（根据实际情况可调整）
    double pixelToMeter = 0.5;
    
    return resolution3dB * pixelToMeter;
}

cv::Mat RangeResolution::computeRangeProfile(const cv::Mat& image) {
    // 根据距离方向（假设为行方向）计算平均剖面
    cv::Mat rangeProfile;
    cv::reduce(image, rangeProfile, 0, cv::REDUCE_AVG, CV_32F);
    
    // 对范围剖面应用窗函数（汉明窗）减少旁瓣
    int width = rangeProfile.cols;
    for (int i = 0; i < width; i++) {
        float windowCoeff = 0.54f - 0.46f * cos(2.0f * CV_PI * i / (width - 1));
        rangeProfile.at<float>(i) *= windowCoeff;
    }
    
    return rangeProfile;
}

} // namespace Analysis
} // namespace SAR 