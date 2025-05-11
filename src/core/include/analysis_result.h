#ifndef SAR_ANALYSIS_RESULT_H
#define SAR_ANALYSIS_RESULT_H

#include <QDateTime>
#include <QMap>
#include <QString>
#include "analysis_result_item.h"

namespace SAR {
namespace Core {

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
    
    // 输出方法
    QString toHtml() const;
    QString toPlainText() const;
    QString toTableHtml(const QMap<QString, QPair<double, QString>>& thresholds) const;
    QString toTableText(const QMap<QString, QPair<double, QString>>& thresholds) const;
    bool toPdf(const QString& filePath, const QMap<QString, QPair<double, QString>>& thresholds) const;
    
    // 属性访问器
    QString getImagePath() const { return imagePath; }
    void setImagePath(const QString& path) { imagePath = path; }
    QString getDescription() const { return description; }
    void setDescription(const QString& desc) { description = desc; }
    QDateTime getAnalysisTime() const { return analysisTime; }
    void setAnalysisTime(const QDateTime& time) { analysisTime = time; }

private:
    QMap<QString, AnalysisResultItem> results;
    QString imagePath;
    QString description;
    QDateTime analysisTime;

    // 辅助方法
    bool isPassThreshold(double value, double threshold, const QString& methodName) const;
};

} // namespace Core
} // namespace SAR

#endif // SAR_ANALYSIS_RESULT_H 