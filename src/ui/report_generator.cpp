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

            QTextDocument document;
            document.setHtml(result.toHtml());

            // 设置页面大小为 A4
            document.setPageSize(QSizeF(595, 842)); // 单位是点

            // 创建打印机配置
            QPrinter printer(QPrinter::HighResolution);
            printer.setPageMargins(QMarginsF(30, 30, 30, 30), QPageLayout::Point);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(finalOutputPath);
            printer.setPageSize(QPageSize(QPageSize::A4));

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

            QTextStream out(&file);
            out << result.toPlainText();
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
