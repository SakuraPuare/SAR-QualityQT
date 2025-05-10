#ifndef AZIMUTH_RESOLUTION_H
#define AZIMUTH_RESOLUTION_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 方位分辨率分析类
 * 提供用于分析SAR图像方位分辨率的方法
 */
class AzimuthResolution {
public:
    AzimuthResolution();

    /**
     * @brief 计算图像的方位分辨率
     * @param image 输入图像
     * @return 方位分辨率值(m)
     */
    double calculateAzimuthResolution(const cv::Mat& image);

    /**
     * @brief 计算指定范围内的方位分辨率
     * @param image 输入图像
     * @param roi 区域of interest
     * @return 方位分辨率值(m)
     */
    double calculateAzimuthResolutionInROI(const cv::Mat& image, const cv::Rect& roi);

    /**
     * @brief 设置波长参数
     * @param wavelength 雷达波长(m)
     */
    void setWavelength(double wavelength);

    /**
     * @brief 设置天线长度参数
     * @param antennaLength 天线长度(m)
     */
    void setAntennaLength(double antennaLength);

    /**
     * @brief 获取分析结果描述
     * @return 分析结果的文字描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量
    double lastAzimuthResolution;
    double wavelength;      // 雷达波长 (m)
    double antennaLength;   // 天线长度 (m)
    bool hasParameters;     // 是否设置了所需参数

    // 私有辅助方法
    double computeTheoreticalAzimuthResolution();
    double estimateResolutionFromImage(const cv::Mat& image);
    cv::Mat computeAzimuthProfile(const cv::Mat& image);
};

} // namespace Analysis
} // namespace SAR

#endif // AZIMUTH_RESOLUTION_H