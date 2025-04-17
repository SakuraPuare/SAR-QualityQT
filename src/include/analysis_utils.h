#ifndef ANALYSIS_UTILS_H
#define ANALYSIS_UTILS_H

#include <QString>
#include <opencv2/core/mat.hpp> // For cv::Mat

// 结果结构体
struct AnalysisResult {
    QString detailedLog;     // 详细结果和日志
    QString overviewSummary; // 用于概览选项卡的简短摘要
    bool success = false;    // 分析是否成功完成
    QString analysisName;    // 分析方法的名称 (可选，用于日志)
};

// 声明独立的分析函数
AnalysisResult performSNRAnalysis(const cv::Mat& imageData);
AnalysisResult performInfoContentAnalysis(const cv::Mat& imageData);
AnalysisResult performClarityAnalysis(const cv::Mat& imageData);
AnalysisResult performRadiometricAnalysis(const cv::Mat& imageData);
AnalysisResult performGLCMAnalysis(const cv::Mat& imageData);

// 声明独立的辅助函数 (特别是 GLCM 相关的)
// prepareImageForGLCM 返回准备好的图像，并通过引用传递日志
cv::Mat prepareImageForGLCM(const cv::Mat& inputImage, QString& log);
// computeGLCM 计算 GLCM 矩阵
void computeGLCM(const cv::Mat& img, cv::Mat& glcm, int dx, int dy, int levels, bool symmetric, bool normalize, QString& log); // 添加 log 参数
// calculateGLCMFeatures 计算特征，可能需要记录警告
void calculateGLCMFeatures(const cv::Mat& glcm, int levels,
                           double& contrast, double& energy, double& homogeneity, double& correlation, QString& log); // 添加 log 参数

#endif // ANALYSIS_UTILS_H 