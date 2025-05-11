#ifndef RASR_H
#define RASR_H

#include <opencv2/core.hpp>
#include <QString>
#include <vector>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 距离模糊度分析类
 * 提供用于分析SAR图像距离模糊度的方法
 */
class RASR {
public:
    RASR();

    /**
     * @brief 计算图像的距离模糊度
     * @param image 输入图像
     * @param PRF 雷达脉冲重复频率 (Hz)
     * @param R0 目标距离 (m)
     * @param incidenceAngle 入射角 (rad)
     * @param antennaGain 天线增益函数 (仅用于参考)
     * @return 距离模糊度比值
     */
    double calculateRASR(const cv::Mat& image, double PRF, double R0, double incidenceAngle,
                        const std::vector<double>& antennaGain = std::vector<double>());

    /**
     * @brief 计算指定范围内的距离模糊度
     * @param image 输入图像
     * @param roi 区域of interest
     * @param PRF 雷达脉冲重复频率 (Hz)
     * @param R0 目标距离 (m)
     * @param incidenceAngle 入射角 (rad)
     * @param antennaGain 天线增益函数 (仅用于参考)
     * @return 距离模糊度比值
     */
    double calculateRASRInROI(const cv::Mat& image, const cv::Rect& roi, 
                             double PRF, double R0, double incidenceAngle,
                             const std::vector<double>& antennaGain = std::vector<double>());

    /**
     * @brief 获取分析结果描述
     * @return 分析结果的文字描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量
    double lastRASR;

    // 私有辅助方法
    double calculateRASRValue(const cv::Mat& image, double PRF, double R0, double incidenceAngle,
                             const std::vector<double>& antennaGain);
    double simulateAntennaGainPattern(double angle, const std::vector<double>& antennaGain);
    double calculateBackscatterCoefficient(const cv::Mat& image, int x, int y);
};

} // namespace Analysis
} // namespace SAR

#endif // RASR_H 