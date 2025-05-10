#ifndef PSLR_H
#define PSLR_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 峰值旁瓣比分析类
 * 提供用于分析SAR图像峰值旁瓣比的方法
 */
class PSLR {
public:
    PSLR();

    /**
     * @brief 计算图像的峰值旁瓣比
     * @param image 输入图像
     * @return 峰值旁瓣比值(dB)
     */
    double calculatePSLR(const cv::Mat& image);

    /**
     * @brief 计算指定范围内的峰值旁瓣比
     * @param image 输入图像
     * @param roi 区域of interest
     * @return 峰值旁瓣比值(dB)
     */
    double calculatePSLRInROI(const cv::Mat& image, const cv::Rect& roi);

    /**
     * @brief 获取分析结果描述
     * @return 分析结果的文字描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量
    double lastPSLR;

    // 私有辅助方法
    cv::Mat calculatePointSpreadFunction(const cv::Mat& image);
    cv::Point findPeakLocation(const cv::Mat& psf);
    cv::Point findMaxSidelobePeak(const cv::Mat& psf, const cv::Point& mainPeakLoc);
    double computePSLRFromPSF(const cv::Mat& psf, const cv::Point& mainPeakLoc, const cv::Point& sidelobePeakLoc);
};

} // namespace Analysis
} // namespace SAR

#endif // PSLR_H 