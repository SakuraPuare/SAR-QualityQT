#include "analysis_base.h"
#include "../include/logger.h"
#include <QDebug>

namespace SAR {
namespace Analysis {

AnalysisBase::AnalysisBase() {
}

AnalysisBase::~AnalysisBase() {
}

void AnalysisBase::setParameter(const QString& key, const QVariant& value) {
    parameters[key] = value;
}

QVariant AnalysisBase::getParameter(const QString& key, const QVariant& defaultValue) const {
    auto it = parameters.find(key);
    if (it != parameters.end()) {
        return it->second;
    }
    return defaultValue;
}

cv::Mat AnalysisBase::convertToFloat(const cv::Mat& image) const {
    return Utils::convertToFloat(image);
}

bool AnalysisBase::validateImage(const cv::Mat& image) const {
    return Utils::validateImage(image);
}

bool AnalysisBase::validateROI(const cv::Mat& image, const cv::Rect& roi) const {
    return Utils::validateROI(image, roi);
}

void AnalysisBase::log(const QString& message, int level) const {
    switch (level) {
    case 0: // 信息
        LOG_INFO("[Analysis] " + message);
        break;
    case 1: // 警告
        LOG_WARNING("[Analysis] " + message);
        break;
    case 2: // 错误
        LOG_ERROR("[Analysis] " + message);
        break;
    default:
        LOG_INFO("[Analysis] " + message);
    }
}

AnalysisBase::Result AnalysisBase::createFailureResult(const QString& errorMessage) const {
    Result result;
    result.isSuccess = false;
    result.errorMessage = errorMessage;
    return result;
}

AnalysisBase::Result AnalysisBase::createSuccessResult(double value, const QString& unit) const {
    Result result;
    result.isSuccess = true;
    result.numericValue = value;
    result.unit = unit;
    return result;
}

void AnalysisBase::warn(const QString& message) {
    LOG_WARNING("[Analysis] " + message);
}

void AnalysisBase::error(const QString& message) {
    LOG_ERROR("[Analysis] " + message);
}

} // namespace Analysis
} // namespace SAR 