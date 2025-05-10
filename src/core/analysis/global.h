#ifndef GLOBAL_H
#define GLOBAL_H

#include <opencv2/core.hpp>
#include <QString>
#include <map>
#include <memory>

// 包含各种分析模块的头文件
#include "clarity.h"
#include "glcm.h"
#include "radiometric.h"
#include "snr.h"
#include "infocontent.h"

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

    /**
     * @brief 运行清晰度分析
     * @return 是否成功
     */
    bool runClarityAnalysis();

    /**
     * @brief 运行GLCM分析
     * @return 是否成功
     */
    bool runGLCMAnalysis();

    /**
     * @brief 运行辐射度分析
     * @return 是否成功
     */
    bool runRadiometricAnalysis();

    /**
     * @brief 运行SNR分析
     * @return 是否成功
     */
    bool runSNRAnalysis();

    /**
     * @brief 运行信息内容分析
     * @return 是否成功
     */
    bool runInfoContentAnalysis();

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

    // 各个分析模块的实例
    std::unique_ptr<Clarity> clarityAnalyzer;
    std::unique_ptr<GLCM> glcmAnalyzer;
    std::unique_ptr<Radiometric> radiometricAnalyzer;
    std::unique_ptr<SNR> snrAnalyzer;
    std::unique_ptr<InfoContent> infoContentAnalyzer;

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