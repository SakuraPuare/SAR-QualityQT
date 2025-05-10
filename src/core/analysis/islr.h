#ifndef ISLR_H
#define ISLR_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 积分旁瓣比分析类
 * 提供用于分析SAR图像积分旁瓣比的方法
 */
class ISLR {
public:
    ISLR();

    /**
     * @brief 计算图像的积分旁瓣比
     * @param image 输入图像
     * @return 积分旁瓣比值(dB)
     */
    double calculateISLR(const cv::Mat& image);

    /**
     * @brief 计算指定范围内的积分旁瓣比
     * @param image 输入图像
     * @param roi 区域of interest
     * @return 积分旁瓣比值(dB)
     */
    double calculateISLRInROI(const cv::Mat& image, const cv::Rect& roi);

    /**
     * @brief 获取分析结果描述
     * @return 分析结果的文字描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量
    double lastISLR;

    // 私有辅助方法
    cv::Mat calculatePointSpreadFunction(const cv::Mat& image);
    cv::Mat extractMainLobe(const cv::Mat& psf, cv::Point& peakLocation);
    double computeISLRFromPSF(const cv::Mat& psf, const cv::Mat& mainLobe, const cv::Point& peakLocation);
};

} // namespace Analysis
} // namespace SAR

#endif // ISLR_H 