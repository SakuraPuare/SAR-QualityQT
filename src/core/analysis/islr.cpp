#include "islr.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace SAR {
namespace Analysis {

ISLR::ISLR() : lastISLR(0.0) {
}

double ISLR::calculateISLR(const cv::Mat& image) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为32位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算点扩散函数(PSF)
    cv::Mat psf = calculatePointSpreadFunction(floatImage);

    // 提取主瓣
    cv::Point peakLocation;
    cv::Mat mainLobe = extractMainLobe(psf, peakLocation);

    // 计算积分旁瓣比
    lastISLR = computeISLRFromPSF(psf, mainLobe, peakLocation);

    return lastISLR;
}

double ISLR::calculateISLRInROI(const cv::Mat& image, const cv::Rect& roi) {
    if (image.empty() || 
        roi.x < 0 || roi.y < 0 || 
        roi.x + roi.width > image.cols || 
        roi.y + roi.height > image.rows) {
        return 0.0;
    }

    // 将ROI区域提取出来
    cv::Mat roiImage = image(roi).clone();
    
    // 计算ROI区域的ISLR
    return calculateISLR(roiImage);
}

QString ISLR::getResultDescription() const {
    return QString("积分旁瓣比(ISLR): %1 dB")
        .arg(lastISLR, 0, 'f', 2);
}

cv::Mat ISLR::calculatePointSpreadFunction(const cv::Mat& image) {
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

    // 对数变换增强动态范围
    psf += cv::Scalar::all(1);
    cv::log(psf, psf);

    // 归一化
    cv::normalize(psf, psf, 0, 1, cv::NORM_MINMAX);

    return psf;
}

cv::Mat ISLR::extractMainLobe(const cv::Mat& psf, cv::Point& peakLocation) {
    // 找到PSF的峰值位置
    double minVal, maxVal;
    cv::Point minLoc;
    cv::minMaxLoc(psf, &minVal, &maxVal, &minLoc, &peakLocation);

    // 确定主瓣区域（以峰值位置为中心，定义一定半径内的区域为主瓣）
    int mainLobeRadius = std::min(psf.rows, psf.cols) / 10; // 可根据实际情况调整
    
    // 创建与PSF大小相同的零矩阵
    cv::Mat mainLobe = cv::Mat::zeros(psf.size(), CV_32F);
    
    // 提取主瓣区域
    for (int y = std::max(0, peakLocation.y - mainLobeRadius); 
         y < std::min(psf.rows, peakLocation.y + mainLobeRadius); y++) {
        for (int x = std::max(0, peakLocation.x - mainLobeRadius); 
             x < std::min(psf.cols, peakLocation.x + mainLobeRadius); x++) {
            // 计算到峰值的距离
            double dist = std::sqrt(std::pow(x - peakLocation.x, 2) + std::pow(y - peakLocation.y, 2));
            
            // 如果在主瓣半径内，则复制PSF的值
            if (dist <= mainLobeRadius) {
                mainLobe.at<float>(y, x) = psf.at<float>(y, x);
            }
        }
    }
    
    return mainLobe;
}

double ISLR::computeISLRFromPSF(const cv::Mat& psf, const cv::Mat& mainLobe, const cv::Point& peakLocation) {
    // 计算总能量
    double totalEnergy = 0.0;
    for (int y = 0; y < psf.rows; y++) {
        for (int x = 0; x < psf.cols; x++) {
            totalEnergy += std::pow(psf.at<float>(y, x), 2);
        }
    }
    
    // 计算主瓣能量
    double mainLobeEnergy = 0.0;
    for (int y = 0; y < mainLobe.rows; y++) {
        for (int x = 0; x < mainLobe.cols; x++) {
            mainLobeEnergy += std::pow(mainLobe.at<float>(y, x), 2);
        }
    }
    
    // 计算旁瓣能量
    double sideLobeEnergy = totalEnergy - mainLobeEnergy;
    
    // 计算积分旁瓣比并转换为dB
    if (mainLobeEnergy > 1e-10) {
        return 10.0 * std::log10(sideLobeEnergy / mainLobeEnergy);
    } else {
        return 0.0;
    }
}

} // namespace Analysis
} // namespace SAR 