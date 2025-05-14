#include "include/report_generator.h"
#include <QDateTime>
#include <QTextStream>
#include <QFileDialog>
#include <QPdfWriter>
#include <QFileInfo>
#include <QRegularExpression>
#include <QApplication>
#include <QStandardPaths>
#include <QTextDocument>
#include <QPageSize>
#include <QPrinter>
namespace SAR {
    namespace UI {
        ReportGenerator::ReportGenerator(QObject *parent, std::function<void(const QString &)> logCallback)
            : QObject(parent), logCallback(logCallback) {
        }

        ReportGenerator::~ReportGenerator() {
        }

        bool ReportGenerator::generateReport(const QString &format,
                                             const SAR::Core::AnalysisResult &result,
                                             const QString &outputPath) {
            if (format == "PDF") {
                return generatePDFReport(result, outputPath);
            } else if (format == "TXT") {
                return generateTXTReport(result, outputPath);
            } else {
                if (logCallback) {
                    logCallback(tr("不支持的报告格式：%1").arg(format));
                }
                return false;
            }
        }
        
        bool ReportGenerator::generateReport(const QString &format,
                                           const QMap<QString, QString> &results,
                                           const QString &imagePath,
                                           const QString &outputPath) {
            // 将旧格式结果转换为新的分析结果对象
            SAR::Core::AnalysisResult result = convertToAnalysisResult(results, imagePath);
            // 使用新接口生成报告
            return generateReport(format, result, outputPath);
        }

        bool ReportGenerator::generatePDFReport(const SAR::Core::AnalysisResult &result, const QString &outputPath) {
            // 如果没有指定输出路径，让用户选择
            QString finalOutputPath = outputPath;
            if (finalOutputPath.isEmpty()) {
                // 设置默认文件名：图像名称 + 日期时间
                QString defaultFileName = "SAR 质量报告_";
                if (!result.getImagePath().isEmpty()) {
                    QFileInfo imageInfo(result.getImagePath());
                    defaultFileName += imageInfo.baseName() + "_";
                }
                defaultFileName += QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".pdf";
                
                QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) 
                                     + "/" + defaultFileName;

                finalOutputPath = QFileDialog::getSaveFileName(QApplication::activeWindow(),
                                                               tr("保存 PDF 报告"),
                                                               defaultPath,
                                                               tr("PDF 文件 (*.pdf)"));
                if (finalOutputPath.isEmpty()) {
                    return false;
                }
            }

            // 创建表格所需的阈值参数
            QMap<QString, QPair<double, QString>> thresholds;
            thresholds["PSLR"] = QPair<double, QString>(-20.0, "dB");
            thresholds["ISLR"] = QPair<double, QString>(-13.0, "dB");
            thresholds["AzimuthResolution"] = QPair<double, QString>(20.0, "dB");
            thresholds["RangeResolution"] = QPair<double, QString>(20.0, "dB");
            thresholds["SNR"] = QPair<double, QString>(8.0, "dB");
            thresholds["NESZ"] = QPair<double, QString>(-19.0, "dB");
            thresholds["AbsoluteRadiometricAccuracy"] = QPair<double, QString>(1.5, "dB");
            thresholds["RelativeRadiometricAccuracy"] = QPair<double, QString>(1.0, "dB");
            thresholds["RadiometricResolution"] = QPair<double, QString>(3.5, "dB");
            thresholds["ENL"] = QPair<double, QString>(3.0, "");

            // 创建包含表格的HTML文档
            QString html = "<!DOCTYPE html><html><head>";
            html += "<meta charset='UTF-8'>";
            html += "<style>";
            html += "body { font-family: Arial, sans-serif; font-size: 10pt; }";
            html += "h1 { font-size: 16pt; text-align: center; margin: 20px 0; }";
            html += "h2 { font-size: 14pt; margin: 16px 0 8px 0; }";
            html += "p { margin: 6px 0; }";
            html += "table { width: 100%; border-collapse: collapse; margin-top: 15px; margin-bottom: 20px; table-layout: fixed; }";
            html += "th, td { padding: 6px; border: 1px solid black; text-align: center; word-wrap: break-word; }";
            html += "th { background-color: #f0f0f0; font-weight: bold; }";
            html += ".text-section { margin-top: 20px; page-break-before: always; }";
            html += ".result-item { margin: 8px 0; border-bottom: 1px solid #ccc; padding-bottom: 8px; }";
            html += ".success { color: green; }";
            html += ".failure { color: red; }";
            html += ".detail-list { margin-left: 20px; }";
            html += "</style>";
            html += "</head><body>";
            
            // 添加标题和基本信息
            html += "<h1>SAR 图像质量分析报告</h1>";
            html += "<p><b>分析时间：</b>" + result.getAnalysisTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";
            html += "<p><b>图像路径：</b>" + result.getImagePath() + "</p>";
            if (!result.getDescription().isEmpty()) {
                html += "<p><b>分析描述：</b>" + result.getDescription() + "</p>";
            }
            
