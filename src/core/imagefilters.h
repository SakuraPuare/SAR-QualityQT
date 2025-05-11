#ifndef IMAGEFILTERS_H
#define IMAGEFILTERS_H

#include <QString>
#include <opencv2/core/mat.hpp>

namespace SAR {
namespace Core {

/**
 * @brief 滤波器类型枚举
 */
enum class FilterType {
    LowPass,     // 低通滤波
    HighPass,    // 高通滤波
    BandPass,    // 带通滤波
    Median,      // 中值滤波
    Gaussian,    // 高斯滤波
    Bilateral,   // 双边滤波
    Lee,         // Lee滤波 (SAR图像专用)
    Frost,       // Frost滤波 (SAR图像专用)
    Kuan,        // Kuan滤波 (SAR图像专用)
    Custom       // 自定义滤波
};

/**
 * @brief 滤波器参数结构体
 */
struct FilterParameters {
    FilterType type = FilterType::Gaussian; // 滤波器类型
    int kernelSize = 3;          // 核大小
    double sigma = 1.0;          // 高斯/双边滤波的sigma值
    double param1 = 0.0;         // 通用参数1（如低通/高通滤波的半径）
    double param2 = 0.0;         // 通用参数2（如带通滤波的外半径）
    double damping = 1.0;        // Lee/Frost滤波的阻尼系数
    QString customFilterName;    // 自定义滤波器名称
};

/**
 * @brief 图像滤波器类
 * 提供各种滤波算法实现
 */
class ImageFilters {
public:
    /**
     * @brief 应用滤波器到图像
     * @param image 输入图像
     * @param params 滤波参数
     * @param log 可选的日志输出引用
     * @return 滤波后的图像
     */
    static cv::Mat applyFilter(const cv::Mat& image, 
                               const FilterParameters& params, 
                               QString* log = nullptr);

    /**
     * @brief 获取滤波器类型的描述信息
     * @param type 滤波器类型
     * @return 描述字符串
     */
    static QString getFilterTypeDescription(FilterType type);

private:
    // 低通滤波器实现
    static cv::Mat applyLowPassFilter(const cv::Mat& image, 
                                      const FilterParameters& params, 
                                      QString* log = nullptr);
    
    // 高通滤波器实现
    static cv::Mat applyHighPassFilter(const cv::Mat& image, 
                                       const FilterParameters& params, 
                                       QString* log = nullptr);
    
    // 带通滤波器实现
    static cv::Mat applyBandPassFilter(const cv::Mat& image, 
                                        const FilterParameters& params, 
                                        QString* log = nullptr);
    
    // 中值滤波器实现
    static cv::Mat applyMedianFilter(const cv::Mat& image, 
                                     const FilterParameters& params, 
                                     QString* log = nullptr);
    
    // 高斯滤波器实现
    static cv::Mat applyGaussianFilter(const cv::Mat& image, 
                                       const FilterParameters& params, 
                                       QString* log = nullptr);
    
    // 双边滤波器实现
    static cv::Mat applyBilateralFilter(const cv::Mat& image, 
                                        const FilterParameters& params, 
                                        QString* log = nullptr);
    
    // Lee滤波器实现
    static cv::Mat applyLeeFilter(const cv::Mat& image, 
                                  const FilterParameters& params, 
                                  QString* log = nullptr);
    
    // Frost滤波器实现
    static cv::Mat applyFrostFilter(const cv::Mat& image, 
                                    const FilterParameters& params, 
                                    QString* log = nullptr);
    
    // Kuan滤波器实现
    static cv::Mat applyKuanFilter(const cv::Mat& image, 
                                   const FilterParameters& params, 
                                   QString* log = nullptr);
    
    // 自定义滤波器实现
    static cv::Mat applyCustomFilter(const cv::Mat& image, 
                                     const FilterParameters& params, 
                                     QString* log = nullptr);
    
    // 记录日志的内部方法
    static void logMessage(QString* log, const QString& message);
    
    // 估计噪声方差的内部方法
    static double estimateNoiseVariance(const cv::Mat& image);
};

} // namespace Core
} // namespace SAR

#endif // IMAGEFILTERS_H 