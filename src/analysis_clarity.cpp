#include "analysis_utils.h" // 包含新的头文件
#include <opencv2/imgproc.hpp> // Sobel, addWeighted, mean, convertScaleAbs
#include <opencv2/core.hpp>    // Mat, Scalar
#include <vector>
#include <QString>
#include <QCoreApplication> // For translate()

// 独立的 Clarity Analysis (Average Gradient Magnitude) 函数
AnalysisResult performClarityAnalysis(const cv::Mat& inputImage) {
    AnalysisResult result;
    result.analysisName = QCoreApplication::translate("Analysis", "Clarity (Gradient Magnitude)");
    result.detailedLog = QCoreApplication::translate("Analysis", "Clarity Analysis (Average Gradient Magnitude):\n");
    QString overviewPrefix = QCoreApplication::translate("Analysis", "Clarity (GradMag): ");

    if (inputImage.empty()) {
        result.detailedLog += QCoreApplication::translate("Analysis", "\nError: No valid image data provided.");
        result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "Error - No Data");
        result.success = false;
        return result;
    }

    cv::Mat analysisMat;
    QString prepareLog;
    // 使用独立的 prepareImageForGLCM 函数准备 8 位灰度图
    result.detailedLog += QCoreApplication::translate("Analysis", "Preparing 8-bit grayscale image...\n");
    analysisMat = prepareImageForGLCM(inputImage, prepareLog); // 调用独立函数
    result.detailedLog += prepareLog;

    if (analysisMat.empty() || analysisMat.type() != CV_8UC1) {
        result.detailedLog += QCoreApplication::translate("Analysis", "\nError: Failed to prepare 8-bit single channel image for gradient calculation.");
        result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "Error - Prep Failed");
        result.success = false;
        return result;
    }

    try {
        result.detailedLog += QCoreApplication::translate("Analysis", "Calculating gradients using Sobel operator...\n");
        cv::Mat grad_x, grad_y;
        cv::Mat abs_grad_x, abs_grad_y;
        cv::Mat grad;

        cv::Sobel(analysisMat, grad_x, CV_16S, 1, 0, 3);
        cv::Sobel(analysisMat, grad_y, CV_16S, 0, 1, 3);

        cv::convertScaleAbs(grad_x, abs_grad_x);
        cv::convertScaleAbs(grad_y, abs_grad_y);

        cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

        result.detailedLog += QCoreApplication::translate("Analysis", "Calculating average gradient magnitude...\n");
        cv::Scalar avgGradScalar = cv::mean(grad);
        double averageGradient = avgGradScalar[0];

        result.detailedLog += QCoreApplication::translate("Analysis", "\n--- Gradient Analysis Results ---\n");
        result.detailedLog += QCoreApplication::translate("Analysis", "Average Gradient Magnitude: %1\n").arg(averageGradient);
        result.detailedLog += QCoreApplication::translate("Analysis", "\nInterpretation: Higher values generally indicate sharper details or more texture/noise.");

        result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "%1").arg(QString::number(averageGradient, 'f', 2));
        result.detailedLog += QCoreApplication::translate("Analysis", "Internal Log: Clarity analysis completed: %1\n").arg(averageGradient);
        result.success = true;

    } catch (const cv::Exception& e) {
        result.detailedLog += QCoreApplication::translate("Analysis", "\nError during gradient calculation: %1").arg(QString::fromStdString(e.what()));
        result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "Error - Calculation Failed");
        result.detailedLog += QCoreApplication::translate("Analysis", "Internal Log: OpenCV Error during Clarity (Gradient) calculation: %1\n").arg(QString::fromStdString(e.what()));
        result.success = false;
    }

    return result;
}