            // 添加表格
            html += "<h2>表格概览</h2>";
            html += result.toTableHtml(thresholds);
            
            // 添加详细文本信息部分
            html += "<div class='text-section'>";
            html += "<h2>详细分析结果</h2>";
            
            // 添加各个分析结果的详细信息
            QMapIterator<QString, SAR::Core::AnalysisResultItem> i(result.getAllResults());
            while (i.hasNext()) {
                i.next();
                const SAR::Core::AnalysisResultItem& item = i.value();
                
                html += "<div class='result-item'>";
                html += "<h3>" + item.methodName + "</h3>";
                
                if (item.isSuccess) {
                    html += "<p><b>结果值：</b><span class='success'>" + QString::number(item.numericValue);
                    if (!item.unit.isEmpty()) {
                        html += " " + item.unit;
                    }
                    html += "</span></p>";
                    
                    if (!item.description.isEmpty()) {
                        html += "<p>" + item.description + "</p>";
                    }
                    
                    // 附加数值结果
                    if (!item.additionalValues.isEmpty()) {
                        html += "<p><b>详细数值：</b></p><ul class='detail-list'>";
                        QMapIterator<QString, double> j(item.additionalValues);
                        while (j.hasNext()) {
                            j.next();
                            html += "<li>" + j.key() + ": " + QString::number(j.value()) + "</li>";
                        }
                        html += "</ul>";
                    }
                    
                    // 附加信息
                    if (!item.additionalInfo.isEmpty()) {
                        html += "<p><b>附加信息：</b></p><ul class='detail-list'>";
                        QMapIterator<QString, QString> k(item.additionalInfo);
                        while (k.hasNext()) {
                            k.next();
                            // 过滤掉详细结果，因为可能包含HTML标签
                            if (k.key() != "详细结果") {
                                html += "<li>" + k.key() + ": " + k.value() + "</li>";
                            }
                        }
                        html += "</ul>";
                    }
                } else {
                    html += "<p><b>分析失败</b></p>";
                    html += "<p class='failure'><b>错误信息：</b>" + item.errorMessage + "</p>";
                }
                
                html += "</div>";
            }
            
            html += "</div>";
            html += "</body></html>";

            // 创建PDF文档
            QTextDocument document;
            document.setHtml(html);

            // 设置页面大小为 A4
            document.setPageSize(QSizeF(595, 842)); // 单位是点，A4大小

            // 创建打印机配置
            QPrinter printer(QPrinter::HighResolution);
            printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Point); // 减小边距以显示更多内容
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(finalOutputPath);
            printer.setPageSize(QPageSize(QPageSize::A4));
            printer.setCreator("SAR 质量分析工具");
            
            // 打印文档到 PDF
            document.print(&printer);

            if (logCallback) {
                logCallback(tr("PDF 报告已保存至：%1").arg(finalOutputPath));
            }

