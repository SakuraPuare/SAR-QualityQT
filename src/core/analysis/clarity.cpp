#include "clarity.h"
#include <opencv2/imgproc.hpp>

namespace SAR {
namespace Analysis {

double ClarityAnalysis::calculateClarity(const cv::Mat& image) {
    // 实现从原来的analysis_clarity.cpp移植
    // ...原有的计算清晰度的代码...
    
    // 这里是简化示例
    return calculateGradientEnergy(image);
}

double ClarityAnalysis::calculateGradientEnergy(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() > 1) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // 使用Sobel算子计算梯度
    cv::Mat grad_x, grad_y;
    cv::Sobel(gray, grad_x, CV_64F, 1, 0, 3);
    cv::Sobel(gray, grad_y, CV_64F, 0, 1, 3);
    
    // 计算梯度平方和
    cv::Mat grad_pow = grad_x.mul(grad_x) + grad_y.mul(grad_y);
    
    // 计算梯度能量
    double energy = cv::sum(grad_pow)[0];
    
    return energy / (gray.rows * gray.cols);
}

double ClarityAnalysis::calculateTenengradVariance(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() > 1) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // 使用Sobel算子计算梯度
    cv::Mat grad_x, grad_y;
    cv::Sobel(gray, grad_x, CV_64F, 1, 0, 3);
    cv::Sobel(gray, grad_y, CV_64F, 0, 1, 3);
    
    // 计算梯度平方和
    cv::Mat grad_pow = grad_x.mul(grad_x) + grad_y.mul(grad_y);
    
    // 计算Tenengrad方差
    cv::Scalar mean, stddev;
    cv::meanStdDev(grad_pow, mean, stddev);
    
    return stddev[0];
}

double ClarityAnalysis::calculateEntropy(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() > 1) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // 计算直方图
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    cv::Mat hist;
    cv::calcHist(&gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
    
    // 归一化直方图
    double total = gray.rows * gray.cols;
    hist /= total;
    
    // 计算熵
    double entropy = 0;
    for (int i = 0; i < hist.rows; i++) {
        double p = hist.at<float>(i);
        if (p > 0) {
            entropy -= p * log2(p);
        }
    }
    
    return entropy;
}

} // namespace Analysis
} // namespace SAR 