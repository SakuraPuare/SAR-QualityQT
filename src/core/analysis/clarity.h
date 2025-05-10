#ifndef CLARITY_H
#define CLARITY_H

#include <opencv2/core.hpp>
#include <QString>

namespace SAR {
namespace Analysis {

/**
 * @brief 清晰度分析类
 * 提供用于分析SAR图像清晰度的方法
 */
class Clarity {
public:
    Clarity();

    /**
     * @brief 计算图像的清晰度得分
     * @param image 输入图像
     * @return 清晰度得分
     */
    double calculateClarityScore(const cv::Mat& image);

    /**
     * @brief 计算图像的边缘强度
     * @param image 输入图像
     * @return 边缘强度值
     */
    double calculateEdgeStrength(const cv::Mat& image);

    /**
     * @brief 获取分析结果的描述
     * @return 分析结果描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量和辅助方法
    double lastClarityScore;
    double lastEdgeStrength;
};

} // namespace Analysis
} // namespace SAR

#endif // CLARITY_H