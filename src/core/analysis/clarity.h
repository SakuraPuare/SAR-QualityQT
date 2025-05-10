#ifndef CLARITY_ANALYSIS_H
#define CLARITY_ANALYSIS_H

#include <opencv2/opencv.hpp>
#include <QString>

namespace SAR {
namespace Analysis {

/**
 * @brief 图像清晰度分析功能
 */
class ClarityAnalysis {
public:
    /**
     * @brief 计算图像清晰度指标
     * @param image 输入图像
     * @return 清晰度指标值
     */
    static double calculateClarity(const cv::Mat& image);
    
    /**
     * @brief 计算图像的梯度能量
     * @param image 输入图像
     * @return 梯度能量值
     */
    static double calculateGradientEnergy(const cv::Mat& image);
    
    /**
     * @brief 计算Tenengrad方差
     * @param image 输入图像
     * @return Tenengrad方差值
     */
    static double calculateTenengradVariance(const cv::Mat& image);
    
    /**
     * @brief 计算图像熵
     * @param image 输入图像
     * @return 熵值
     */
    static double calculateEntropy(const cv::Mat& image);
};

} // namespace Analysis
} // namespace SAR

#endif // CLARITY_ANALYSIS_H 