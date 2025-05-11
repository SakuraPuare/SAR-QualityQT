#include "../include/analysis_result.h"
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>

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

bool AnalysisResult::isPassThreshold(double value, double threshold, const QString& methodName) const
{
    if (methodName.contains("旁瓣比") || methodName.contains("模糊度")) {
        return value <= threshold;
    } else {
        return value >= threshold;
    }
}

QString AnalysisResult::toTableHtml(const QMap<QString, QPair<double, QString>>& thresholds) const
{
    QString html;
    
    html += "<table border='1' cellspacing='0' cellpadding='5' style='border-collapse: collapse;'>";
    html += "<tr bgcolor='#f0f0f0'>";
    html += "<th>指标</th>";
    html += "<th>合格标准</th>";
    html += "<th>计算指标</th>";
    html += "<th>是否合格</th>";
    html += "</tr>";

    QMapIterator<QString, AnalysisResultItem> i(results);
    while (i.hasNext()) {
        i.next();
        const AnalysisResultItem& item = i.value();
        
        html += "<tr>";
        html += "<td>" + i.key() + "</td>";
        
        // 合格标准
        QString threshold;
        if (thresholds.contains(i.key())) {
            threshold = QString::number(thresholds[i.key()].first) + " " + thresholds[i.key()].second;
        } else {
            threshold = "未设置";
        }
        html += "<td>" + threshold + "</td>";
        
        // 计算指标
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            
            // 是否合格
            bool isPass = false;
            if (thresholds.contains(i.key())) {
                isPass = isPassThreshold(item.numericValue, thresholds[i.key()].first, i.key());
            }
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        
        html += "</tr>";
    }
    
    html += "</table>";
    return html;
}

QString AnalysisResult::toTableText(const QMap<QString, QPair<double, QString>>& thresholds) const
{
    QString text;
    
    // 表头
    text += QString("%-20s | %-15s | %-15s | %-10s\n").arg("指标", "合格标准", "计算指标", "是否合格");
    text += QString("-").repeated(70) + "\n";

    QMapIterator<QString, AnalysisResultItem> i(results);
    while (i.hasNext()) {
        i.next();
        const AnalysisResultItem& item = i.value();
        
        QString methodName = i.key();
        
        // 合格标准
        QString threshold;
        if (thresholds.contains(methodName)) {
            threshold = QString::number(thresholds[methodName].first) + " " + thresholds[methodName].second;
        } else {
            threshold = "未设置";
        }
        
        // 计算指标和是否合格
        QString calculatedValue;
        QString passStatus;
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            
            if (thresholds.contains(methodName)) {
                bool isPass = isPassThreshold(item.numericValue, thresholds[methodName].first, methodName);
                passStatus = isPass ? "合格" : "不合格";
            } else {
                passStatus = "未知";
            }
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("%-20s | %-15s | %-15s | %-10s\n")
                .arg(methodName, threshold, calculatedValue, passStatus);
    }
    
    return text;
}

bool AnalysisResult::toPdf(const QString& filePath, const QMap<QString, QPair<double, QString>>& thresholds) const
{
    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(30, 30, 30, 30));
    
    QPainter painter(&writer);
    if (!painter.isActive()) {
        return false;
    }
    
    QTextDocument doc;
    
    // 创建完整的 HTML 文档
    QString html = "<!DOCTYPE html><html><head>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; }";
    html += "table { width: 100%; border-collapse: collapse; }";
    html += "th, td { padding: 8px; border: 1px solid black; }";
    html += "th { background-color: #f0f0f0; }";
    html += "</style>";
    html += "</head><body>";
    
    // 添加标题和基本信息
    html += "<h1>SAR 图像质量分析报告</h1>";
    html += "<p><b>分析时间：</b>" + analysisTime.toString("yyyy-MM-dd hh:mm:ss") + "</p>";
    html += "<p><b>图像路径：</b>" + imagePath + "</p>";
    if (!description.isEmpty()) {
        html += "<p><b>分析描述：</b>" + description + "</p>";
    }
    html += "<br>";
    
    // 添加表格
    html += toTableHtml(thresholds);
    
    html += "</body></html>";
    
    doc.setHtml(html);
    doc.setPageSize(painter.viewport().size());
    doc.drawContents(&painter);
    
    return true;
}

} // namespace Core
} // namespace SAR 