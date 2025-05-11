#include "report_generator.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QFileInfo>
#include <QDir>
#include <cmath>
#include <QRegularExpression>

namespace SAR {
namespace UI {

ReportGenerator::ReportGenerator(QWidget *parent, const std::function<void(const QString&)>& logCallback)
    : parent(parent), logCallback(logCallback) {
}

bool ReportGenerator::generateReport(const QString &format, const QMap<QString, QString> &results, const QString &imagePath) {
    // 检查是否有结果可以导出
    if (results.isEmpty()) {
        QMessageBox::warning(parent, QObject::tr("导出失败"),
                          QObject::tr("没有可以导出的分析结果"));
        return false;
    }

    // 获取保存路径
    QString defaultPath = QDir::homePath();
    QString defaultName = QFileInfo(imagePath).baseName() + "_" + 
                         QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    
    QString filePath;
    if (format.toLower() == "pdf") {
        filePath = QFileDialog::getSaveFileName(
            parent, QObject::tr("保存PDF报告"), defaultPath + "/" + defaultName + ".pdf",
            QObject::tr("PDF文件 (*.pdf)"));
            
        if (filePath.isEmpty()) return false;
        
        return generatePDFReport(filePath, results, imagePath);
    } else if (format.toLower() == "txt") {
        filePath = QFileDialog::getSaveFileName(
            parent, QObject::tr("保存文本报告"), defaultPath + "/" + defaultName + ".txt",
            QObject::tr("文本文件 (*.txt)"));
            
        if (filePath.isEmpty()) return false;
        
        return generateTextReport(filePath, results, imagePath);
    } else {
        QMessageBox::warning(parent, QObject::tr("导出失败"),
                          QObject::tr("不支持的导出格式: %1").arg(format));
        return false;
    }
}

QString ReportGenerator::getCurrentDateTime() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

bool ReportGenerator::generatePDFReport(const QString &filePath, const QMap<QString, QString> &results, const QString &imagePath) {
    try {
        logCallback(QObject::tr("正在生成PDF报告：%1").arg(filePath));
        
        // 创建PDF写入器
        QPdfWriter pdfWriter(filePath);
        pdfWriter.setPageSize(QPageSize(QPageSize::A4));
        pdfWriter.setPageMargins(QMarginsF(30, 30, 30, 30));
        pdfWriter.setTitle(QObject::tr("SAR图像质量评估报告"));
        
        // 创建绘图器
        QPainter painter(&pdfWriter);
        painter.setPen(Qt::black);
        
        // 创建文本文档并设置HTML内容
        QTextDocument document;
        document.setDefaultFont(QFont("Arial", 10));
        
        // 生成HTML报告
        QString html = generateReportHtml(results, imagePath);
        document.setHtml(html);
        
        // 设置文档页面大小与PDF页面大小匹配
        document.setPageSize(pdfWriter.pageRect().size());
        
        // 绘制文档到PDF
        document.drawContents(&painter);
        
        // 完成绘制
        painter.end();
        
        logCallback(QObject::tr("PDF报告已生成：%1").arg(filePath));
        return true;
    } catch (const std::exception &e) {
        logCallback(QObject::tr("生成PDF报告时发生错误：%1").arg(e.what()));
        QMessageBox::critical(parent, QObject::tr("导出失败"),
                            QObject::tr("生成PDF报告时发生错误：%1").arg(e.what()));
        return false;
    } catch (...) {
        logCallback(QObject::tr("生成PDF报告时发生未知错误"));
        QMessageBox::critical(parent, QObject::tr("导出失败"),
                            QObject::tr("生成PDF报告时发生未知错误"));
        return false;
    }
}

bool ReportGenerator::generateTextReport(const QString &filePath, const QMap<QString, QString> &results, const QString &imagePath) {
    try {
        logCallback(QObject::tr("正在生成文本报告：%1").arg(filePath));
        
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            logCallback(QObject::tr("无法打开文件进行写入：%1").arg(filePath));
            QMessageBox::critical(parent, QObject::tr("导出失败"),
                               QObject::tr("无法打开文件进行写入：%1").arg(filePath));
            return false;
        }
        
        QTextStream out(&file);
        out.setCodec("UTF-8");
        
        // 写入报告标题
        out << QObject::tr("SAR图像质量评估报告") << "\n";
        out << QObject::tr("====================") << "\n\n";
        
        // 写入报告日期和时间
        out << QObject::tr("报告生成时间：%1").arg(getCurrentDateTime()) << "\n\n";
        
        // 写入图像信息
        out << QObject::tr("图像文件：%1").arg(QFileInfo(imagePath).fileName()) << "\n";
        out << QObject::tr("图像路径：%1").arg(imagePath) << "\n\n";
        
        // 写入质量指标结果表格
        out << generateQualityTable(results) << "\n\n";
        
        // 写入详细结果
        out << QObject::tr("详细分析结果") << "\n";
        out << QObject::tr("==============") << "\n\n";
        
        for (auto it = results.begin(); it != results.end(); ++it) {
            out << QObject::tr("[%1]").arg(it.key()) << "\n";
            out << QObject::tr("----------------") << "\n";
            out << it.value() << "\n\n";
        }
        
        file.close();
        logCallback(QObject::tr("文本报告已生成：%1").arg(filePath));
        return true;
    } catch (const std::exception &e) {
        logCallback(QObject::tr("生成文本报告时发生错误：%1").arg(e.what()));
        QMessageBox::critical(parent, QObject::tr("导出失败"),
                            QObject::tr("生成文本报告时发生错误：%1").arg(e.what()));
        return false;
    } catch (...) {
        logCallback(QObject::tr("生成文本报告时发生未知错误"));
        QMessageBox::critical(parent, QObject::tr("导出失败"),
                            QObject::tr("生成文本报告时发生未知错误"));
        return false;
    }
}

QString ReportGenerator::generateReportHtml(const QMap<QString, QString> &results, const QString &imagePath) {
    QString html = "<html><head>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; }";
    html += "h1 { color: #1a5276; text-align: center; }";
    html += "h2 { color: #2874a6; margin-top: 20px; }";
    html += "table { width: 100%; border-collapse: collapse; margin: 15px 0; }";
    html += "th, td { padding: 8px; text-align: left; border: 1px solid #ddd; }";
    html += "th { background-color: #f2f2f2; }";
    html += "tr:nth-child(even) { background-color: #f9f9f9; }";
    html += ".result-section { margin: 20px 0; padding: 10px; border: 1px solid #ddd; background-color: #f9f9f9; }";
    html += ".result-title { font-weight: bold; color: #2874a6; margin-bottom: 10px; }";
    html += ".result-content { white-space: pre-wrap; font-family: monospace; }";
    html += "</style>";
    html += "</head><body>";
    
    // 标题
    html += "<h1>SAR图像质量评估报告</h1>";
    
    // 报告信息
    html += "<p><b>报告生成时间：</b>" + getCurrentDateTime() + "</p>";
    html += "<p><b>图像文件：</b>" + QFileInfo(imagePath).fileName() + "</p>";
    html += "<p><b>图像路径：</b>" + imagePath + "</p>";
    
    // 质量指标结果表格
    html += "<h2>质量指标汇总</h2>";
    html += generateQualityTableHtml(results);
    
    // 详细结果
    html += "<h2>详细分析结果</h2>";
    
    for (auto it = results.begin(); it != results.end(); ++it) {
        html += "<div class='result-section'>";
        html += "<div class='result-title'>" + it.key() + "</div>";
        
        // 将结果文本中的换行符替换为 <br> 标签，以便在 HTML 中正确显示
        QString resultText = it.value();
        resultText.replace("\n", "<br>");
        
        html += "<div class='result-content'>" + resultText + "</div>";
        html += "</div>";
    }
    
    html += "</body></html>";
    return html;
}

QString ReportGenerator::generateQualityTableHtml(const QMap<QString, QString> &results) {
    QString html = "<table>";
    html += "<tr><th>质量指标</th><th>结果值</th><th>单位</th><th>符合标准</th></tr>";
    
    // 定义质量标准阈值（示例值，实际应根据具体要求设置）
    const double SNR_THRESHOLD = 20.0;     // dB
    const double ISLR_THRESHOLD = -20.0;   // dB
    const double PSLR_THRESHOLD = -30.0;   // dB
    const double RASR_THRESHOLD = 0.05;    // 无量纲
    const double AASR_THRESHOLD = 0.05;    // 无量纲
    const double NESZ_THRESHOLD = -25.0;   // dB
    
    for (auto it = results.begin(); it != results.end(); ++it) {
        const QString &method = it.key();
        const QString &resultText = it.value();
        
        QString resultValue;
        double value = extractValueFromResult(resultText, resultValue);
        
        QString unit;
        bool passes = false;
        
        if (method == "SNR") {
            unit = "dB";
            passes = (value >= SNR_THRESHOLD);
        } else if (method == "ISLR") {
            unit = "dB";
            passes = (value <= ISLR_THRESHOLD);
        } else if (method == "PSLR") {
            unit = "dB";
            passes = (value <= PSLR_THRESHOLD);
        } else if (method == "RASR") {
            unit = "";
            passes = (value <= RASR_THRESHOLD);
        } else if (method == "AASR") {
            unit = "";
            passes = (value <= AASR_THRESHOLD);
        } else if (method == "NESZ") {
            unit = "dB";
            passes = (value <= NESZ_THRESHOLD);
        } else if (method == "RadiametricAccuracy") {
            unit = "dB";
            passes = true; // 假设该指标无明确阈值
        } else if (method == "RadiometricResolution") {
            unit = "dB";
            passes = true; // 假设该指标无明确阈值
        } else if (method == "ENL") {
            unit = "";
            passes = true; // 假设该指标无明确阈值
        } else {
            unit = "";
            passes = true; // 为未知指标，暂定为通过
        }
        
        html += "<tr>";
        html += "<td>" + method + "</td>";
        html += "<td>" + resultValue + "</td>";
        html += "<td>" + unit + "</td>";
        html += "<td>" + (passes ? "✓" : "✗") + "</td>";
        html += "</tr>";
    }
    
    html += "</table>";
    return html;
}

QString ReportGenerator::generateQualityTable(const QMap<QString, QString> &results) {
    QString table;
    
    // 表头
    table += QObject::tr("质量指标\t结果值\t单位\t符合标准\n");
    table += QObject::tr("------------------------------------------------\n");
    
    // 定义质量标准阈值（示例值，实际应根据具体要求设置）
    const double SNR_THRESHOLD = 20.0;     // dB
    const double ISLR_THRESHOLD = -20.0;   // dB
    const double PSLR_THRESHOLD = -30.0;   // dB
    const double RASR_THRESHOLD = 0.05;    // 无量纲
    const double AASR_THRESHOLD = 0.05;    // 无量纲
    const double NESZ_THRESHOLD = -25.0;   // dB
    
    for (auto it = results.begin(); it != results.end(); ++it) {
        const QString &method = it.key();
        const QString &resultText = it.value();
        
        QString resultValue;
        double value = extractValueFromResult(resultText, resultValue);
        
        QString unit;
        bool passes = false;
        
        if (method == "SNR") {
            unit = "dB";
            passes = (value >= SNR_THRESHOLD);
        } else if (method == "ISLR") {
            unit = "dB";
            passes = (value <= ISLR_THRESHOLD);
        } else if (method == "PSLR") {
            unit = "dB";
            passes = (value <= PSLR_THRESHOLD);
        } else if (method == "RASR") {
            unit = "";
            passes = (value <= RASR_THRESHOLD);
        } else if (method == "AASR") {
            unit = "";
            passes = (value <= AASR_THRESHOLD);
        } else if (method == "NESZ") {
            unit = "dB";
            passes = (value <= NESZ_THRESHOLD);
        } else if (method == "RadiametricAccuracy") {
            unit = "dB";
            passes = true; // 假设该指标无明确阈值
        } else if (method == "RadiometricResolution") {
            unit = "dB";
            passes = true; // 假设该指标无明确阈值
        } else if (method == "ENL") {
            unit = "";
            passes = true; // 假设该指标无明确阈值
        } else {
            unit = "";
            passes = true; // 为未知指标，暂定为通过
        }
        
        table += method + "\t" + resultValue + "\t" + unit + "\t" + 
                (passes ? QObject::tr("是") : QObject::tr("否")) + "\n";
    }
    
    return table;
}

double ReportGenerator::extractValueFromResult(const QString &resultText) {
    QString resultValue;
    return extractValueFromResult(resultText, resultValue);
}

double ReportGenerator::extractValueFromResult(const QString &resultText, QString &resultValue) {
    // 默认值
    double value = 0.0;
    resultValue = "N/A";
    
    // 尝试使用正则表达式提取数值
    QRegularExpression re("结果：.*?([+-]?\\d*\\.?\\d+)");
    QRegularExpressionMatch match = re.match(resultText);
    
    if (match.hasMatch()) {
        resultValue = match.captured(1);
        bool ok;
        value = resultValue.toDouble(&ok);
        if (!ok) {
            value = 0.0;
            resultValue = "N/A";
        }
    } else {
        // 尝试另一种格式
        re = QRegularExpression("=\\s*([+-]?\\d*\\.?\\d+)");
        match = re.match(resultText);
        
        if (match.hasMatch()) {
            resultValue = match.captured(1);
            bool ok;
            value = resultValue.toDouble(&ok);
            if (!ok) {
                value = 0.0;
                resultValue = "N/A";
            }
        }
    }
    
    return value;
}

} // namespace UI
} // namespace SAR 