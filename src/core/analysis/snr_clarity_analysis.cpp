#include "analysis_utils.h"
#include "snr.h"
#include "clarity.h"
#include <QString>
#include <QTextStream>
#include <opencv2/imgproc.hpp>

// SNR分析函数实现
AnalysisResult performSNRAnalysis(const cv::Mat &imageData) {
    AnalysisResult result;
    result.analysisName = "SNR分析";
    
    try {
        // 使用SNR类进行分析
        SAR::Analysis::SNR snrAnalyzer;
        double snrValue = snrAnalyzer.calculateSNR(imageData);
        double noiseLevel = snrAnalyzer.estimateNoiseLevel(imageData);
        
        // 格式化详细结果
        QString detailedLog;
        QTextStream stream(&detailedLog);
        
        stream << "# 信噪比 (SNR) 分析结果\n\n";
        stream << "## 综合指标\n";
        stream << "- 信噪比 (dB): " << QString::number(snrValue, 'f', 2) << "\n";
        stream << "- 估计噪声水平: " << QString::number(noiseLevel, 'f', 3) << "\n\n";
        
        stream << "## 分析说明\n";
        stream << "信噪比(SNR)是衡量SAR图像质量的基本指标，表示信号强度与噪声强度之比。\n";
        stream << "- SNR > 15dB: 图像质量优秀\n";
        stream << "- 10dB ≤ SNR ≤ 15dB: 图像质量良好\n";
        stream << "- 5dB ≤ SNR < 10dB: 图像质量一般\n";
        stream << "- SNR < 5dB: 图像质量较差\n\n";
        
        // 添加结果评估
        stream << "## 评估结果\n";
        if (snrValue > 15.0) {
            stream << "根据信噪比分析，当前图像质量 **优秀**。\n";
        } else if (snrValue >= 10.0) {
            stream << "根据信噪比分析，当前图像质量 **良好**。\n";
        } else if (snrValue >= 5.0) {
            stream << "根据信噪比分析，当前图像质量 **一般**。\n";
        } else {
            stream << "根据信噪比分析，当前图像质量 **较差**，建议检查图像获取设置。\n";
        }
        
        // 设置结果
        result.detailedLog = detailedLog;
        result.overviewSummary = QString("SNR分析: %1 dB (%2)")
                                .arg(snrValue, 0, 'f', 2)
                                .arg(snrValue > 10.0 ? "良好" : "需改进");
        result.success = true;
    }
    catch (const std::exception &e) {
        result.detailedLog = QString("SNR分析失败: %1").arg(e.what());
        result.overviewSummary = "SNR分析: 失败";
        result.success = false;
    }
    
    return result;
}

// 清晰度分析函数实现
AnalysisResult performClarityAnalysis(const cv::Mat &imageData) {
    AnalysisResult result;
    result.analysisName = "清晰度分析";
    
    try {
        // 使用Clarity类进行分析
        SAR::Analysis::Clarity clarityAnalyzer;
        double clarityScore = clarityAnalyzer.calculateClarityScore(imageData);
        double edgeStrength = clarityAnalyzer.calculateEdgeStrength(imageData);
        
        // 格式化详细结果
        QString detailedLog;
        QTextStream stream(&detailedLog);
        
        stream << "# 清晰度分析结果\n\n";
        stream << "## 综合指标\n";
        stream << "- 清晰度得分: " << QString::number(clarityScore, 'f', 2) << "\n";
        stream << "- 边缘强度: " << QString::number(edgeStrength, 'f', 3) << "\n\n";
        
        stream << "## 分析说明\n";
        stream << "清晰度是衡量SAR图像细节保留程度的重要指标。\n";
        stream << "- 清晰度得分反映图像的锐度和细节水平\n";
        stream << "- 边缘强度反映图像中边缘特征的明显程度\n\n";
        
        // 基于图像大小和类型设置阈值
        double clarityThresholdHigh = 2000.0; // 这些阈值需根据具体图像类型调整
        double clarityThresholdMedium = 500.0;
        
        // 添加结果评估
        stream << "## 评估结果\n";
        if (clarityScore > clarityThresholdHigh) {
            stream << "根据清晰度分析，当前图像清晰度 **优秀**。\n";
        } else if (clarityScore > clarityThresholdMedium) {
            stream << "根据清晰度分析，当前图像清晰度 **良好**。\n";
        } else {
            stream << "根据清晰度分析，当前图像清晰度 **一般**，可能存在模糊或细节丢失。\n";
        }
        
        if (edgeStrength > 30.0) {
            stream << "图像边缘特征 **明显**，目标轮廓清晰。\n";
        } else if (edgeStrength > 15.0) {
            stream << "图像边缘特征 **较明显**，目标轮廓可辨识。\n";
        } else {
            stream << "图像边缘特征 **不明显**，目标轮廓模糊。\n";
        }
        
        // 设置结果
        result.detailedLog = detailedLog;
        result.overviewSummary = QString("清晰度分析: 得分 %1 (%2)")
                                .arg(clarityScore, 0, 'f', 2)
                                .arg(clarityScore > clarityThresholdMedium ? "清晰" : "模糊");
        result.success = true;
    }
    catch (const std::exception &e) {
        result.detailedLog = QString("清晰度分析失败: %1").arg(e.what());
        result.overviewSummary = "清晰度分析: 失败";
        result.success = false;
    }
    
    return result;
}
