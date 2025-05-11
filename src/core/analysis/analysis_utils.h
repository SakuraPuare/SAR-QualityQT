#ifndef ANALYSIS_UTILS_H
#define ANALYSIS_UTILS_H

// 移除循环依赖
// #include "analysis_base.h"
#include <QString>
#include <opencv2/core/mat.hpp> // For cv::Mat
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <QDebug>

namespace SAR {
namespace Analysis {

// 前向声明
class AnalysisBase;

namespace Utils {

/**
 * @brief 图像类型枚举
 */
enum class ImageType {
    Unknown,    // 未知类型
    Amplitude,  // 幅度图像
    Intensity,  // 强度图像
    Complex,    // 复数图像
    Phase,      // 相位图像
    RGB         // 彩色图像
};

/**
 * @brief 检测图像类型
 * @param image 输入图像
 * @return 检测到的图像类型
 */
ImageType detectImageType(const cv::Mat& image);

/**
 * @brief 将图像转换为32位浮点格式
 * @param image 输入图像
 * @return 转换后的图像
 */
cv::Mat convertToFloat(const cv::Mat& image);

/**
 * @brief 验证图像是否有效
 * @param image 输入图像
 * @return 是否有效
 */
bool validateImage(const cv::Mat& image);

/**
 * @brief 验证ROI区域是否有效
 * @param image 输入图像
 * @param roi ROI区域
 * @return 是否有效
 */
bool validateROI(const cv::Mat& image, const cv::Rect& roi);

/**
 * @brief 计算图像的统计信息
 * @param image 输入图像
 * @param mask 可选掩膜
 * @return 包含均值、方差、最小值、最大值的向量
 */
cv::Scalar calculateImageStats(const cv::Mat& image, const cv::Mat& mask = cv::Mat());

/**
 * @brief 估计图像噪声级别
 * @param image 输入图像
 * @return 估计的噪声标准差
 */
double estimateNoiseLevel(const cv::Mat& image);

/**
 * @brief 计算两个图像区域之间的比率（以dB为单位）
 * @param image 输入图像
 * @param region1 第一个区域
 * @param region2 第二个区域
 * @return 比率(dB)
 */
double calculateRatioDB(const cv::Mat& image, const cv::Rect& region1, const cv::Rect& region2);

/**
 * @brief 找到图像中的峰值位置
 * @param image 输入图像
 * @return 峰值位置
 */
cv::Point findPeak(const cv::Mat& image);

/**
 * @brief 计算图像的傅立叶变换
 * @param image 输入图像
 * @param shift 是否进行频移（将DC分量移至中心）
 * @return 复数频谱
 */
cv::Mat calculateFFT(const cv::Mat& image, bool shift = true);

/**
 * @brief 计算图像的功率谱
 * @param image 输入图像
 * @return 功率谱图像
 */
cv::Mat calculatePowerSpectrum(const cv::Mat& image);

/**
 * @brief 滤波器类型枚举
 */
enum class FilterType {
    LowPass,    // 低通滤波
    HighPass,   // 高通滤波
    BandPass,   // 带通滤波
    Median,     // 中值滤波
    Gaussian    // 高斯滤波
};

/**
 * @brief 应用滤波器到图像
 * @param image 输入图像
 * @param type 滤波器类型
 * @param param1 滤波器参数1
 * @param param2 滤波器参数2
 * @return 滤波后的图像
 */
cv::Mat applyFilter(const cv::Mat& image, FilterType type, double param1 = 0, double param2 = 0);

} // namespace Utils

// 前向声明 Result 结构体
namespace {
    struct Result;
}

// 结果结构体
struct AnalysisResult {
  QString detailedLog;     // 详细结果和日志
  QString overviewSummary; // 用于概览选项卡的简短摘要
  bool success = false;    // 分析是否成功完成
  QString analysisName;    // 分析方法的名称 (可选，用于日志)
};

// 声明独立的分析函数
// 移除对 AnalysisBase::Result 的引用，因为这会引入循环依赖
// ...

// 声明独立的辅助函数
cv::Mat prepareImageForGLCM(const cv::Mat &inputImage, QString &log);

void computeGLCM(const cv::Mat &img, cv::Mat &glcm, int dx, int dy, int levels,
                 bool symmetric, bool normalize, QString &log);

void calculateGLCMFeatures(const cv::Mat &glcm, int levels, double &contrast,
                           double &energy, double &homogeneity,
                           double &correlation, QString &log);

} // namespace Analysis
} // namespace SAR

#endif // ANALYSIS_UTILS_H