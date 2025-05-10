#ifndef GLOBAL_H
#define GLOBAL_H

#include <opencv2/core.hpp>
#include <QString>
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
 * @brief 全局分析类
 * 集成了所有分析模块，提供对整个图像的全局分析功能
 */
class Global {
public:
    Global();
    ~Global();

    /**
     * @brief 设置输入图像
     * @param image 要分析的图像
     */
    void setImage(const cv::Mat& image);

    /**
     * @brief 运行所有分析
     * @return 是否成功完成所有分析
     */
    bool runAllAnalysis();

#if CONFIG_ENABLE_CLARITY
    /**
     * @brief 运行清晰度分析
     * @return 是否成功
     */
    bool runClarityAnalysis();
#endif

#if CONFIG_ENABLE_GLCM
    /**
     * @brief 运行GLCM分析
     * @return 是否成功
     */
    bool runGLCMAnalysis();
#endif

#if CONFIG_ENABLE_RADIOMETRIC_ACC || CONFIG_ENABLE_RADIOMETRIC_RES || CONFIG_ENABLE_ENL
    /**
     * @brief 运行辐射度分析
     * @return 是否成功
     */
    bool runRadiometricAnalysis();
#endif

#if CONFIG_ENABLE_SNR
    /**
     * @brief 运行SNR分析
     * @return 是否成功
     */
    bool runSNRAnalysis();
#endif

#if CONFIG_ENABLE_INFO_CONTENT
    /**
     * @brief 运行信息内容分析
     * @return 是否成功
     */
    bool runInfoContentAnalysis();
#endif

#if CONFIG_ENABLE_NESZ
    /**
     * @brief 运行噪声等效后向散射系数分析
     * @return 是否成功
     */
    bool runNESZAnalysis();
#endif

    /**
     * @brief 获取分析结果摘要
     * @return 包含所有分析结果的映射表
     */
    std::map<QString, double> getResultSummary() const;

    /**
     * @brief 获取分析结果的详细描述
     * @return 分析结果的详细文字描述
     */
    QString getDetailedResultDescription() const;

    /**
     * @brief 导出分析结果到CSV文件
     * @param filePath 导出文件路径
     * @return 是否成功导出
     */
    bool exportResultsToCSV(const QString& filePath) const;

private:
    // 私有成员变量
    cv::Mat inputImage;
    bool imageSet;

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

    // 分析结果
    std::map<QString, std::map<QString, double>> results;
    bool analysisComplete;

    // 私有辅助方法
    void initializeAnalyzers();
    void clearResults();
};

} // namespace Analysis
} // namespace SAR

#endif // GLOBAL_H