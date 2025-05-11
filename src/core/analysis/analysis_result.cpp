#include "../include/analysis_result.h"

namespace SAR {
namespace Core {

AnalysisResult::AnalysisResult()
    : analysisTime(QDateTime::currentDateTime())
{
}

AnalysisResult::~AnalysisResult()
{
}

void AnalysisResult::addResult(const QString& methodName, const AnalysisResultItem& result)
{
    results[methodName] = result;
}

AnalysisResultItem AnalysisResult::getResult(const QString& methodName) const
{
    if (results.contains(methodName)) {
        return results[methodName];
    }
    return AnalysisResultItem();
}

bool AnalysisResult::hasResult(const QString& methodName) const
{
    return results.contains(methodName);
}

QMap<QString, AnalysisResultItem> AnalysisResult::getAllResults() const
{
    return results;
}

void AnalysisResult::clear()
{
    results.clear();
    imagePath.clear();
    description.clear();
    analysisTime = QDateTime::currentDateTime();
}

QString AnalysisResult::toHtml() const
{
    QString html;

    // 添加 HTML 头部和 CSS 样式
    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<title>SAR 图像质量分析告</title>";
    html += "<style>";
    // 在这里添加 CSS 样式来增大字体
    // 示例：将所有段落和列表项的字体大小设置为 1.2em
    html += "body { font-family: sans-serif; }"; // 可选：设置一个通用的字体
    html += "p, li { font-size: 1.2em; line-height: 1.5; }"; // 增大字体大小和行高
    html += "h2 { font-size: 1.8em; margin-top: 20px; }"; // 增大标题字体
    html += "h3 { font-size: 1.4em; margin-top: 15px; }"; // 增大子标题字体
    html += "b { font-weight: bold; }"; // 加粗文本
    html += "hr { border: none; height: 1px; background-color: #ccc; margin: 20px 0; }"; // 分隔线样式
    html += "</style>";
    html += "</head>";
    html += "<body>"; // HTML 内容放在 body 标签内

    // 标题和基本信息
    html += "<h2>SAR 图像质量分析报告</h2>";
    html += "<p><b>分析时间：</b>" + analysisTime.toString("yyyy-MM-dd hh:mm:ss") + "</p>";
    html += "<p><b>图像路径：</b>" + imagePath + "</p>";
    if (!description.isEmpty()) {
        html += "<p><b>分析描述：</b>" + description + "</p>";
    }

    html += "<hr/>";

    // 各个分析结果
    QMapIterator<QString, AnalysisResultItem> i(results);
    while (i.hasNext()) {
        i.next();
        const AnalysisResultItem& item = i.value();

        html += "<h3>" + item.methodName + "</h3>";

        if (item.isSuccess) {
            html += "<p><b>结果值：</b>" + QString::number(item.numericValue);
            if (!item.unit.isEmpty()) {
                html += " " + item.unit;
            }
            html += "</p>";

            if (!item.description.isEmpty()) {
                html += "<p>" + item.description + "</p>";
            }

            // 附加数值结果
            if (!item.additionalValues.isEmpty()) {
                html += "<p><b>详细数值：</b></p><ul>";
                QMapIterator<QString, double> j(item.additionalValues);
                while (j.hasNext()) {
                    j.next();
                    html += "<li>" + j.key() + ": " + QString::number(j.value()) + "</li>";
                }
                html += "</ul>";
            }

            // 附加信息
            if (!item.additionalInfo.isEmpty()) {
                html += "<p><b>附加信息：</b></p><ul>";
                QMapIterator<QString, QString> k(item.additionalInfo);
                while (k.hasNext()) {
                    k.next();
                    html += "<li>" + k.key() + ": " + k.value() + "</li>";
                }
                html += "</ul>";
            }
        } else {
            html += "<p><b>分析失败</b></p>";
            html += "<p><b>错误信息：</b>" + item.errorMessage + "</p>";
        }

        html += "<hr/>";
    }

    html += "</body>"; // 结束 body 标签
    html += "</html>"; // 结束 html 标签

    return html;
}

QString AnalysisResult::toPlainText() const
{
    QString text;
    
    // 标题和基本信息
    text += "SAR 图像质量分析报告\n";
    text += "分析时间：" + analysisTime.toString("yyyy-MM-dd hh:mm:ss") + "\n";
    text += "图像路径：" + imagePath + "\n";
    if (!description.isEmpty()) {
        text += "分析描述：" + description + "\n";
    }
    
    text += "\n----------------------------------------\n\n";
    
    // 各个分析结果
    QMapIterator<QString, AnalysisResultItem> i(results);
    while (i.hasNext()) {
        i.next();
        const AnalysisResultItem& item = i.value();
        
        text += item.methodName + "\n";
        
        if (item.isSuccess) {
            text += "结果值：" + QString::number(item.numericValue);
            if (!item.unit.isEmpty()) {
                text += " " + item.unit;
            }
            text += "\n";
            
            if (!item.description.isEmpty()) {
                text += item.description + "\n";
            }
            
            // 附加数值结果
            if (!item.additionalValues.isEmpty()) {
                text += "详细数值：\n";
                QMapIterator<QString, double> j(item.additionalValues);
                while (j.hasNext()) {
                    j.next();
                    text += "  - " + j.key() + ": " + QString::number(j.value()) + "\n";
                }
            }
            
            // 附加信息
            if (!item.additionalInfo.isEmpty()) {
                text += "附加信息：\n";
                QMapIterator<QString, QString> k(item.additionalInfo);
                while (k.hasNext()) {
                    k.next();
                    text += "  - " + k.key() + ": " + k.value() + "\n";
                }
            }
        } else {
            text += "分析失败\n";
            text += "错误信息：" + item.errorMessage + "\n";
        }
        
        text += "\n----------------------------------------\n\n";
    }
    
    return text;
}

} // namespace Core
} // namespace SAR 