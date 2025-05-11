#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <QString>
#include <QMap>
#include <QStringList>
#include <functional>

class QWidget;

namespace SAR {
namespace UI {

/**
 * @brief 报告生成器类，负责生成PDF和文本报告
 */
class ReportGenerator {
public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     * @param logCallback 日志回调函数
     */
    ReportGenerator(QWidget *parent, const std::function<void(const QString&)>& logCallback);

    /**
     * @brief 生成报告
     * @param format 报告格式 ("pdf" 或 "txt")
     * @param results 分析结果
     * @param imagePath 图像路径
     * @return 是否生成成功
     */
    bool generateReport(const QString &format, const QMap<QString, QString> &results, const QString &imagePath);

    /**
     * @brief 获取当前日期时间字符串
     * @return 格式化的日期时间字符串
     */
    static QString getCurrentDateTime();

private:
    QWidget *parent; ///< 父窗口
    std::function<void(const QString&)> logCallback; ///< 日志回调函数

    /**
     * @brief 生成PDF报告
     * @param filePath 文件路径
     * @param results 分析结果
     * @param imagePath 图像路径
     * @return 是否生成成功
     */
    bool generatePDFReport(const QString &filePath, const QMap<QString, QString> &results, const QString &imagePath);

    /**
     * @brief 生成文本报告
     * @param filePath 文件路径
     * @param results 分析结果
     * @param imagePath 图像路径
     * @return 是否生成成功
     */
    bool generateTextReport(const QString &filePath, const QMap<QString, QString> &results, const QString &imagePath);

    /**
     * @brief 生成HTML报告内容
     * @param results 分析结果
     * @param imagePath 图像路径
     * @return HTML内容
     */
    QString generateReportHtml(const QMap<QString, QString> &results, const QString &imagePath);

    /**
     * @brief 生成HTML质量指标表格
     * @param results 分析结果
     * @return HTML表格内容
     */
    QString generateQualityTableHtml(const QMap<QString, QString> &results);

    /**
     * @brief 生成文本质量指标表格
     * @param results 分析结果
     * @return 文本表格内容
     */
    QString generateQualityTable(const QMap<QString, QString> &results);

    /**
     * @brief 从结果文本中提取指标值
     * @param resultText 结果文本
     * @return 提取的指标值
     */
    double extractValueFromResult(const QString &resultText);

    /**
     * @brief 从结果文本中提取指标值并返回字符串
     * @param resultText 结果文本
     * @param resultValue 输出参数，提取的结果字符串
     * @return 提取的指标值
     */
    double extractValueFromResult(const QString &resultText, QString &resultValue);
};

} // namespace UI
} // namespace SAR

#endif // REPORT_GENERATOR_H 