#ifndef GLCM_H
#define GLCM_H

#include <opencv2/core.hpp>
#include <QString>
#include <vector>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 灰度共生矩阵(GLCM)分析类
 * 提供用于计算和分析图像纹理特征的功能
 */
class GLCM {
public:
    GLCM();

    /**
     * @brief 计算图像的灰度共生矩阵
     * @param image 输入图像
     * @param distance 像素距离
     * @param angle 角度（弧度）
     * @return 灰度共生矩阵
     */
    cv::Mat calculateGLCM(const cv::Mat& image, int distance, double angle);

    /**
     * @brief 计算图像的对比度特征
     * @param image 输入图像
     * @return 对比度值
     */
    double calculateContrast(const cv::Mat& image);

    /**
     * @brief 计算图像的同质性特征
     * @param image 输入图像
     * @return 同质性值
     */
    double calculateHomogeneity(const cv::Mat& image);

    /**
     * @brief 计算图像的能量特征
     * @param image 输入图像
     * @return 能量值
     */
    double calculateEnergy(const cv::Mat& image);

    /**
     * @brief 计算图像的相关性特征
     * @param image 输入图像
     * @return 相关性值
     */
    double calculateCorrelation(const cv::Mat& image);

    /**
     * @brief 获取所有GLCM特征
     * @param image 输入图像
     * @return 包含所有特征的映射表
     */
    std::map<QString, double> getAllFeatures(const cv::Mat& image);

private:
    // 私有辅助方法和成员变量
    int grayLevels;
    int defaultDistance;
    double defaultAngle;

    cv::Mat normalizeGLCM(const cv::Mat& glcm);
};

} // namespace Analysis
} // namespace SAR

#endif // GLCM_H