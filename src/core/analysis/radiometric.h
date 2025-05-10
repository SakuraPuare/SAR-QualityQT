#ifndef RADIOMETRIC_H
#define RADIOMETRIC_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 辐射度分析类
 * 提供用于分析SAR图像辐射特性的方法
 */
class Radiometric {
public:
    Radiometric();

    /**
     * @brief 计算图像的亮度均值
     * @param image 输入图像
     * @return 亮度均值
     */
    double calculateMeanIntensity(const cv::Mat& image);

    /**
     * @brief 计算图像的对比度
     * @param image 输入图像
     * @return 对比度值
     */
    double calculateContrast(const cv::Mat& image);

    /**
     * @brief 计算图像的动态范围
     * @param image 输入图像
     * @return 动态范围值（dB）
     */
    double calculateDynamicRange(const cv::Mat& image);

    /**
     * @brief 计算图像的均方根值
     * @param image 输入图像
     * @return RMS值
     */
    double calculateRMS(const cv::Mat& image);

    /**
     * @brief 计算图像的辐射精度
     * @param image 输入图像
     * @param reference 参考图像
     * @return 辐射精度值(dB)
     */
    double calculateRadiometricAccuracy(const cv::Mat& image, const cv::Mat& reference);

    /**
     * @brief 计算图像的辐射分辨率
     * @param image 输入图像
     * @return 辐射分辨率值(dB)
     */
    double calculateRadiometricResolution(const cv::Mat& image);

    /**
     * @brief 计算图像的等效视数
     * @param image 输入图像
     * @return ENL值
     */
    double calculateENL(const cv::Mat& image);

    /**
     * @brief 获取所有辐射度特征
     * @param image 输入图像
     * @return 包含所有特征的映射表
     */
    std::map<QString, double> getAllFeatures(const cv::Mat& image);

    /**
     * @brief 获取分析结果描述
     * @return 分析结果的文字描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量
    std::map<QString, double> lastResults;
};

} // namespace Analysis
} // namespace SAR

#endif // RADIOMETRIC_H