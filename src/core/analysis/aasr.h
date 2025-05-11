#ifndef AASR_H
#define AASR_H

#include <opencv2/core.hpp>
#include <QString>
#include <vector>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 方位模糊度分析类
 * 提供用于分析SAR图像方位模糊度的方法
 */
class AASR {
public:
    AASR();

    /**
     * @brief 计算图像的方位模糊度
     * @param image 输入图像
     * @param dopplerCenterFreq 多普勒中心频率 (Hz)
     * @param processingBandwidth 方位向成像处理带宽 (Hz)
     * @param antennaGain 天线方位向增益函数 (仅用于参考)
     * @return 方位模糊度比值
     */
    double calculateAASR(const cv::Mat& image, double dopplerCenterFreq, double processingBandwidth,
                        const std::vector<double>& antennaGain = std::vector<double>());

    /**
     * @brief 计算指定范围内的方位模糊度
     * @param image 输入图像
     * @param roi 区域of interest
     * @param dopplerCenterFreq 多普勒中心频率 (Hz)
     * @param processingBandwidth 方位向成像处理带宽 (Hz)
     * @param antennaGain 天线方位向增益函数 (仅用于参考)
     * @return 方位模糊度比值
     */
    double calculateAASRInROI(const cv::Mat& image, const cv::Rect& roi, 
                             double dopplerCenterFreq, double processingBandwidth,
                             const std::vector<double>& antennaGain = std::vector<double>());

    /**
     * @brief 获取分析结果描述
     * @return 分析结果的文字描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量
    double lastAASR;

    // 私有辅助方法
    double calculateAASRValue(const cv::Mat& image, double dopplerCenterFreq, double processingBandwidth,
                             const std::vector<double>& antennaGain);
    double simulateAntennaAzimuthGain(double dopplerFreq, const std::vector<double>& antennaGain);
    cv::Mat estimateAzimuthSpectrum(const cv::Mat& image);
};

} // namespace Analysis
} // namespace SAR

#endif // AASR_H 