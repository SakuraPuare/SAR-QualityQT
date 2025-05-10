#ifndef SNR_H
#define SNR_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>

namespace SAR {
namespace Analysis {

/**
 * @brief 信噪比分析类
 * 提供用于分析SAR图像信噪比的方法
 */
class SNR {
public:
    SNR();

    /**
     * @brief 计算图像的信噪比
     * @param image 输入图像
     * @return 信噪比值(dB)
     */
    double calculateSNR(const cv::Mat& image);

    /**
     * @brief 使用用户指定的区域计算信噪比
     * @param image 输入图像
     * @param signalROI 信号区域
     * @param noiseROI 噪声区域
     * @return 信噪比值(dB)
     */
    double calculateSNRWithROI(const cv::Mat& image, const cv::Rect& signalROI, const cv::Rect& noiseROI);

    /**
     * @brief 估计图像的噪声水平
     * @param image 输入图像
     * @return 估计的噪声水平
     */
    double estimateNoiseLevel(const cv::Mat& image);

    /**
     * @brief 获取分析结果描述
     * @return 分析结果的文字描述
     */
    QString getResultDescription() const;

private:
    // 私有成员变量
    double lastSNR;
    double lastNoiseLevel;

    // 私有辅助方法
    cv::Mat estimateSignalComponent(const cv::Mat& image);
    cv::Mat estimateNoiseComponent(const cv::Mat& image);
};

} // namespace Analysis
} // namespace SAR

#endif // SNR_H