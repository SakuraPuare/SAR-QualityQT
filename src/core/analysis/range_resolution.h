#ifndef RANGE_RESOLUTION_H
#define RANGE_RESOLUTION_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 距离分辨率分析类
 * 提供用于分析SAR图像距离分辨率的方法
 */
class RangeResolution {
public:
    RangeResolution();

    /**
     * @brief 计算图像的距离分辨率
     * @param image 输入图像
     * @return 距离分辨率值(m)
     */
    double calculateRangeResolution(const cv::Mat& image);

    /**
     * @brief 计算指定范围内的距离分辨率
     * @param image 输入图像
     * @param roi 区域of interest
     * @return 距离分辨率值(m)
     */
    double calculateRangeResolutionInROI(const cv::Mat& image, const cv::Rect& roi);

    /**
     * @brief 设置波长参数
     * @param wavelength 雷达波长(m)
     */
    void setWavelength(double wavelength);

    /**
     * @brief 设置带宽参数
     * @param bandwidth 信号带宽(Hz)
     */
    void setBandwidth(double bandwidth);

    /**
     * @brief 获取分析结果描述
     * @return 分析结果的文字描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量
    double lastRangeResolution;
    double wavelength;    // 雷达波长(m)
    double bandwidth;     // 信号带宽(Hz)
    bool hasParameters;   // 是否设置了所需参数

    // 私有辅助方法
    double computeTheoreticalRangeResolution();
    double estimateResolutionFromImage(const cv::Mat& image);
    cv::Mat computeRangeProfile(const cv::Mat& image);
};

} // namespace Analysis
} // namespace SAR

#endif // RANGE_RESOLUTION_H 