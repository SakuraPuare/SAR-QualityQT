#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <QObject>
#include <QString>
#include <QMap>
#include <functional>
#include "../../core/include/analysis_result.h"

namespace SAR {
namespace UI {

class ReportGenerator : public QObject
{
    Q_OBJECT

public:
    explicit ReportGenerator(QObject *parent = nullptr, 
                           std::function<void(const QString&)> logCallback = nullptr);
    ~ReportGenerator();

    /**
     * @brief 生成分析报告
     * @param format 报告格式，如 "PDF" 或 "TXT"
     * @param result 分析结果
     * @param outputPath 输出路径，如果为空则由用户选择
     * @return 是否成功
     */
    bool generateReport(const QString &format, 
                      const SAR::Core::AnalysisResult &result,
                      const QString &outputPath = QString());
    
    /**
     * @brief 生成分析报告 (旧版接口，保留向后兼容性)
     * @param format 报告格式，如 "PDF" 或 "TXT"
     * @param results 分析结果映射
     * @param imagePath 图像路径
     * @param outputPath 输出路径，如果为空则由用户选择
     * @return 是否成功
     */
    bool generateReport(const QString &format, 
                      const QMap<QString, QString> &results,
                      const QString &imagePath,
                      const QString &outputPath = QString());

private:
    bool generatePDFReport(const SAR::Core::AnalysisResult &result, const QString &outputPath);
    bool generateTXTReport(const SAR::Core::AnalysisResult &result, const QString &outputPath);
    
    /**
     * @brief 将旧格式的结果转换为分析结果对象
     * @param results 旧格式的结果映射
     * @param imagePath 图像路径
     * @return 分析结果对象
     */
    SAR::Core::AnalysisResult convertToAnalysisResult(const QMap<QString, QString> &results, const QString &imagePath);
    
    QString getCurrentDateTime();
    QString generateReportHtml(const QMap<QString, QString> &results, const QString &imagePath);
    QString generateQualityTableHtml(const QMap<QString, QString> &results);
    QString generateQualityTable(const QMap<QString, QString> &results);
    double extractValueFromResult(const QString &resultText);
    double extractValueFromResult(const QString &resultText, QString &resultValue);
    
    std::function<void(const QString&)> logCallback;
};

} // namespace UI
} // namespace SAR

#endif // REPORT_GENERATOR_H 