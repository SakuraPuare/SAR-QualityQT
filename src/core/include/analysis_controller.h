#ifndef ANALYSIS_CONTROLLER_H
#define ANALYSIS_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QRect>
#include <functional>
#include "analysis_result.h"

namespace SAR {
namespace Core {

class ImageHandler;

/**
 * @brief 分析控制器类，处理所有的图像分析功能
 */
class AnalysisController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造分析控制器
     * @param parent 父对象
     * @param imageHandler 图像处理器实例
     * @param progressCallback 进度回调函数
     */
    explicit AnalysisController(QObject *parent = nullptr, 
                               ImageHandler *imageHandler = nullptr,
                               std::function<void(int, const QString&)> progressCallback = nullptr);
    ~AnalysisController();

    /**
     * @brief 执行所选的分析方法
     * @param selectedMethods 要执行的分析方法列表
     * @param imagePath 要分析的图像路径
     * @param roi 感兴趣区域（如果有）
     * @return 包含所有分析结果的结果集合
     */
    AnalysisResult performAnalysis(const QStringList &selectedMethods, 
                                  const QString &imagePath,
                                  const QRect &roi = QRect());

    /**
     * @brief 执行单个分析方法
     * @param methodName 要执行的分析方法名称
     * @param imagePath 要分析的图像路径
     * @param roi 感兴趣区域（如果有）
     * @return 分析结果项
     */
    AnalysisResultItem analyzeMethod(const QString &methodName, 
                                    const QString &imagePath,
                                    const QRect &roi = QRect());

signals:
    /**
     * @brief 分析进度信号
     * @param progress 进度百分比（0-100）
     * @param message 进度消息
     */
    void analysisProgress(int progress, const QString &message);

    /**
     * @brief 分析完成信号
     * @param results 分析结果
     */
    void analysisComplete(const AnalysisResult &results);

private:
    // 各种分析方法的实现
    AnalysisResultItem analyzeISLR(const QString &imagePath, const QRect &roi);
    AnalysisResultItem analyzePSLR(const QString &imagePath, const QRect &roi);
    AnalysisResultItem analyzeRASR(const QString &imagePath, const QRect &roi);
    AnalysisResultItem analyzeAASR(const QString &imagePath, const QRect &roi);
    AnalysisResultItem analyzeSNR(const QString &imagePath, const QRect &roi);
    AnalysisResultItem analyzeNESZ(const QString &imagePath, const QRect &roi);
    AnalysisResultItem analyzeRadiometricAccuracy(const QString &imagePath, const QRect &roi);
    AnalysisResultItem analyzeRadiometricResolution(const QString &imagePath, const QRect &roi);
    AnalysisResultItem analyzeENL(const QString &imagePath, const QRect &roi);

    ImageHandler *imageHandler;
    std::function<void(int, const QString&)> progressCallback;
};

} // namespace Core
} // namespace SAR

#endif // ANALYSIS_CONTROLLER_H 