            return true;
        }

        bool ReportGenerator::generateTXTReport(const SAR::Core::AnalysisResult &result, const QString &outputPath) {
            // 如果没有指定输出路径，让用户选择
            QString finalOutputPath = outputPath;
            if (finalOutputPath.isEmpty()) {
                // 设置默认文件名：图像名称 + 日期时间
                QString defaultFileName = "SAR 质量报告_";
                if (!result.getImagePath().isEmpty()) {
                    QFileInfo imageInfo(result.getImagePath());
                    defaultFileName += imageInfo.baseName() + "_";
                }
                defaultFileName += QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt";
                
                QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) 
                                     + "/" + defaultFileName;
                
                finalOutputPath = QFileDialog::getSaveFileName(QApplication::activeWindow(),
                                                               tr("保存文本报告"),
                                                               defaultPath,
                                                               tr("文本文件 (*.txt)"));
                if (finalOutputPath.isEmpty()) {
                    return false;
                }
            }

            QFile file(finalOutputPath);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                if (logCallback) {
                    logCallback(tr("无法打开文件进行写入：%1").arg(finalOutputPath));
                }
                return false;
            }

            // 创建表格所需的阈值参数
            QMap<QString, QPair<double, QString>> thresholds;
            thresholds["PSLR"] = QPair<double, QString>(-20.0, "dB");
            thresholds["ISLR"] = QPair<double, QString>(-13.0, "dB");
            thresholds["AzimuthResolution"] = QPair<double, QString>(20.0, "dB");
            thresholds["RangeResolution"] = QPair<double, QString>(20.0, "dB");
            thresholds["SNR"] = QPair<double, QString>(8.0, "dB");
            thresholds["NESZ"] = QPair<double, QString>(-19.0, "dB");
            thresholds["AbsoluteRadiometricAccuracy"] = QPair<double, QString>(1.5, "dB");
            thresholds["RelativeRadiometricAccuracy"] = QPair<double, QString>(1.0, "dB");
            thresholds["RadiometricResolution"] = QPair<double, QString>(3.5, "dB");
            thresholds["ENL"] = QPair<double, QString>(3.0, "");

            QTextStream out(&file);
            
            // 添加标题和基本信息
            out << "SAR 图像质量分析报告\n";
            out << "====================================\n\n";
            out << "分析时间：" << result.getAnalysisTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
            out << "图像路径：" << result.getImagePath() << "\n";
            if (!result.getDescription().isEmpty()) {
                out << "分析描述：" << result.getDescription() << "\n";
            }
            out << "\n====================================\n\n";
            
            // 添加表格
            out << result.toTableText(thresholds);
            
            // 添加详细文本信息部分
            out << "\n\n====================================\n";
            out << "详细分析结果\n";
            out << "====================================\n\n";
            
            // 添加各个分析结果的详细信息
            QMapIterator<QString, SAR::Core::AnalysisResultItem> i(result.getAllResults());
            while (i.hasNext()) {
                i.next();
                const SAR::Core::AnalysisResultItem& item = i.value();
                
                out << "* " << item.methodName << "\n";
                out << "------------------------------------\n";
                
                if (item.isSuccess) {
                    out << "结果值：";
                    out << QString::number(item.numericValue);
                    if (!item.unit.isEmpty()) {
                        out << " " << item.unit;
                    }
                    out << "\n";
                    
                    if (!item.description.isEmpty()) {
                        out << item.description << "\n";
                    }
                    
                    // 附加数值结果
                    if (!item.additionalValues.isEmpty()) {
                        out << "详细数值：\n";
                        QMapIterator<QString, double> j(item.additionalValues);
                        while (j.hasNext()) {
                            j.next();
                            out << "  - " << j.key() << ": " << QString::number(j.value()) << "\n";
                        }
                    }
                    
                    // 附加信息
                    if (!item.additionalInfo.isEmpty()) {
                        out << "附加信息：\n";
                        QMapIterator<QString, QString> k(item.additionalInfo);
                        while (k.hasNext()) {
                            k.next();
                            // 过滤掉详细结果，因为可能包含HTML标签
                            if (k.key() != "详细结果") {
                                out << "  - " << k.key() << ": " << k.value() << "\n";
                            }
                        }
                    }
                } else {
                    out << "分析失败\n";
                    out << "错误信息：" << item.errorMessage << "\n";
                }
                
                out << "\n";
            }
            
            file.close();

            if (logCallback) {
                logCallback(tr("文本报告已保存至：%1").arg(finalOutputPath));
            }

            return true;
        }
        
        SAR::Core::AnalysisResult ReportGenerator::convertToAnalysisResult(
            const QMap<QString, QString> &results, 
            const QString &imagePath) {
            SAR::Core::AnalysisResult result;
            
            // 设置图像路径
            result.setImagePath(imagePath);
            
            // 遍历结果映射，并添加到分析结果对象中
            QMapIterator<QString, QString> i(results);
            while (i.hasNext()) {
                i.next();
                const QString &methodName = i.key();
                const QString &resultHtml = i.value();
                
                // 创建分析结果项
                SAR::Core::AnalysisResultItem item;
                item.methodName = methodName;
                item.isSuccess = true;  // 假设旧的结果都是成功的
                
                // 尝试从 HTML 中提取结果值
                QString resultValue;
                item.numericValue = extractValueFromResult(resultHtml, resultValue);
                
                // 添加详情作为附加信息
                item.additionalInfo["详细结果"] = resultHtml;
                
                // 添加到分析结果对象
                result.addResult(methodName, item);
            }
            
            return result;
        }

        QString ReportGenerator::getCurrentDateTime() {
            return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        }
        
        double ReportGenerator::extractValueFromResult(const QString &resultText) {
            QString dummy;
            return extractValueFromResult(resultText, dummy);
        }
        
        double ReportGenerator::extractValueFromResult(const QString &resultText, QString &resultValue) {
            // 尝试从结果 HTML 或文本中提取数值
            // 通常格式为 "结果值：XX.XX" 或类似格式
            QRegularExpression regex("结果值 [：:](.*?)(<|\n|$)");
            QRegularExpressionMatch match = regex.match(resultText);
            
            if (match.hasMatch()) {
                resultValue = match.captured(1).trimmed();
                
                // 提取数值
                QRegularExpression numberRegex("([\\-+]?[0-9]*\\.?[0-9]+)");
                QRegularExpressionMatch numberMatch = numberRegex.match(resultValue);
                
                if (numberMatch.hasMatch()) {
                    return numberMatch.captured(1).toDouble();
                }
            }
            
            return 0.0;
        }
    } // namespace UI
} // namespace SAR
