#include "imagefilters.h"
#include "include/logger.h"

#include <QCoreApplication>
#include <opencv2/imgproc.hpp>
#include <cmath>

namespace SAR {
namespace Core {

cv::Mat ImageFilters::applyFilter(const cv::Mat& image, 
                                  const FilterParameters& params, 
                                  QString* log) {
    // 首先检查图像是否有效
    if (image.empty()) {
        logMessage(log, QCoreApplication::translate("ImageFilters", "错误：输入图像为空"));
        return cv::Mat();
    }

    // 根据滤波器类型调用相应的方法
    switch (params.type) {
        case FilterType::LowPass:
            return applyLowPassFilter(image, params, log);
        case FilterType::HighPass:
            return applyHighPassFilter(image, params, log);
        case FilterType::BandPass:
            return applyBandPassFilter(image, params, log);
        case FilterType::Median:
            return applyMedianFilter(image, params, log);
        case FilterType::Gaussian:
            return applyGaussianFilter(image, params, log);
        case FilterType::Bilateral:
            return applyBilateralFilter(image, params, log);
        case FilterType::Lee:
            return applyLeeFilter(image, params, log);
        case FilterType::Frost:
            return applyFrostFilter(image, params, log);
        case FilterType::Kuan:
            return applyKuanFilter(image, params, log);
        case FilterType::Custom:
            return applyCustomFilter(image, params, log);
        default:
            logMessage(log, QCoreApplication::translate("ImageFilters", "警告：未知滤波器类型，使用高斯滤波"));
            return applyGaussianFilter(image, params, log);
    }
}

QString ImageFilters::getFilterTypeDescription(FilterType type) {
    switch (type) {
        case FilterType::LowPass:
            return QCoreApplication::translate("ImageFilters", "低通滤波");
        case FilterType::HighPass:
            return QCoreApplication::translate("ImageFilters", "高通滤波");
        case FilterType::BandPass:
            return QCoreApplication::translate("ImageFilters", "带通滤波");
        case FilterType::Median:
            return QCoreApplication::translate("ImageFilters", "中值滤波");
        case FilterType::Gaussian:
            return QCoreApplication::translate("ImageFilters", "高斯滤波");
        case FilterType::Bilateral:
            return QCoreApplication::translate("ImageFilters", "双边滤波");
        case FilterType::Lee:
            return QCoreApplication::translate("ImageFilters", "Lee滤波");
        case FilterType::Frost:
            return QCoreApplication::translate("ImageFilters", "Frost滤波");
        case FilterType::Kuan:
            return QCoreApplication::translate("ImageFilters", "Kuan滤波");
        case FilterType::Custom:
            return QCoreApplication::translate("ImageFilters", "自定义滤波");
        default:
            return QCoreApplication::translate("ImageFilters", "未知滤波器");
    }
}

cv::Mat ImageFilters::applyLowPassFilter(const cv::Mat& image, 
                                         const FilterParameters& params, 
                                         QString* log) {
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用低通滤波器..."));
    
    // 确保图像为浮点类型
    cv::Mat floatImage;
    if (image.depth() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }
    
    // 频域滤波实现
    // 计算FFT
    cv::Mat padded;
    int m = cv::getOptimalDFTSize(floatImage.rows);
    int n = cv::getOptimalDFTSize(floatImage.cols);
    cv::copyMakeBorder(floatImage, padded, 0, m - floatImage.rows, 0, n - floatImage.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    
    cv::Mat planes[] = {padded, cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);
    
    // 执行傅立叶变换
    cv::dft(complexI, complexI);
    
    // 频移
    int cx = complexI.cols / 2;
    int cy = complexI.rows / 2;
    
    cv::Mat q0(complexI, cv::Rect(0, 0, cx, cy));      // 左上角
    cv::Mat q1(complexI, cv::Rect(cx, 0, cx, cy));     // 右上角
    cv::Mat q2(complexI, cv::Rect(0, cy, cx, cy));     // 左下角
    cv::Mat q3(complexI, cv::Rect(cx, cy, cx, cy));    // 右下角
    
    cv::Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
    
    // 创建低通滤波器掩膜
    cv::Mat filter = cv::Mat::zeros(complexI.size(), CV_32F);
    double radius = params.param1 > 0 ? params.param1 : std::min(cx, cy) / 4;
    
    // 创建圆形滤波器
    cv::circle(filter, cv::Point(cx, cy), radius, cv::Scalar(1), -1);
    
    // 分离实部和虚部
    cv::split(complexI, planes);
    
    // 应用滤波器
    planes[0] = planes[0].mul(filter);
    planes[1] = planes[1].mul(filter);
    
    // 合并实部和虚部
    cv::merge(planes, 2, complexI);
    
    // 执行逆傅立叶变换
    cv::idft(complexI, complexI);
    
    // 分离实部和虚部
    cv::split(complexI, planes);
    
    // 计算幅值
    cv::magnitude(planes[0], planes[1], planes[0]);
    cv::Mat result = planes[0];
    
    // 归一化
    cv::normalize(result, result, 0, 1, cv::NORM_MINMAX);
    
    // 裁剪回原始尺寸
    result = result(cv::Rect(0, 0, image.cols, image.rows));
    
    // 转换回原始数据类型
    if (image.depth() != CV_32F) {
        double minVal, maxVal;
        cv::minMaxLoc(image, &minVal, &maxVal);
        result.convertTo(result, image.type(), maxVal, 0);
    }
    
    logMessage(log, QCoreApplication::translate("ImageFilters", "低通滤波器应用完成"));
    return result;
}

cv::Mat ImageFilters::applyHighPassFilter(const cv::Mat& image, 
                                          const FilterParameters& params, 
                                          QString* log) {
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用高通滤波器..."));
    
    // 确保图像为浮点类型
    cv::Mat floatImage;
    if (image.depth() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }
    
    // 频域滤波实现，与低通滤波类似，但滤波器掩膜相反
    // 计算FFT
    cv::Mat padded;
    int m = cv::getOptimalDFTSize(floatImage.rows);
    int n = cv::getOptimalDFTSize(floatImage.cols);
    cv::copyMakeBorder(floatImage, padded, 0, m - floatImage.rows, 0, n - floatImage.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    
    cv::Mat planes[] = {padded, cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);
    
    // 执行傅立叶变换
    cv::dft(complexI, complexI);
    
    // 频移
    int cx = complexI.cols / 2;
    int cy = complexI.rows / 2;
    
    cv::Mat q0(complexI, cv::Rect(0, 0, cx, cy));      // 左上角
    cv::Mat q1(complexI, cv::Rect(cx, 0, cx, cy));     // 右上角
    cv::Mat q2(complexI, cv::Rect(0, cy, cx, cy));     // 左下角
    cv::Mat q3(complexI, cv::Rect(cx, cy, cx, cy));    // 右下角
    
    cv::Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
    
    // 创建高通滤波器掩膜（与低通相反）
    cv::Mat filter = cv::Mat::ones(complexI.size(), CV_32F);
    double radius = params.param1 > 0 ? params.param1 : std::min(cx, cy) / 4;
    
    // 创建圆形滤波器（从掩膜中去除低频）
    cv::circle(filter, cv::Point(cx, cy), radius, cv::Scalar(0), -1);
    
    // 分离实部和虚部
    cv::split(complexI, planes);
    
    // 应用滤波器
    planes[0] = planes[0].mul(filter);
    planes[1] = planes[1].mul(filter);
    
    // 合并实部和虚部
    cv::merge(planes, 2, complexI);
    
    // 执行逆傅立叶变换
    cv::idft(complexI, complexI);
    
    // 分离实部和虚部
    cv::split(complexI, planes);
    
    // 计算幅值
    cv::magnitude(planes[0], planes[1], planes[0]);
    cv::Mat result = planes[0];
    
    // 归一化
    cv::normalize(result, result, 0, 1, cv::NORM_MINMAX);
    
    // 裁剪回原始尺寸
    result = result(cv::Rect(0, 0, image.cols, image.rows));
    
    // 转换回原始数据类型
    if (image.depth() != CV_32F) {
        double minVal, maxVal;
        cv::minMaxLoc(image, &minVal, &maxVal);
        result.convertTo(result, image.type(), maxVal, 0);
    }
    
    logMessage(log, QCoreApplication::translate("ImageFilters", "高通滤波器应用完成"));
    return result;
}

cv::Mat ImageFilters::applyBandPassFilter(const cv::Mat& image, 
                                          const FilterParameters& params, 
                                          QString* log) {
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用带通滤波器..."));
    
    // 确保图像为浮点类型
    cv::Mat floatImage;
    if (image.depth() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }
    
    // 频域滤波实现
    // 计算FFT
    cv::Mat padded;
    int m = cv::getOptimalDFTSize(floatImage.rows);
    int n = cv::getOptimalDFTSize(floatImage.cols);
    cv::copyMakeBorder(floatImage, padded, 0, m - floatImage.rows, 0, n - floatImage.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    
    cv::Mat planes[] = {padded, cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);
    
    // 执行傅立叶变换
    cv::dft(complexI, complexI);
    
    // 频移
    int cx = complexI.cols / 2;
    int cy = complexI.rows / 2;
    
    cv::Mat q0(complexI, cv::Rect(0, 0, cx, cy));      // 左上角
    cv::Mat q1(complexI, cv::Rect(cx, 0, cx, cy));     // 右上角
    cv::Mat q2(complexI, cv::Rect(0, cy, cx, cy));     // 左下角
    cv::Mat q3(complexI, cv::Rect(cx, cy, cx, cy));    // 右下角
    
    cv::Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
    
    // 创建带通滤波器掩膜
    cv::Mat filter = cv::Mat::zeros(complexI.size(), CV_32F);
    double innerRadius = params.param1 > 0 ? params.param1 : std::min(cx, cy) / 8;
    double outerRadius = params.param2 > 0 ? params.param2 : std::min(cx, cy) / 4;
    
    // 创建环形滤波器
    cv::circle(filter, cv::Point(cx, cy), outerRadius, cv::Scalar(1), -1);
    cv::circle(filter, cv::Point(cx, cy), innerRadius, cv::Scalar(0), -1);
    
    // 分离实部和虚部
    cv::split(complexI, planes);
    
    // 应用滤波器
    planes[0] = planes[0].mul(filter);
    planes[1] = planes[1].mul(filter);
    
    // 合并实部和虚部
    cv::merge(planes, 2, complexI);
    
    // 执行逆傅立叶变换
    cv::idft(complexI, complexI);
    
    // 分离实部和虚部
    cv::split(complexI, planes);
    
    // 计算幅值
    cv::magnitude(planes[0], planes[1], planes[0]);
    cv::Mat result = planes[0];
    
    // 归一化
    cv::normalize(result, result, 0, 1, cv::NORM_MINMAX);
    
    // 裁剪回原始尺寸
    result = result(cv::Rect(0, 0, image.cols, image.rows));
    
    // 转换回原始数据类型
    if (image.depth() != CV_32F) {
        double minVal, maxVal;
        cv::minMaxLoc(image, &minVal, &maxVal);
        result.convertTo(result, image.type(), maxVal, 0);
    }
    
    logMessage(log, QCoreApplication::translate("ImageFilters", "带通滤波器应用完成"));
    return result;
}

cv::Mat ImageFilters::applyMedianFilter(const cv::Mat& image, 
                                        const FilterParameters& params, 
                                        QString* log) {
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用中值滤波器..."));
    
    int ksize = params.kernelSize;
    if (ksize % 2 == 0) {
        ksize++;  // 确保内核大小为奇数
        logMessage(log, QCoreApplication::translate("ImageFilters", "警告：内核大小必须为奇数，已调整为 %1").arg(ksize));
    }
    
    cv::Mat result;
    cv::medianBlur(image, result, ksize);
    
    logMessage(log, QCoreApplication::translate("ImageFilters", "中值滤波器应用完成，内核大小：%1").arg(ksize));
    return result;
}

cv::Mat ImageFilters::applyGaussianFilter(const cv::Mat& image, 
                                          const FilterParameters& params, 
                                          QString* log) {
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用高斯滤波器..."));
    
    int ksize = params.kernelSize;
    if (ksize % 2 == 0) {
        ksize++;  // 确保内核大小为奇数
        logMessage(log, QCoreApplication::translate("ImageFilters", "警告：内核大小必须为奇数，已调整为 %1").arg(ksize));
    }
    
    double sigma = params.sigma > 0 ? params.sigma : 0;
    
    cv::Mat result;
    cv::GaussianBlur(image, result, cv::Size(ksize, ksize), sigma);
    
    logMessage(log, QCoreApplication::translate("ImageFilters", "高斯滤波器应用完成，内核大小：%1，Sigma：%2").arg(ksize).arg(sigma));
    return result;
}

cv::Mat ImageFilters::applyBilateralFilter(const cv::Mat& image, 
                                           const FilterParameters& params, 
                                           QString* log) {
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用双边滤波器..."));
    
    int d = params.kernelSize;
    double sigmaColor = params.sigma;
    double sigmaSpace = params.param1 > 0 ? params.param1 : params.sigma;
    
    cv::Mat result;
    cv::bilateralFilter(image, result, d, sigmaColor, sigmaSpace);
    
    logMessage(log, QCoreApplication::translate("ImageFilters", "双边滤波器应用完成，直径：%1，色彩Sigma：%2，空间Sigma：%3")
                    .arg(d).arg(sigmaColor).arg(sigmaSpace));
    return result;
}

cv::Mat ImageFilters::applyLeeFilter(const cv::Mat& image, 
                                     const FilterParameters& params, 
                                     QString* log) {
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用Lee滤波器..."));
    
    // 确保图像为浮点类型
    cv::Mat floatImage;
    if (image.depth() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }
    
    int ksize = params.kernelSize;
    if (ksize % 2 == 0) {
        ksize++;  // 确保内核大小为奇数
    }
    
    double damping = params.damping;
    
    cv::Mat result = floatImage.clone();
    cv::Mat mean, stdDev;
    
    // 先计算均值
    cv::boxFilter(floatImage, mean, CV_32F, cv::Size(ksize, ksize), cv::Point(-1, -1), true, cv::BORDER_REFLECT);
    
    // 计算局部标准差
    cv::Mat localVariance;
    cv::multiply(floatImage, floatImage, localVariance);
    cv::boxFilter(localVariance, localVariance, CV_32F, cv::Size(ksize, ksize), cv::Point(-1, -1), true, cv::BORDER_REFLECT);
    cv::multiply(mean, mean, stdDev);
    cv::subtract(localVariance, stdDev, localVariance);
    
    // 估计整体噪声
    double noiseVariance = cv::mean(localVariance)[0];
    
    // 应用Lee滤波器
    for (int i = 0; i < floatImage.rows; i++) {
        for (int j = 0; j < floatImage.cols; j++) {
            float localMean = mean.at<float>(i, j);
            float localVar = localVariance.at<float>(i, j);
            
            if (localVar < 0) localVar = 0; // 防止计算误差导致负方差
            
            float k = localVar / (localVar + damping * noiseVariance);
            if (k < 0) k = 0;
            if (k > 1) k = 1;
            
            result.at<float>(i, j) = localMean + k * (floatImage.at<float>(i, j) - localMean);
        }
    }
    
    // 转换回原始数据类型
    if (image.depth() != CV_32F) {
        result.convertTo(result, image.type());
    }
    
    logMessage(log, QCoreApplication::translate("ImageFilters", "Lee滤波器应用完成，内核大小：%1，阻尼系数：%2").arg(ksize).arg(damping));
    return result;
}

cv::Mat ImageFilters::applyFrostFilter(const cv::Mat& image, 
                                       const FilterParameters& params, 
                                       QString* log) {
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用Frost滤波器..."));
    
    // 确保图像为浮点类型
    cv::Mat floatImage;
    if (image.depth() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }
    
    int ksize = params.kernelSize;
    if (ksize % 2 == 0) {
        ksize++;  // 确保内核大小为奇数
    }
    
    double damping = params.damping;
    int radius = ksize / 2;
    
    cv::Mat result = cv::Mat::zeros(floatImage.size(), CV_32F);
    
    // 估计噪声
    double noiseVariance = estimateNoiseVariance(floatImage);
    
    // 对每个像素应用Frost滤波器
    for (int i = radius; i < floatImage.rows - radius; i++) {
        for (int j = radius; j < floatImage.cols - radius; j++) {
            // 提取局部窗口
            cv::Mat window = floatImage(cv::Rect(j - radius, i - radius, ksize, ksize));
            
            // 计算局部统计量
            cv::Scalar mean, stdDev;
            cv::meanStdDev(window, mean, stdDev);
            
            double localMean = mean[0];
            double localVar = stdDev[0] * stdDev[0];
            
            // 计算局部变异系数
            double localCV = localVar / (localMean * localMean);
            
            // 为窗口中的每个像素计算权重
            double weightSum = 0;
            double pixelSum = 0;
            
            for (int wi = 0; wi < ksize; wi++) {
                for (int wj = 0; wj < ksize; wj++) {
                    // 计算到中心的距离
                    double dist = std::sqrt((wi - radius) * (wi - radius) + (wj - radius) * (wj - radius));
                    
                    // 计算权重
                    double weight = std::exp(-damping * localCV * dist);
                    
                    // 累加加权像素值
                    double pixelValue = window.at<float>(wi, wj);
                    pixelSum += pixelValue * weight;
                    weightSum += weight;
                }
            }
            
            // 设置结果像素值
            if (weightSum > 0) {
                result.at<float>(i, j) = pixelSum / weightSum;
            } else {
                result.at<float>(i, j) = floatImage.at<float>(i, j);
            }
        }
    }
    
    // 处理边界区域（简单复制）
    for (int i = 0; i < floatImage.rows; i++) {
        for (int j = 0; j < floatImage.cols; j++) {
            if (i < radius || i >= floatImage.rows - radius ||
                j < radius || j >= floatImage.cols - radius) {
                result.at<float>(i, j) = floatImage.at<float>(i, j);
            }
        }
    }
    
    // 转换回原始数据类型
    if (image.depth() != CV_32F) {
        result.convertTo(result, image.type());
    }
    
    logMessage(log, QCoreApplication::translate("ImageFilters", "Frost滤波器应用完成，内核大小：%1，阻尼系数：%2").arg(ksize).arg(damping));
    return result;
}

cv::Mat ImageFilters::applyCustomFilter(const cv::Mat& image, 
                                        const FilterParameters& params, 
                                        QString* log) {
    QString filterName = params.customFilterName;
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用自定义滤波器：%1...").arg(filterName));
    
    // 这里可以根据自定义滤波器名称实现不同的滤波逻辑
    // 目前返回原始图像的拷贝
    logMessage(log, QCoreApplication::translate("ImageFilters", "警告：自定义滤波器 %1 尚未实现").arg(filterName));
    
    return image.clone();
}

void ImageFilters::logMessage(QString* log, const QString& message) {
    if (log) {
        *log += message + "\n";
    }
    
    // 同时输出到全局日志系统
    LOG_INFO(message);
}

// 辅助方法：估计噪声方差（用于Frost滤波器）
double ImageFilters::estimateNoiseVariance(const cv::Mat& image) {
    // 使用中值滤波器差异来估计噪声
    cv::Mat medianFiltered;
    cv::medianBlur(image, medianFiltered, 3);
    
    cv::Mat diff;
    cv::absdiff(image, medianFiltered, diff);
    
    cv::Scalar mean, stdDev;
    cv::meanStdDev(diff, mean, stdDev);
    
    // 返回估计的噪声方差
    return mean[0] * mean[0];
}

cv::Mat ImageFilters::applyKuanFilter(const cv::Mat& image, 
                                     const FilterParameters& params, 
                                     QString* log) {
    logMessage(log, QCoreApplication::translate("ImageFilters", "应用Kuan滤波器..."));
    
    // 确保图像为浮点类型
    cv::Mat floatImage;
    if (image.depth() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }
    
    int ksize = params.kernelSize;
    if (ksize % 2 == 0) {
        ksize++;  // 确保内核大小为奇数
    }
    
    double damping = params.damping;
    int radius = ksize / 2;
    
    cv::Mat result = cv::Mat::zeros(floatImage.size(), CV_32F);
    cv::Mat paddedImage;
    
    // 添加边界填充，以处理边缘像素
    cv::copyMakeBorder(floatImage, paddedImage, radius, radius, radius, radius, 
                      cv::BORDER_REFLECT_101);
    
    // 估计图像噪声水平
    double noiseVariance = estimateNoiseVariance(floatImage);
    
    // 遍历每个像素
    for (int i = 0; i < floatImage.rows; i++) {
        for (int j = 0; j < floatImage.cols; j++) {
            // 提取局部窗口
            cv::Mat window = paddedImage(cv::Rect(j, i, ksize, ksize));
            
            // 计算局部统计量
            cv::Scalar meanValue, stdDev;
            cv::meanStdDev(window, meanValue, stdDev);
            
            double mean = meanValue[0];
            double variance = stdDev[0] * stdDev[0];
            
            // 计算Kuan滤波器的自适应权重
            double alpha = 1.0;
            
            // 防止除以零
            if (mean > 1e-5) {
                double noiseSigma = noiseVariance / (mean * mean);
                double ENL = damping;  // 等效视数，这里使用阻尼系数作为参数
                double cu = 1.0 / std::sqrt(ENL);
                double ci = std::sqrt(variance) / mean;
                
                // 计算Kuan滤波器的权重
                if (ci > cu) {
                    alpha = (1.0 - (cu * cu / (ci * ci))) / (1.0 + cu * cu);
                } else {
                    alpha = 0.0;  // 完全平滑
                }
            }
            
            // 应用滤波器
            double pixelValue = floatImage.at<float>(i, j);
            double filteredValue = mean + alpha * (pixelValue - mean);
            result.at<float>(i, j) = filteredValue;
        }
    }
    
    // 转换回原始数据类型
    if (image.depth() != CV_32F) {
        result.convertTo(result, image.type());
    }
    
    logMessage(log, QCoreApplication::translate("ImageFilters", "Kuan滤波器应用完成，内核大小：%1，阻尼系数：%2").arg(ksize).arg(damping));
    return result;
}

} // namespace Core
} // namespace SAR 