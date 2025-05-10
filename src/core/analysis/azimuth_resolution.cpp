#include "azimuth_resolution.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace SAR {
namespace Analysis {

AzimuthResolution::AzimuthResolution() 
    : lastAzimuthResolution(0.0), wavelength(0.03), antennaLength(10.0), hasParameters(false) {
}

double AzimuthResolution::calculateAzimuthResolution(const cv::Mat& image) {
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
        // 如果已经设置了波长和天线长度参数，使用理论公式计算
        lastAzimuthResolution = computeTheoreticalAzimuthResolution();
    } else {
        // 否则使用图像估计分辨率
        lastAzimuthResolution = estimateResolutionFromImage(floatImage);
    }

    return lastAzimuthResolution;
}

double AzimuthResolution::calculateAzimuthResolutionInROI(const cv::Mat& image, const cv::Rect& roi) {
    if (image.empty() || 
        roi.x < 0 || roi.y < 0 || 
        roi.x + roi.width > image.cols || 
        roi.y + roi.height > image.rows) {
        return 0.0;
    }

    // 将 ROI 区域提取出来
    cv::Mat roiImage = image(roi).clone();
    
    // 计算 ROI 区域的方位分辨率
    return calculateAzimuthResolution(roiImage);
}

void AzimuthResolution::setWavelength(double wavelength) {
    if (wavelength > 0) {
        this->wavelength = wavelength;
        hasParameters = (this->antennaLength > 0);
    }
}

void AzimuthResolution::setAntennaLength(double antennaLength) {
    if (antennaLength > 0) {
        this->antennaLength = antennaLength;
        hasParameters = (this->wavelength > 0);
    }
}

QString AzimuthResolution::getResultDescription() const {
    return QString("方位分辨率：%1 米")
        .arg(lastAzimuthResolution, 0, 'f', 3);
}

double AzimuthResolution::computeTheoreticalAzimuthResolution() {
    // 合成孔径雷达方位分辨率计算公式：δa = L/2
    // 其中 L 是天线长度
    // 对于实际的条带式 SAR, 方位分辨率δa = L/2，而 L = λ/θ，其中θ是方位向波束宽度
    // 一般 SAR 方位分辨率约为天线长度的一半
    return antennaLength / 2.0;
}

double AzimuthResolution::estimateResolutionFromImage(const cv::Mat& image) {
    // 计算方位剖面
    cv::Mat azimuthProfile = computeAzimuthProfile(image);
    
    // 找到主瓣的半功率点
    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(azimuthProfile, nullptr, &maxVal, nullptr, &maxLoc);
    
    double halfPowerLevel = maxVal / 2.0;
    
    // 在主瓣左侧寻找最接近半功率的点
    int leftIndex = maxLoc.y;
    while (leftIndex > 0 && azimuthProfile.at<float>(leftIndex) > halfPowerLevel) {
        leftIndex--;
    }
    
    // 在主瓣右侧寻找最接近半功率的点
    int rightIndex = maxLoc.y;
    while (rightIndex < azimuthProfile.rows - 1 && azimuthProfile.at<float>(rightIndex) > halfPowerLevel) {
        rightIndex++;
    }
    
    // 计算 3dB 带宽（像素数）
    double resolution3dB = rightIndex - leftIndex;
    
    // 假设图像的一个像素对应的距离为 0.5 米（根据实际情况可调整）
    double pixelToMeter = 0.5;
    
    return resolution3dB * pixelToMeter;
}

cv::Mat AzimuthResolution::computeAzimuthProfile(const cv::Mat& image) {
    // 根据方位方向（假设为列方向）计算平均剖面
    cv::Mat azimuthProfile;
    cv::reduce(image, azimuthProfile, 1, cv::REDUCE_AVG, CV_32F);
    
    // 对方位剖面应用窗函数（汉明窗）减少旁瓣
    int height = azimuthProfile.rows;
    for (int i = 0; i < height; i++) {
        float windowCoeff = 0.54f - 0.46f * cos(2.0f * CV_PI * i / (height - 1));
        azimuthProfile.at<float>(i) *= windowCoeff;
    }
    
    return azimuthProfile;
}

} // namespace Analysis
} // namespace SAR 