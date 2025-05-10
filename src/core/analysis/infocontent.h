#ifndef INFOCONTENT_H
#define INFOCONTENT_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 信息内容分析类
 * 提供用于分析SAR图像信息内容的方法
 */
class InfoContent {
public:
    InfoContent();

    /**
     * @brief 计算图像的熵值
     * @param image 输入图像
     * @return 熵值
     */
    double calculateEntropy(const cv::Mat& image);

    /**
     * @brief 计算图像的局部熵
     * @param image 输入图像
     * @param windowSize 局部窗口大小
     * @return 局部熵图像
     */
    cv::Mat calculateLocalEntropy(const cv::Mat& image, int windowSize = 7);

    /**
     * @brief 计算图像的信息量
     * @param image 输入图像
     * @return 信息量
     */
    double calculateInformationContent(const cv::Mat& image);

    /**
     * @brief 计算图像的复杂度
     * @param image 输入图像
     * @return 复杂度值
     */
    double calculateComplexity(const cv::Mat& image);

    /**
     * @brief 获取所有信息内容特征
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

    // 私有辅助方法
    cv::Mat normalizeHistogram(const cv::Mat& hist);
};

} // namespace Analysis
} // namespace SAR

#endif // INFOCONTENT_H