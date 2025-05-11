#include "analysis_controller.h"
#include <QDateTime>
#include <opencv2/core.hpp>

// SAR 分析模块头文件
#include "../core/analysis/snr.h"
#include "../core/analysis/islr.h"
#include "../core/analysis/pslr.h"
#include "../core/analysis/rasr.h"
#include "../core/analysis/aasr.h"
#include "../core/analysis/nesz.h"
#include "../core/analysis/radiometric.h"

namespace SAR {
namespace UI {

AnalysisController::AnalysisController(
    const std::function<void(const QString&)>& logCallback,
    const std::function<void(int, const QString&)>& progressCallback,
    const std::function<void(const QString&, const QString&)>& resultsCallback)
    : logCallback(logCallback),
      progressCallback(progressCallback),
      resultsCallback(resultsCallback) {
}

bool AnalysisController::performAnalysis(const QStringList &selectedMethods, const cv::Mat &imageData) {
    // 清除之前的结果
    clearResults();
    
    // 执行选定的分析方法
    for (int i = 0; i < selectedMethods.size(); ++i) {
        const QString &method = selectedMethods[i];
        
        // 更新进度
        int progress = i * 100 / selectedMethods.size();
        progressCallback(progress, QObject::tr("正在分析：%1 (%2%)").arg(method).arg(progress));
        
        // 检查图像数据是否有效
        if (imageData.empty()) {
            logCallback(QObject::tr("错误：无法获取图像数据进行 %1 分析").arg(method));
            continue;
        }
        
        // 执行分析
        QString result;
        bool success = false;
        
        try {
            if (method == "SNR") {
                result = performSNRAnalysis(imageData);
                success = true;
            } else if (method == "ISLR") {
                result = performISLRAnalysis(imageData);
                success = true;
            } else if (method == "PSLR") {
                result = performPSLRAnalysis(imageData);
                success = true;
            } else if (method == "RASR") {
                result = performRASRAnalysis(imageData);
                success = true;
            } else if (method == "AASR") {
                result = performAASRAnalysis(imageData);
                success = true;
            } else if (method == "NESZ") {
                result = performNESZAnalysis(imageData);
                success = true;
            } else if (method == "RadiometricAccuracy") {
                result = performRadiometricAccuracyAnalysis(imageData);
                success = true;
            } else if (method == "RadiometricResolution") {
                result = performRadiometricResolutionAnalysis(imageData);
                success = true;
            } else if (method == "ENL") {
                result = performENLAnalysis(imageData);
                success = true;
            } else {
                result = QObject::tr("分析方法 %1 尚未实现").arg(method);
                logCallback(QObject::tr("警告：分析方法 %1 尚未实现").arg(method));
            }
        } catch (const std::exception &e) {
            logCallback(QObject::tr("错误：执行 %1 分析时发生异常：%2").arg(method).arg(e.what()));
            result = QObject::tr("分析过程中发生错误：%1").arg(e.what());
        } catch (...) {
            logCallback(QObject::tr("错误：执行 %1 分析时发生未知异常").arg(method));
            result = QObject::tr("分析过程中发生未知错误");
        }
        
        // 格式化结果
        QString resultStr = QObject::tr("方法：%1\n时间：%2\n")
                               .arg(method)
                               .arg(getCurrentDateTime());
        
        if (success) {
            resultStr += result;
            logCallback(QObject::tr("完成 %1 分析").arg(method));
        } else {
            resultStr += QObject::tr("分析未成功完成\n");
            resultStr += result;
            logCallback(QObject::tr("警告：%1 分析未成功完成").arg(method));
        }
        
        // 保存结果
        analysisResults[method] = resultStr;
        resultsCallback(method, resultStr);
    }
    
    // 完成分析
    progressCallback(100, QObject::tr("分析完成"));
    return !analysisResults.isEmpty();
}

const QMap<QString, QString>& AnalysisController::getResults() const {
    return analysisResults;
}

void AnalysisController::clearResults() {
    analysisResults.clear();
}

QString AnalysisController::getCurrentDateTime() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

QString AnalysisController::performSNRAnalysis(const cv::Mat &imageData) {
    // 使用SNR类的实例，调用analyze方法
    SAR::Analysis::SNR analyzer;
    auto result = analyzer.analyze(imageData);
    double snr = result.numericValue;
    
    QString resultStr = QObject::tr("方法：SNR (信噪比)\n");
    resultStr += QObject::tr("结果：%1 dB\n").arg(snr, 0, 'f', 2);
    resultStr += analyzer.getResultDescription();
    return resultStr;
}

QString AnalysisController::performISLRAnalysis(const cv::Mat &imageData) {
    SAR::Analysis::ISLR analyzer;
    double islr = analyzer.calculateISLR(imageData);
    QString result = QObject::tr("方法：积分旁瓣比分析\n");
    result += QObject::tr("结果：ISLR = %1 dB\n").arg(islr, 0, 'f', 2);
    return result;
}

QString AnalysisController::performPSLRAnalysis(const cv::Mat &imageData) {
    SAR::Analysis::PSLR analyzer;
    double pslr = analyzer.calculatePSLR(imageData);
    QString result = QObject::tr("方法：峰值旁瓣比分析\n");
    result += QObject::tr("结果：PSLR = %1 dB\n").arg(pslr, 0, 'f', 2);
    return result;
}

QString AnalysisController::performRASRAnalysis(const cv::Mat &imageData) {
    SAR::Analysis::RASR analyzer;
    // 设置默认参数值，实际应用中可能需要从配置或用户界面获取
    double PRF = 1500.0;         // 脉冲重复频率 (Hz)
    double R0 = 800000.0;        // 目标距离 (m)
    double incidenceAngle = 0.5; // 入射角 (rad)，约 28.6 度

    double rasr = analyzer.calculateRASR(imageData, PRF, R0, incidenceAngle);
    QString result = QObject::tr("方法：距离模糊度分析 (RASR)\n");
    result += QObject::tr("结果：RASR = %1\n").arg(rasr, 0, 'f', 4);
    result += QObject::tr("参数：\n");
    result += QObject::tr("  PRF = %1 Hz\n").arg(PRF);
    result += QObject::tr("  R0 = %1 km\n").arg(R0 / 1000.0);
    result += QObject::tr("  入射角 = %1 度\n").arg(incidenceAngle * 180.0 / M_PI);
    result += analyzer.getResultDescription();
    return result;
}

QString AnalysisController::performAASRAnalysis(const cv::Mat &imageData) {
    // 使用AASR类的实例，调用analyze方法
    SAR::Analysis::AASR analyzer;
    auto result = analyzer.analyze(imageData);
    double aasr = result.numericValue;
    
    QString resultStr = QObject::tr("方法：方位模糊度分析 (AASR)\n");
    resultStr += QObject::tr("结果：AASR = %1\n").arg(aasr, 0, 'f', 4);
    resultStr += QObject::tr("参数：\n");
    
    // 从结果中获取参数
    if (result.additionalValues.find("多普勒中心频率") != result.additionalValues.end()) {
        resultStr += QObject::tr("  多普勒中心频率 = %1 Hz\n")
                       .arg(result.additionalValues.at("多普勒中心频率"));
    }
    
    if (result.additionalValues.find("处理带宽") != result.additionalValues.end()) {
        resultStr += QObject::tr("  处理带宽 = %1 Hz\n")
                       .arg(result.additionalValues.at("处理带宽"));
    }
    
    resultStr += analyzer.getResultDescription();
    return resultStr;
}

QString AnalysisController::performNESZAnalysis(const cv::Mat &imageData) {
    SAR::Analysis::NESZ analyzer;
    double nesz = analyzer.calculateNESZ(imageData);
    QString result = QObject::tr("方法：噪声等效零散射截面分析\n");
    result += QObject::tr("结果：NESZ = %1 dB\n").arg(nesz, 0, 'f', 2);
    return result;
}

QString AnalysisController::performRadiometricAccuracyAnalysis(const cv::Mat &imageData) {
    SAR::Analysis::Radiometric analyzer;
    double accuracy = analyzer.calculateRadiometricAccuracy(imageData, imageData); // 理想情况下需要参考图像
    QString result = QObject::tr("方法：辐射精度分析\n");
    result += QObject::tr("结果：辐射精度 = %1 dB\n").arg(accuracy, 0, 'f', 2);
    result += QObject::tr("详细信息：\n");
    result += QObject::tr("  平均亮度 = %1\n")
                .arg(analyzer.calculateMeanIntensity(imageData), 0, 'f', 2);
    result += QObject::tr("  对比度 = %1\n")
                .arg(analyzer.calculateContrast(imageData), 0, 'f', 2);
    result += QObject::tr("  动态范围 = %1 dB\n")
                .arg(analyzer.calculateDynamicRange(imageData), 0, 'f', 2);
    return result;
}

QString AnalysisController::performRadiometricResolutionAnalysis(const cv::Mat &imageData) {
    SAR::Analysis::Radiometric analyzer;
    double resolution = analyzer.calculateRadiometricResolution(imageData);
    QString result = QObject::tr("方法：辐射分辨率分析\n");
    result += QObject::tr("结果：辐射分辨率 = %1 dB\n").arg(resolution, 0, 'f', 2);
    return result;
}

QString AnalysisController::performENLAnalysis(const cv::Mat &imageData) {
    SAR::Analysis::Radiometric analyzer;
    double enl = analyzer.calculateENL(imageData);
    QString result = QObject::tr("方法：等效视数分析\n");
    result += QObject::tr("结果：ENL = %1\n").arg(enl, 0, 'f', 2);
    return result;
}

} // namespace UI
} // namespace SAR 