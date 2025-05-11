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
    
    html += "<table border='1' cellspacing='0' cellpadding='5' style='border-collapse: collapse; width: 100%;'>";
    html += "<tr bgcolor='#f0f0f0'>";
    html += "<th>指标</th>";
    html += "<th>合格标准</th>";
    html += "<th>计算指标</th>";
    html += "<th>是否合格</th>";
    html += "</tr>";

    // 添加峰值旁瓣比
    if (results.contains("PSLR")) {
        const AnalysisResultItem& item = results["PSLR"];
        html += "<tr>";
        html += "<td>峰值旁瓣比</td>";
        html += "<td>&lt;-20.0dB</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            bool isPass = item.numericValue <= -20.0;
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        html += "</tr>";
    }

    // 添加积分旁瓣比
    if (results.contains("ISLR")) {
        const AnalysisResultItem& item = results["ISLR"];
        html += "<tr>";
        html += "<td>积分旁瓣比</td>";
        html += "<td>&lt;-13.0dB</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            bool isPass = item.numericValue <= -13.0;
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        html += "</tr>";
    }

    // 添加方位向模糊度
    if (results.contains("AzimuthResolution")) {
        const AnalysisResultItem& item = results["AzimuthResolution"];
        html += "<tr>";
        html += "<td>方位向模糊度</td>";
        html += "<td>&le;20dB</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            bool isPass = item.numericValue <= 20.0;
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        html += "</tr>";
    }

    // 添加距离向模糊度
    if (results.contains("RangeResolution")) {
        const AnalysisResultItem& item = results["RangeResolution"];
        html += "<tr>";
        html += "<td>距离向模糊度</td>";
        html += "<td>&le;20dB</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            bool isPass = item.numericValue <= 20.0;
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        html += "</tr>";
    }

    // 添加信噪比
    if (results.contains("SNR")) {
        const AnalysisResultItem& item = results["SNR"];
        html += "<tr>";
        html += "<td>信噪比</td>";
        html += "<td>&ge;8dB</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            bool isPass = item.numericValue >= 8.0;
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        html += "</tr>";
    }

    // 添加噪声等效散射系数
    if (results.contains("NESZ")) {
        const AnalysisResultItem& item = results["NESZ"];
        html += "<tr>";
        html += "<td>噪声等效散射系数</td>";
        html += "<td>分辨率 1-10m: &le;-19.0dB<br>分辨率 25-500m: &le;-21.0dB</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            bool isPass = true; // 根据实际分辨率判断
            if (item.additionalInfo.contains("分辨率")) {
                double resolution = item.additionalInfo["分辨率"].toDouble();
                if (resolution >= 1 && resolution <= 10) {
                    isPass = item.numericValue <= -19.0;
                } else if (resolution >= 25 && resolution <= 500) {
                    isPass = item.numericValue <= -21.0;
                }
            }
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        html += "</tr>";
    }

    // 添加绝对辐射精度
    if (results.contains("AbsoluteRadiometricAccuracy")) {
        const AnalysisResultItem& item = results["AbsoluteRadiometricAccuracy"];
        html += "<tr>";
        html += "<td>绝对辐射精度</td>";
        html += "<td>&le;1.5dB</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            bool isPass = item.numericValue <= 1.5;
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        html += "</tr>";
    }

    // 添加相对辐射精度
    if (results.contains("RelativeRadiometricAccuracy")) {
        const AnalysisResultItem& item = results["RelativeRadiometricAccuracy"];
        html += "<tr>";
        html += "<td>相对辐射精度</td>";
        html += "<td>&le;1.0dB</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            bool isPass = item.numericValue <= 1.0;
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        html += "</tr>";
    }

    // 添加辐射分辨率
    if (results.contains("RadiometricResolution")) {
        const AnalysisResultItem& item = results["RadiometricResolution"];
        html += "<tr>";
        html += "<td>辐射分辨率</td>";
        html += "<td>分辨率 1-10m: 3.5dB<br>分辨率 25-500m: 2.0dB</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + " " + item.unit + "</td>";
            bool isPass = true; // 根据实际分辨率判断
            if (item.additionalInfo.contains("分辨率")) {
                double resolution = item.additionalInfo["分辨率"].toDouble();
                if (resolution >= 1 && resolution <= 10) {
                    isPass = item.numericValue >= 3.5;
                } else if (resolution >= 25 && resolution <= 500) {
                    isPass = item.numericValue >= 2.0;
                }
            }
            html += "<td bgcolor='" + QString(isPass ? "#90EE90" : "#FFB6C1") + "'>" + 
                   (isPass ? "合格" : "不合格") + "</td>";
        } else {
            html += "<td colspan='2' bgcolor='#FFB6C1'>分析失败：" + item.errorMessage + "</td>";
        }
        html += "</tr>";
    }

    // 添加等效视数
    if (results.contains("ENL")) {
        const AnalysisResultItem& item = results["ENL"];
        html += "<tr>";
        html += "<td>等效视数</td>";
        html += "<td>&ge;3</td>";
        
        if (item.isSuccess) {
            html += "<td>" + QString::number(item.numericValue) + "</td>";
            bool isPass = item.numericValue >= 3.0;
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
    text += "指标                 | 合格标准                  | 计算指标         | 是否合格\n";
    text += QString("-").repeated(80) + "\n";

    // 添加峰值旁瓣比
    if (results.contains("PSLR")) {
        const AnalysisResultItem& item = results["PSLR"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            bool isPass = item.numericValue <= -20.0;
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("峰值旁瓣比").leftJustified(20) + "| ";
        text += QString("<-20.0dB").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
    }

    // 添加积分旁瓣比
    if (results.contains("ISLR")) {
        const AnalysisResultItem& item = results["ISLR"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            bool isPass = item.numericValue <= -13.0;
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("积分旁瓣比").leftJustified(20) + "| ";
        text += QString("<-13.0dB").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
    }

    // 添加方位向模糊度
    if (results.contains("AzimuthResolution")) {
        const AnalysisResultItem& item = results["AzimuthResolution"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            bool isPass = item.numericValue <= 20.0;
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("方位向模糊度").leftJustified(20) + "| ";
        text += QString("≤20dB").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
    }

    // 添加距离向模糊度
    if (results.contains("RangeResolution")) {
        const AnalysisResultItem& item = results["RangeResolution"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            bool isPass = item.numericValue <= 20.0;
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("距离向模糊度").leftJustified(20) + "| ";
        text += QString("≤20dB").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
    }

    // 添加信噪比
    if (results.contains("SNR")) {
        const AnalysisResultItem& item = results["SNR"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            bool isPass = item.numericValue >= 8.0;
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("信噪比").leftJustified(20) + "| ";
        text += QString("≥8dB").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
    }

    // 添加噪声等效散射系数
    if (results.contains("NESZ")) {
        const AnalysisResultItem& item = results["NESZ"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            bool isPass = true; // 根据实际分辨率判断
            if (item.additionalInfo.contains("分辨率")) {
                double resolution = item.additionalInfo["分辨率"].toDouble();
                if (resolution >= 1 && resolution <= 10) {
                    isPass = item.numericValue <= -19.0;
                } else if (resolution >= 25 && resolution <= 500) {
                    isPass = item.numericValue <= -21.0;
                }
            }
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("噪声等效散射系数").leftJustified(20) + "| ";
        text += QString("分辨率 1-10m: ≤-19.0dB").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
        text += QString(" ").repeated(20) + "| ";
        text += QString("分辨率 25-500m: ≤-21.0dB").leftJustified(25) + "| ";
        text += QString(" ").leftJustified(15) + "| ";
        text += QString(" ").leftJustified(10) + "\n";
    }

    // 添加绝对辐射精度
    if (results.contains("AbsoluteRadiometricAccuracy")) {
        const AnalysisResultItem& item = results["AbsoluteRadiometricAccuracy"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            bool isPass = item.numericValue <= 1.5;
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("绝对辐射精度").leftJustified(20) + "| ";
        text += QString("≤1.5dB").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
    }

    // 添加相对辐射精度
    if (results.contains("RelativeRadiometricAccuracy")) {
        const AnalysisResultItem& item = results["RelativeRadiometricAccuracy"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            bool isPass = item.numericValue <= 1.0;
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("相对辐射精度").leftJustified(20) + "| ";
        text += QString("≤1.0dB").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
    }

    // 添加辐射分辨率
    if (results.contains("RadiometricResolution")) {
        const AnalysisResultItem& item = results["RadiometricResolution"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue) + " " + item.unit;
            bool isPass = true; // 根据实际分辨率判断
            if (item.additionalInfo.contains("分辨率")) {
                double resolution = item.additionalInfo["分辨率"].toDouble();
                if (resolution >= 1 && resolution <= 10) {
                    isPass = item.numericValue >= 3.5;
                } else if (resolution >= 25 && resolution <= 500) {
                    isPass = item.numericValue >= 2.0;
                }
            }
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("辐射分辨率").leftJustified(20) + "| ";
        text += QString("分辨率 1-10m: 3.5dB").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
        text += QString(" ").repeated(20) + "| ";
        text += QString("分辨率 25-500m: 2.0dB").leftJustified(25) + "| ";
        text += QString(" ").leftJustified(15) + "| ";
        text += QString(" ").leftJustified(10) + "\n";
    }

    // 添加等效视数
    if (results.contains("ENL")) {
        const AnalysisResultItem& item = results["ENL"];
        QString calculatedValue;
        QString passStatus;
        
        if (item.isSuccess) {
            calculatedValue = QString::number(item.numericValue);
            bool isPass = item.numericValue >= 3.0;
            passStatus = isPass ? "合格" : "不合格";
        } else {
            calculatedValue = "分析失败";
            passStatus = "N/A";
        }
        
        text += QString("等效视数").leftJustified(20) + "| ";
        text += QString("≥3").leftJustified(25) + "| ";
        text += calculatedValue.leftJustified(15) + "| ";
        text += passStatus.leftJustified(10) + "\n";
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