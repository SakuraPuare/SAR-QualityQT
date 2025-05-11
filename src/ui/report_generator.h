#ifndef REPORT_GENERATOR_H
#define REPORT_GENERATOR_H

#include <QString>
#include <QMap>
#include <QStringList>
#include <functional>
#include "../core/include/analysis_result.h"

class QWidget;
class QObject;

namespace SAR {
    namespace UI {
        /**
         * @brief 报告生成器类，负责生成PDF和文本报告
         */
        class ReportGenerator : public QObject {
            Q_OBJECT

        public:
            /**
             * @brief 构造函数
             * @param parent 父窗口
             * @param logCallback 日志回调函数
             */
            ReportGenerator(QObject *parent, std::function<void(const QString &)> logCallback);

            /**
             * @brief 析构函数
             */
            ~ReportGenerator();

            /**
             * @brief 生成报告
             * @param format 报告格式 ("PDF" 或 "TXT")
             * @param result 分析结果
             * @param outputPath 输出路径
             * @return 是否生成成功
             */
            bool generateReport(const QString &format,
                                const SAR::Core::AnalysisResult &result,
                                const QString &outputPath);

            /**
             * @brief 获取当前日期时间字符串
             * @return 格式化的日期时间字符串
             */
            static QString getCurrentDateTime();

        private:
            QObject *parent; ///< 父窗口
            std::function<void(const QString &)> logCallback; ///< 日志回调函数

            /**
             * @brief 生成PDF报告
             * @param result 分析结果
             * @param outputPath 输出路径
             * @return 是否生成成功
             */
            bool generatePDFReport(const SAR::Core::AnalysisResult &result, const QString &outputPath);

            /**
             * @brief 生成文本报告
             * @param result 分析结果
             * @param outputPath 输出路径
             * @return 是否生成成功
             */
            bool generateTXTReport(const SAR::Core::AnalysisResult &result, const QString &outputPath);
        };
    } // namespace UI
} // namespace SAR

#endif // REPORT_GENERATOR_H
