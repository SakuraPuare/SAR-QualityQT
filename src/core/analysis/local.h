#ifndef LOCAL_H
#define LOCAL_H

#include <opencv2/core.hpp>
#include <QString>
#include <vector>
#include <map>
#include <memory>

// 包含配置文件
#include "analysis_config.h"

// 根据配置条件包含各种分析模块的头文件
#if CONFIG_ENABLE_CLARITY
#include "clarity.h"
#endif

#if CONFIG_ENABLE_GLCM
#include "glcm.h"
#endif

#if CONFIG_ENABLE_SNR || CONFIG_ENABLE_NESZ
#include "snr.h"
#endif

#if CONFIG_ENABLE_RADIOMETRIC_ACC || CONFIG_ENABLE_RADIOMETRIC_RES || CONFIG_ENABLE_ENL
#include "radiometric.h"
#endif

#if CONFIG_ENABLE_INFO_CONTENT
#include "infocontent.h"
#endif

namespace SAR {
namespace Analysis {

/**
 * @brief 局部分析类
 * 提供对图像局部区域的分析功能
 */
class Local {
public:
    Local();
    ~Local();

    /**
     * @brief 设置输入图像
     * @param image 要分析的图像
     */
    void setImage(const cv::Mat& image);

    /**
     * @brief 设置感兴趣区域
     * @param roi 感兴趣的区域
     */
    void setROI(const cv::Rect& roi);

    /**
     * @brief 添加多个感兴趣区域
     * @param rois 感兴趣区域列表
     */
    void addROIs(const std::vector<cv::Rect>& rois);

    /**
     * @brief 清除所有ROI
     */
    void clearROIs();

    /**
     * @brief 运行局部区域的所有分析
     * @return 是否成功完成所有分析
     */
    bool runAllAnalysis();

    /**
     * @brief 运行指定ROI的所有分析
     * @param roiIndex ROI索引
     * @return 是否成功
     */
    bool runAnalysisForROI(size_t roiIndex);

    /**
     * @brief 获取指定ROI的分析结果
     * @param roiIndex ROI索引
     * @return 包含该ROI所有分析结果的映射表
     */
    std::map<QString, double> getResultForROI(size_t roiIndex) const;

    /**
     * @brief 获取所有ROI的比较结果
     * @return 包含所有ROI比较结果的结构
     */
    QString getROIComparisonReport() const;

    /**
     * @brief 导出所有ROI的分析结果到CSV文件
     * @param filePath 导出文件路径
     * @return 是否成功导出
     */
    bool exportResultsToCSV(const QString& filePath) const;

    /**
     * @brief 获取分析的ROI图像
     * @param roiIndex ROI索引
     * @return ROI图像
     */
    cv::Mat getROIImage(size_t roiIndex) const;

private:
    // 私有成员变量
    cv::Mat inputImage;
    bool imageSet;
    std::vector<cv::Rect> regions;

    // 各个分析模块的实例，根据配置条件定义
#if CONFIG_ENABLE_CLARITY
    std::unique_ptr<Clarity> clarityAnalyzer;
#endif

#if CONFIG_ENABLE_GLCM
    std::unique_ptr<GLCM> glcmAnalyzer;
#endif

#if CONFIG_ENABLE_RADIOMETRIC_ACC || CONFIG_ENABLE_RADIOMETRIC_RES || CONFIG_ENABLE_ENL
    std::unique_ptr<Radiometric> radiometricAnalyzer;
#endif

#if CONFIG_ENABLE_SNR || CONFIG_ENABLE_NESZ
    std::unique_ptr<SNR> snrAnalyzer;
#endif

#if CONFIG_ENABLE_INFO_CONTENT
    std::unique_ptr<InfoContent> infoContentAnalyzer;
#endif

    // 分析结果，以 ROI 索引为键
    std::map<size_t, std::map<QString, std::map<QString, double>>> results;

    // 私有辅助方法
    void initializeAnalyzers();
    cv::Mat extractROI(const cv::Rect& roi) const;
    bool validateROI(const cv::Rect& roi) const;
    void clearResults();
};

} // namespace Analysis
} // namespace SAR

#endif // LOCAL_H