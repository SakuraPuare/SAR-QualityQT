#include "pslr.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace SAR {
namespace Analysis {

PSLR::PSLR() : lastPSLR(0.0) {
}

double PSLR::calculatePSLR(const cv::Mat& image) {
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

    // 计算点扩散函数 (PSF)
    cv::Mat psf = calculatePointSpreadFunction(floatImage);

    // 找到主瓣峰值位置
    cv::Point mainPeakLoc = findPeakLocation(psf);

    // 找到最大旁瓣峰值位置
    cv::Point sidelobePeakLoc = findMaxSidelobePeak(psf, mainPeakLoc);

    // 计算峰值旁瓣比
    lastPSLR = computePSLRFromPSF(psf, mainPeakLoc, sidelobePeakLoc);

    return lastPSLR;
}

double PSLR::calculatePSLRInROI(const cv::Mat& image, const cv::Rect& roi) {
    if (image.empty() || 
        roi.x < 0 || roi.y < 0 || 
        roi.x + roi.width > image.cols || 
        roi.y + roi.height > image.rows) {
        return 0.0;
    }

    // 将 ROI 区域提取出来
    cv::Mat roiImage = image(roi).clone();
    
    // 计算 ROI 区域的 PSLR
    return calculatePSLR(roiImage);
}

QString PSLR::getResultDescription() const {
    return QString("峰值旁瓣比 (PSLR): %1 dB")
        .arg(lastPSLR, 0, 'f', 2);
}

cv::Mat PSLR::calculatePointSpreadFunction(const cv::Mat& image) {
    // 使用傅里叶变换计算点扩散函数
    cv::Mat padded;
    int m = cv::getOptimalDFTSize(image.rows);
    int n = cv::getOptimalDFTSize(image.cols);
    cv::copyMakeBorder(image, padded, 0, m - image.rows, 0, n - image.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    // 创建实部和虚部
    cv::Mat planes[] = {padded, cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexImage;
    cv::merge(planes, 2, complexImage);

    // 执行正向傅里叶变换
    cv::dft(complexImage, complexImage);

    // 计算幅度谱
    cv::split(complexImage, planes);
    cv::magnitude(planes[0], planes[1], planes[0]);
    cv::Mat psf = planes[0];

    // 对数变换增强动态范围（可选）
    psf += cv::Scalar::all(1);
    cv::log(psf, psf);

    // 归一化
    cv::normalize(psf, psf, 0, 1, cv::NORM_MINMAX);

    return psf;
}

cv::Point PSLR::findPeakLocation(const cv::Mat& psf) {
    // 找到 PSF 的峰值位置
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(psf, &minVal, &maxVal, &minLoc, &maxLoc);
    
    return maxLoc;
}

cv::Point PSLR::findMaxSidelobePeak(const cv::Mat& psf, const cv::Point& mainPeakLoc) {
    // 创建主瓣掩码
    cv::Mat mainLobeMask = cv::Mat::zeros(psf.size(), CV_8U);
    
    // 定义主瓣区域（以峰值位置为中心，定义一定半径内的区域为主瓣）
    int mainLobeRadius = std::min(psf.rows, psf.cols) / 10; // 可根据实际情况调整
    
    // 创建主瓣掩码
    cv::circle(mainLobeMask, mainPeakLoc, mainLobeRadius, cv::Scalar(255), -1);
    
    // 创建 PSF 的副本并掩盖主瓣区域
    cv::Mat psfWithoutMainLobe = psf.clone();
    psfWithoutMainLobe.setTo(0, mainLobeMask);
    
    // 查找旁瓣最大值位置
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(psfWithoutMainLobe, &minVal, &maxVal, &minLoc, &maxLoc);
    
    return maxLoc;
}

double PSLR::computePSLRFromPSF(const cv::Mat& psf, const cv::Point& mainPeakLoc, const cv::Point& sidelobePeakLoc) {
    // 获取主瓣峰值
    float mainPeakValue = psf.at<float>(mainPeakLoc.y, mainPeakLoc.x);
    
    // 获取最大旁瓣峰值
    float sidelobePeakValue = psf.at<float>(sidelobePeakLoc.y, sidelobePeakLoc.x);
    
    // 计算峰值旁瓣比并转换为 dB
    if (mainPeakValue > 1e-10) {
        return 20.0 * std::log10(sidelobePeakValue / mainPeakValue);
    } else {
        return 0.0;
    }
}

} // namespace Analysis
} // namespace SAR 