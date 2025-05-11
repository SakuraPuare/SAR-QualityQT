#include "analysis_utils.h"
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <limits>
#include "../include/logger.h"

namespace SAR {
namespace Analysis {
namespace Utils {

ImageType detectImageType(const cv::Mat& image) {
    if (image.empty()) {
        return ImageType::Unknown;
    }
    
    if (image.channels() == 3) {
        return ImageType::RGB;
    }
    
    if (image.channels() == 2 && image.type() == CV_32FC2) {
        return ImageType::Complex;
    }
    
    if (image.channels() == 1) {
        // 假设强度图像值范围较大，幅度图像范围较小
        cv::Scalar mean, stddev;
        cv::meanStdDev(image, mean, stddev);
        
        if (mean[0] > 1000 || stddev[0] > 1000) {
            return ImageType::Intensity;
        } else {
            return ImageType::Amplitude;
        }
    }
    
    return ImageType::Unknown;
}

cv::Mat convertToFloat(const cv::Mat& image) {
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }
    return floatImage;
}

bool validateImage(const cv::Mat& image) {
    if (image.empty()) {
        LOG_ERROR("[Analysis Utils] 错误：输入图像为空");
        return false;
    }
    return true;
}

bool validateROI(const cv::Mat& image, const cv::Rect& roi) {
    if (!validateImage(image)) {
        return false;
    }
    
    if (roi.x < 0 || roi.y < 0 || 
        roi.x + roi.width > image.cols || 
        roi.y + roi.height > image.rows ||
        roi.width <= 0 || roi.height <= 0) {
        LOG_ERROR("[Analysis Utils] 错误：ROI区域无效");
        return false;
    }
    return true;
}

cv::Scalar calculateImageStats(const cv::Mat& image, const cv::Mat& mask) {
    cv::Scalar mean, stddev;
    cv::meanStdDev(image, mean, stddev, mask);
    
    double min, max;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(image, &min, &max, &minLoc, &maxLoc, mask);
    
    cv::Scalar result;
    result[0] = mean[0];     // 均值
    result[1] = stddev[0];   // 标准差
    result[2] = min;         // 最小值
    result[3] = max;         // 最大值
    
    return result;
}

double estimateNoiseLevel(const cv::Mat& image) {
    // 简单的噪声估计：使用中值滤波器的差异
    cv::Mat medianFiltered;
    cv::medianBlur(image, medianFiltered, 3);
    
    cv::Mat diff;
    cv::absdiff(image, medianFiltered, diff);
    
    cv::Scalar mean = cv::mean(diff);
    return mean[0] * 1.4826; // 中值绝对偏差到标准差的转换因子
}

double calculateRatioDB(const cv::Mat& image, const cv::Rect& region1, const cv::Rect& region2) {
    if (!validateROI(image, region1) || !validateROI(image, region2)) {
        return 0.0;
    }
    
    cv::Mat roi1 = image(region1);
    cv::Mat roi2 = image(region2);
    
    cv::Scalar mean1 = cv::mean(roi1);
    cv::Scalar mean2 = cv::mean(roi2);
    
    if (mean2[0] <= 0) {
        LOG_WARNING("[Analysis Utils] 警告：除数接近零，无法计算比率");
        return 0.0;
    }
    
    double ratio = mean1[0] / mean2[0];
    double ratioDB = 10.0 * std::log10(ratio);
    
    return ratioDB;
}

cv::Point findPeak(const cv::Mat& image) {
    cv::Point maxLoc;
    double minVal, maxVal;
    cv::Point minLoc;
    
    cv::minMaxLoc(image, &minVal, &maxVal, &minLoc, &maxLoc);
    return maxLoc;
}

cv::Mat calculateFFT(const cv::Mat& image, bool shift) {
    // 确保图像尺寸是2的幂
    int rows = cv::getOptimalDFTSize(image.rows);
    int cols = cv::getOptimalDFTSize(image.cols);
    
    cv::Mat padded;
    cv::copyMakeBorder(image, padded, 0, rows - image.rows, 0, cols - image.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    
    // 创建复数图像
    cv::Mat planes[] = {padded, cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complex;
    cv::merge(planes, 2, complex);
    
    // 执行傅立叶变换
    cv::dft(complex, complex);
    
    // 频移（可选）
    if (shift) {
        int cx = complex.cols / 2;
        int cy = complex.rows / 2;
        
        cv::Mat q0(complex, cv::Rect(0, 0, cx, cy));    // 左上角
        cv::Mat q1(complex, cv::Rect(cx, 0, cx, cy));   // 右上角
        cv::Mat q2(complex, cv::Rect(0, cy, cx, cy));   // 左下角
        cv::Mat q3(complex, cv::Rect(cx, cy, cx, cy));  // 右下角
        
        cv::Mat tmp;
        q0.copyTo(tmp);
        q3.copyTo(q0);
        tmp.copyTo(q3);
        
        q1.copyTo(tmp);
        q2.copyTo(q1);
        tmp.copyTo(q2);
    }
    
    return complex;
}

cv::Mat calculatePowerSpectrum(const cv::Mat& image) {
    // 计算FFT
    cv::Mat complex = calculateFFT(image, true);
    
    // 分离实部和虚部
    cv::Mat planes[2];
    cv::split(complex, planes);
    
    // 计算幅度谱
    cv::Mat magnitude;
    cv::magnitude(planes[0], planes[1], magnitude);
    
    // 计算功率谱（幅度的平方）
    cv::multiply(magnitude, magnitude, magnitude);
    
    // 对数变换以增强显示效果
    cv::Mat logMagnitude;
    cv::log(magnitude + 1.0, logMagnitude);
    
    // 归一化以便显示
    cv::normalize(logMagnitude, logMagnitude, 0, 1, cv::NORM_MINMAX);
    
    return logMagnitude;
}

cv::Mat applyFilter(const cv::Mat& image, FilterType type, double param1, double param2) {
    cv::Mat result;
    
    switch (type) {
    case FilterType::Gaussian:
        {
            int ksize = param1 > 0 ? static_cast<int>(param1) : 3;
            double sigma = param2 > 0 ? param2 : 1.0;
            cv::GaussianBlur(image, result, cv::Size(ksize, ksize), sigma);
        }
        break;
        
    case FilterType::Median:
        {
            int ksize = param1 > 0 ? static_cast<int>(param1) : 3;
            if (ksize % 2 == 0) ksize++; // 确保奇数
            cv::medianBlur(image, result, ksize);
        }
        break;
        
    case FilterType::LowPass:
        {
            // 频域低通滤波
            cv::Mat complex = calculateFFT(image, true);
            
            // 创建低通滤波器
            cv::Mat filter = cv::Mat::zeros(complex.size(), CV_32F);
            int cx = complex.cols / 2;
            int cy = complex.rows / 2;
            double radius = param1 > 0 ? param1 : std::min(cx, cy) / 4;
            
            // 创建圆形掩膜
            cv::circle(filter, cv::Point(cx, cy), radius, cv::Scalar(1), -1);
            
            // 分离实部和虚部
            cv::Mat planes[2];
            cv::split(complex, planes);
            
            // 应用滤波器
            planes[0] = planes[0].mul(filter);
            planes[1] = planes[1].mul(filter);
            
            // 合并实部和虚部
            cv::merge(planes, 2, complex);
            
            // 逆变换
            cv::dft(complex, result, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
        }
        break;
        
    case FilterType::HighPass:
        {
            // 频域高通滤波
            cv::Mat complex = calculateFFT(image, true);
            
            // 创建高通滤波器
            cv::Mat filter = cv::Mat::ones(complex.size(), CV_32F);
            int cx = complex.cols / 2;
            int cy = complex.rows / 2;
            double radius = param1 > 0 ? param1 : std::min(cx, cy) / 4;
            
            // 创建圆形掩膜
            cv::circle(filter, cv::Point(cx, cy), radius, cv::Scalar(0), -1);
            
            // 分离实部和虚部
            cv::Mat planes[2];
            cv::split(complex, planes);
            
            // 应用滤波器
            planes[0] = planes[0].mul(filter);
            planes[1] = planes[1].mul(filter);
            
            // 合并实部和虚部
            cv::merge(planes, 2, complex);
            
            // 逆变换
            cv::dft(complex, result, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
        }
        break;
        
    case FilterType::BandPass:
        {
            // 频域带通滤波
            cv::Mat complex = calculateFFT(image, true);
            
            // 创建带通滤波器
            cv::Mat filter = cv::Mat::zeros(complex.size(), CV_32F);
            int cx = complex.cols / 2;
            int cy = complex.rows / 2;
            double innerRadius = param1 > 0 ? param1 : std::min(cx, cy) / 8;
            double outerRadius = param2 > 0 ? param2 : std::min(cx, cy) / 4;
            
            // 创建环形掩膜
            cv::circle(filter, cv::Point(cx, cy), outerRadius, cv::Scalar(1), -1);
            cv::circle(filter, cv::Point(cx, cy), innerRadius, cv::Scalar(0), -1);
            
            // 分离实部和虚部
            cv::Mat planes[2];
            cv::split(complex, planes);
            
            // 应用滤波器
            planes[0] = planes[0].mul(filter);
            planes[1] = planes[1].mul(filter);
            
            // 合并实部和虚部
            cv::merge(planes, 2, complex);
            
            // 逆变换
            cv::dft(complex, result, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
        }
        break;
        
    case FilterType::Kuan:
        {
            // Kuan滤波 - 自适应滤波器，用于抑制SAR图像中的乘性噪声
            
            // 获取参数
            int windowSize = param1 > 0 ? static_cast<int>(param1) : 7;
            if (windowSize % 2 == 0) windowSize++; // 确保窗口大小为奇数
            double damping = param2 > 0 ? param2 : 1.0; // 阻尼系数
            
            // 创建结果图像
            result = cv::Mat::zeros(image.size(), image.type());
            
            // 获取带填充的图像，以处理边界
            cv::Mat paddedImage;
            int padding = windowSize / 2;
            cv::copyMakeBorder(image, paddedImage, padding, padding, padding, padding, 
                              cv::BORDER_REFLECT_101);
            
            // 估计图像中的噪声水平
            double globalMean = cv::mean(image)[0];
            double noiseVariance = 0.0;
            
            // 计算噪声方差 - 可以根据图像估计或使用预设值
            // 这里使用简单的全局噪声水平估计
            cv::Mat tempDiff;
            cv::absdiff(image, globalMean, tempDiff);
            cv::multiply(tempDiff, tempDiff, tempDiff);
            noiseVariance = cv::mean(tempDiff)[0];
            
            // 噪声标准差估计
            double noiseStdDev = std::sqrt(noiseVariance);
            
            // 遍历图像的每个像素
            for (int i = 0; i < image.rows; i++) {
                for (int j = 0; j < image.cols; j++) {
                    // 提取当前窗口
                    cv::Mat window = paddedImage(cv::Rect(j, i, windowSize, windowSize));
                    
                    // 计算窗口的均值和方差
                    cv::Scalar windowMean, windowStdDev;
                    cv::meanStdDev(window, windowMean, windowStdDev);
                    
                    double mean = windowMean[0];
                    double variance = windowStdDev[0] * windowStdDev[0];
                    
                    // 计算噪声与信号的方差比
                    double alpha = 1.0;
                    
                    // 防止除以零
                    if (mean > std::numeric_limits<double>::epsilon()) {
                        double noiseSigma = noiseVariance / (mean * mean);
                        double ENL = damping; // 等效次数，可作为参数传入
                        double cu = 1.0 / std::sqrt(ENL);
                        double ci = std::sqrt(variance) / mean;
                        
                        // 计算Kuan滤波器的权重
                        if (ci > cu) {
                            alpha = (1.0 - (cu * cu / (ci * ci))) / (1.0 + cu * cu);
                        } else {
                            alpha = 0.0; // 完全平滑
                        }
                    }
                    
                    // 应用滤波
                    double centerValue = image.at<float>(i, j);
                    double filteredValue = mean + alpha * (centerValue - mean);
                    
                    // 设置结果
                    result.at<float>(i, j) = static_cast<float>(filteredValue);
                }
            }
        }
        break;
        
    case FilterType::Frost:
        {
            // Frost滤波 - 基于指数加权的自适应滤波器
            
            // 获取参数
            int windowSize = param1 > 0 ? static_cast<int>(param1) : 7;
            if (windowSize % 2 == 0) windowSize++; // 确保窗口大小为奇数
            double damping = param2 > 0 ? param2 : 2.0; // 阻尼系数，控制滤波强度
            
            // 创建结果图像
            result = cv::Mat::zeros(image.size(), image.type());
            
            // 获取带填充的图像，以处理边界
            cv::Mat paddedImage;
            int padding = windowSize / 2;
            cv::copyMakeBorder(image, paddedImage, padding, padding, padding, padding, 
                              cv::BORDER_REFLECT_101);
            
            // 估计图像中的噪声水平
            double globalMean = cv::mean(image)[0];
            double globalVariance = 0.0;
            
            // 计算全局方差
            cv::Mat tempDiff;
            cv::absdiff(image, globalMean, tempDiff);
            cv::multiply(tempDiff, tempDiff, tempDiff);
            globalVariance = cv::mean(tempDiff)[0];
            
            // 遍历图像的每个像素
            for (int i = 0; i < image.rows; i++) {
                for (int j = 0; j < image.cols; j++) {
                    // 提取当前窗口
                    cv::Mat window = paddedImage(cv::Rect(j, i, windowSize, windowSize));
                    
                    // 计算窗口的局部统计量
                    cv::Scalar windowMean, windowStdDev;
                    cv::meanStdDev(window, windowMean, windowStdDev);
                    
                    double localMean = windowMean[0];
                    double localVariance = windowStdDev[0] * windowStdDev[0];
                    
                    // 计算局部变异系数
                    double variationCoeff = 0.0;
                    if (localMean > std::numeric_limits<double>::epsilon()) {
                        variationCoeff = localVariance / (localMean * localMean);
                    }
                    
                    // 计算中心点
                    int centerX = padding;
                    int centerY = padding;
                    
                    // 权重总和
                    double weightSum = 0.0;
                    double valueSum = 0.0;
                    
                    // 遍历窗口内的每个像素，计算Frost权重并应用
                    for (int wi = 0; wi < windowSize; wi++) {
                        for (int wj = 0; wj < windowSize; wj++) {
                            // 计算到中心的距离
                            double dist = std::sqrt((wi - centerY) * (wi - centerY) + 
                                                   (wj - centerX) * (wj - centerX));
                            
                            // 计算Frost滤波器权重
                            double weight = std::exp(-damping * variationCoeff * dist);
                            
                            // 累加加权值
                            double pixelValue = static_cast<double>(window.at<float>(wi, wj));
                            valueSum += weight * pixelValue;
                            weightSum += weight;
                        }
                    }
                    
                    // 设置结果
                    if (weightSum > std::numeric_limits<double>::epsilon()) {
                        result.at<float>(i, j) = static_cast<float>(valueSum / weightSum);
                    } else {
                        result.at<float>(i, j) = image.at<float>(i, j); // 防止除以零
                    }
                }
            }
        }
        break;
        
    default:
        result = image.clone();
    }
    
    return result;
}

} // namespace Utils
} // namespace Analysis
} // namespace SAR 