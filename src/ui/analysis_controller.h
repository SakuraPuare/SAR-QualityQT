#ifndef ANALYSIS_CONTROLLER_H
#define ANALYSIS_CONTROLLER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <functional>

// 前向声明
namespace cv {
class Mat;
}

namespace SAR {
namespace UI {

/**
 * @brief 分析控制器类，负责管理所有分析功能
 */
class AnalysisController {
public:
    /**
     * @brief 构造函数
     * @param logCallback 日志回调函数
     * @param progressCallback 进度回调函数
     * @param resultsCallback 结果回调函数
     */
    AnalysisController(
        const std::function<void(const QString&)>& logCallback,
        const std::function<void(int, const QString&)>& progressCallback,
        const std::function<void(const QString&, const QString&)>& resultsCallback);

    /**
     * @brief 执行分析
     * @param selectedMethods 选中的分析方法
     * @param imageData 图像数据
     * @return 是否分析成功
     */
    bool performAnalysis(const QStringList &selectedMethods, const cv::Mat &imageData);

    /**
     * @brief 获取所有分析结果
     * @return 分析结果映射
     */
    const QMap<QString, QString>& getResults() const;

    /**
     * @brief 清除所有分析结果
     */
    void clearResults();

    /**
     * @brief 获取当前日期时间字符串
     * @return 格式化的日期时间字符串
     */
    static QString getCurrentDateTime();

private:
    QMap<QString, QString> analysisResults; ///< 存储分析结果
    std::function<void(const QString&)> logCallback; ///< 日志回调函数
    std::function<void(int, const QString&)> progressCallback; ///< 进度回调函数
    std::function<void(const QString&, const QString&)> resultsCallback; ///< 结果回调函数

    /**
     * @brief 执行SNR分析
     * @param imageData 图像数据
     * @return 分析结果
     */
    QString performSNRAnalysis(const cv::Mat &imageData);

    /**
     * @brief 执行ISLR分析
     * @param imageData 图像数据
     * @return 分析结果
     */
    QString performISLRAnalysis(const cv::Mat &imageData);

    /**
     * @brief 执行PSLR分析
     * @param imageData 图像数据
     * @return 分析结果
     */
    QString performPSLRAnalysis(const cv::Mat &imageData);

    /**
     * @brief 执行RASR分析
     * @param imageData 图像数据
     * @return 分析结果
     */
    QString performRASRAnalysis(const cv::Mat &imageData);

    /**
     * @brief 执行AASR分析
     * @param imageData 图像数据
     * @return 分析结果
     */
    QString performAASRAnalysis(const cv::Mat &imageData);

    /**
     * @brief 执行NESZ分析
     * @param imageData 图像数据
     * @return 分析结果
     */
    QString performNESZAnalysis(const cv::Mat &imageData);

    /**
     * @brief 执行辐射精度分析
     * @param imageData 图像数据
     * @return 分析结果
     */
    QString performRadiometricAccuracyAnalysis(const cv::Mat &imageData);

    /**
     * @brief 执行辐射分辨率分析
     * @param imageData 图像数据
     * @return 分析结果
     */
    QString performRadiometricResolutionAnalysis(const cv::Mat &imageData);

    /**
     * @brief 执行ENL分析
     * @param imageData 图像数据
     * @return 分析结果
     */
    QString performENLAnalysis(const cv::Mat &imageData);
};

} // namespace UI
} // namespace SAR

#endif // ANALYSIS_CONTROLLER_H 