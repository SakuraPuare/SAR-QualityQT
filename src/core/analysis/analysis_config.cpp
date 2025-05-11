#include "analysis_config.h"

namespace SAR {
namespace Analysis {

AnalysisConfig::AnalysisConfig() {
    // 默认启用所有分析方法
    enabledMethods = 
        AnalysisMethod::ISLR | 
        AnalysisMethod::PSLR | 
        AnalysisMethod::RangeResolution | 
        AnalysisMethod::AzimuthResolution | 
        AnalysisMethod::RASR | 
        AnalysisMethod::AASR | 
        AnalysisMethod::SNR | 
        AnalysisMethod::NESZ | 
        AnalysisMethod::RadiometricAcc | 
        AnalysisMethod::RadiometricRes | 
        AnalysisMethod::ENL | 
        AnalysisMethod::Clarity | 
        AnalysisMethod::GLCM | 
        AnalysisMethod::InfoContent;
}

AnalysisConfig& AnalysisConfig::getInstance() {
    static AnalysisConfig instance;
    return instance;
}

bool AnalysisConfig::isMethodEnabled(AnalysisMethod method) const {
    return enabledMethods.testFlag(method);
}

void AnalysisConfig::enableMethod(AnalysisMethod method) {
    enabledMethods.setFlag(method, true);
}

void AnalysisConfig::disableMethod(AnalysisMethod method) {
    enabledMethods.setFlag(method, false);
}

void AnalysisConfig::setEnabledMethods(AnalysisMethods methods) {
    enabledMethods = methods;
}

AnalysisMethods AnalysisConfig::getEnabledMethods() const {
    return enabledMethods;
}

} // namespace Analysis
} // namespace SAR 