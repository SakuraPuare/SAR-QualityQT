#include "analysis_factory.h"
#include "../include/analysis_result.h"
#include <QStringList>

namespace SAR {
namespace Analysis {

AnalysisFactory& AnalysisFactory::getInstance() {
    static AnalysisFactory instance;
    return instance;
}

AnalysisFactory::AnalysisFactory() {
    // 构造函数，初始化
}

void AnalysisFactory::registerAnalysis(const QString& methodName, AnalysisCreator creator) {
    creators[methodName] = creator;
}

void AnalysisFactory::registerExecutor(const QString& methodName, AnalysisExecutor executor) {
    executors[methodName] = executor;
}

AnalysisBase* AnalysisFactory::createAnalysis(const QString& methodName) {
    auto it = creators.find(methodName);
    if (it != creators.end()) {
        return it->second();
    }
    return nullptr;
}

SAR::Core::AnalysisResultItem AnalysisFactory::executeAnalysis(const QString& methodName, 
                                                 const QString& imagePath, 
                                                 const QRect& roi) {
    auto it = executors.find(methodName);
    if (it != executors.end()) {
        return it->second(imagePath, roi);
    }
    
    // 方法不存在时返回错误结果
    SAR::Core::AnalysisResultItem errorResult;
    errorResult.methodName = methodName;
    errorResult.isSuccess = false;
    errorResult.errorMessage = QString("不支持的分析方法：%1").arg(methodName);
    return errorResult;
}

bool AnalysisFactory::hasAnalysisMethod(const QString& methodName) const {
    return executors.find(methodName) != executors.end();
}

QStringList AnalysisFactory::getAvailableMethods() const {
    QStringList methodList;
    for (const auto& pair : executors) {
        methodList.append(pair.first);
    }
    return methodList;
}

} // namespace Analysis
} // namespace SAR 