#ifndef ANALYSIS_RESULT_H
#define ANALYSIS_RESULT_H

#include <QString>
#include <QMap>
#include <QDateTime>

namespace SAR {
namespace Core {

/**
 * @brief 分析结果项结构体，表示单个分析方法的结果
 */
struct AnalysisResultItem {
    QString methodName;       // 分析方法名称
    double numericValue;      // 主要数值结果
    QString unit;             // 单位
    QString description;      // 描述
    QMap<QString, double> additionalValues; // 附加数值结果
    QMap<QString, QString> additionalInfo;  // 附加信息
    bool isSuccess;           // 分析是否成功
    QString errorMessage;     // 如果分析失败，错误信息
    
    AnalysisResultItem() : numericValue(0.0), isSuccess(true) {}
};

/**
 * @brief 分析结果集合，包含所有分析方法的结果
 */
class AnalysisResult {
public:
    AnalysisResult();
    ~AnalysisResult();
    
    // 添加单个分析结果
    void addResult(const QString& methodName, const AnalysisResultItem& result);
    
    // 获取单个分析结果
    AnalysisResultItem getResult(const QString& methodName) const;
    
    // 检查是否存在指定方法的结果
    bool hasResult(const QString& methodName) const;
    
    // 获取所有结果
    QMap<QString, AnalysisResultItem> getAllResults() const;
    
    // 清除所有结果
    void clear();
    
    // 结果相关信息
    void setImagePath(const QString& path) { imagePath = path; }
    QString getImagePath() const { return imagePath; }
    
    void setAnalysisTime(const QDateTime& time) { analysisTime = time; }
    QDateTime getAnalysisTime() const { return analysisTime; }
    
    void setDescription(const QString& desc) { description = desc; }
    QString getDescription() const { return description; }
    
    // 转换为 HTML 和文本格式以用于显示和导出
    QString toHtml() const;
    QString toPlainText() const;

private:
    QMap<QString, AnalysisResultItem> results;
    QString imagePath;
    QDateTime analysisTime;
    QString description;
};

} // namespace Core
} // namespace SAR

#endif // ANALYSIS_RESULT_H 