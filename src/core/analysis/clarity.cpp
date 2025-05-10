#include "clarity.h"
#include <opencv2/imgproc.hpp>
#include <stdexcept>

namespace SAR {
namespace Analysis {

Clarity::Clarity() : lastClarityScore(0.0), lastEdgeStrength(0.0) {
}

double Clarity::calculateClarityScore(const cv::Mat& image) {
    if (image.empty()) {
        throw std::invalid_argument("输入图像为空");
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算拉普拉斯算子响应
    cv::Mat laplacian;
    cv::Laplacian(floatImage, laplacian, CV_32F);

    // 计算清晰度得分（使用拉普拉斯响应的方差）
    cv::Scalar mean, stdDev;
    cv::meanStdDev(laplacian, mean, stdDev);

    lastClarityScore = stdDev[0] * stdDev[0];
    return lastClarityScore;
}

double Clarity::calculateEdgeStrength(const cv::Mat& image) {
    if (image.empty()) {
        throw std::invalid_argument("输入图像为空");
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算 Sobel 梯度
    cv::Mat gradX, gradY;
    cv::Sobel(floatImage, gradX, CV_32F, 1, 0);
    cv::Sobel(floatImage, gradY, CV_32F, 0, 1);

    // 计算梯度幅值
    cv::Mat gradMagnitude;
    cv::magnitude(gradX, gradY, gradMagnitude);

    // 计算平均梯度幅值作为边缘强度
    lastEdgeStrength = cv::mean(gradMagnitude)[0];
    return lastEdgeStrength;
}

QString Clarity::getResultDescription() const {
    return QString("清晰度得分：%1, 边缘强度：%2")
        .arg(lastClarityScore, 0, 'f', 2)
        .arg(lastEdgeStrength, 0, 'f', 2);
}

} // namespace Analysis
} // namespace SAR 