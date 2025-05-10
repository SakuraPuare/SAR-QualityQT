#ifndef NESZ_H
#define NESZ_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 噪声等效后向散射系数分析类
 * 提供用于分析SAR图像噪声等效后向散射系数的方法
 */
class NESZ {
public:
    NESZ();

    /**
     * @brief 计算图像的噪声等效后向散射系数
     * @param image 输入图像
     * @return NESZ值(dB)
     */
    double calculateNESZ(const cv::Mat& image);

    /**
     * @brief 计算指定范围内的噪声等效后向散射系数
     * @param image 输入图像
     * @param noiseROI 噪声区域
     * @return NESZ值(dB)
     */
    double calculateNESZWithROI(const cv::Mat& image, const cv::Rect& noiseROI);

    /**
     * @brief 设置系统参数
     * @param transmitPower 发射功率(W)
     * @param antennaGain 天线增益(dB)
     * @param systemLoss 系统损耗(dB)
     */
    void setSystemParameters(double transmitPower, double antennaGain, double systemLoss);

    /**
     * @brief 获取分析结果描述
     * @return 分析结果的文字描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量
    double lastNESZ;
    double transmitPower;  // 发射功率 (W)
    double antennaGain;    // 天线增益 (dB)
    double systemLoss;     // 系统损耗 (dB)
    bool hasParameters;    // 是否设置了所需参数

    // 私有辅助方法
    double estimateNoiseLevel(const cv::Mat& image);
    double calculateTheoreticalNESZ();
};

} // namespace Analysis
} // namespace SAR

#endif // NESZ_H 