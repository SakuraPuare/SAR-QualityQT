#ifndef SAR_ANALYSIS_RESULT_ITEM_H
#define SAR_ANALYSIS_RESULT_ITEM_H

#include <QString>
#include <QMap>

namespace SAR {
namespace Core {

class AnalysisResultItem {
public:
    AnalysisResultItem()
        : isSuccess(true)
        , numericValue(0.0)
    {}

    bool isSuccess;                           // 分析是否成功
    double numericValue;                      // 分析结果数值
    QString unit;                             // 单位
    QString methodName;                       // 分析方法名称
    QString description;                      // 结果描述
    QString errorMessage;                     // 错误信息（如果分析失败）
    QMap<QString, double> additionalValues;   // 附加数值结果
    QMap<QString, QString> additionalInfo;    // 附加信息
};

} // namespace Core
} // namespace SAR

#endif // SAR_ANALYSIS_RESULT_ITEM_H 