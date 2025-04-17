#include "./ui_mainwindow.h"   // 访问 UI 元素
#include "analysis_utils.h"    // 包含新的头文件
#include "mainwindow.h"        // 需要包含 MainWindow 头文件以访问成员和 UI
#include <QCoreApplication>    // For translate()
#include <QString>             // 需要 QString
#include <cmath>               // std::abs, std::sqrt
#include <opencv2/core.hpp>    // meanStdDev, split, magnitude, convertTo
#include <opencv2/imgproc.hpp> // (可能需要，如果用到其他处理)
#include <vector>              // std::vector

// 独立的 SNR/ENL 分析函数
AnalysisResult performSNRAnalysis(const cv::Mat &inputImage) {
  AnalysisResult result;
  result.analysisName =
      QCoreApplication::translate("Analysis", "SNR/ENL"); // 设置分析名称
  result.detailedLog = QCoreApplication::translate(
      "Analysis", "SNR/ENL Analysis Results (Global):\n");
  QString overviewPrefix =
      QCoreApplication::translate("Analysis", "SNR/ENL (Global): ");

  if (inputImage.empty()) {
    result.detailedLog += QCoreApplication::translate(
        "Analysis", "\nError: No valid image data provided.");
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - No Data");
    result.success = false;
    return result;
  }

  cv::Mat analysisMat;
  QString prepLog; // 用于准备步骤的日志

  // 1. 准备单通道浮点图像
  prepLog += QCoreApplication::translate(
      "Analysis", "Preparing single-channel float image...\n");
  if (inputImage.channels() == 2) {
    prepLog += QCoreApplication::translate(
        "Analysis", "Input is complex (2-channel), calculating magnitude.\n");
    std::vector<cv::Mat> channels;
    cv::split(inputImage, channels);
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
      prepLog += QCoreApplication::translate(
          "Analysis",
          "Converting complex channels to CV_32F for magnitude calculation.\n");
      channels[0].convertTo(channels[0], CV_32F);
      channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat);
    prepLog +=
        QCoreApplication::translate("Analysis", "Using magnitude image.\n");
  } else if (inputImage.channels() == 1) {
    prepLog +=
        QCoreApplication::translate("Analysis", "Input is single-channel.\n");
    analysisMat = inputImage.clone();
    if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
      prepLog += QCoreApplication::translate(
                     "Analysis",
                     "Converting single-channel image (type: %1) to CV_32F.\n")
                     .arg(cv::typeToString(analysisMat.type()));
      analysisMat.convertTo(analysisMat, CV_32F);
      prepLog += QCoreApplication::translate(
          "Analysis", "Converted input to floating-point type (CV_32F).\n");
    } else {
      prepLog += QCoreApplication::translate(
          "Analysis", "Using existing single-channel floating-point data.\n");
    }
  } else {
    result.detailedLog +=
        QCoreApplication::translate(
            "Analysis",
            "\nError: Unsupported channel count (%1). Expected 1 or 2.")
            .arg(inputImage.channels());
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - Unsupported Channels");
    result.success = false;
    return result;
  }

  result.detailedLog += prepLog; // 将准备日志添加到详细日志

  if (analysisMat.empty() || analysisMat.channels() != 1 ||
      (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F)) {
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\nError: Failed to prepare a valid single-channel float image.");
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - Prep Failed");
    result.success = false;
    return result;
  }

  // 2. 计算全局均值和标准差
  cv::Scalar meanValue, stdDevValue;
  try {
    result.detailedLog += QCoreApplication::translate(
        "Analysis", "Calculating global mean and standard deviation...\n");
    cv::meanStdDev(analysisMat, meanValue, stdDevValue);

    double mean = meanValue[0];
    double stddev = stdDevValue[0];

    result.detailedLog += QCoreApplication::translate(
        "Analysis", "\n--- Global Statistics ---\n");
    result.detailedLog +=
        QCoreApplication::translate("Analysis", "Mean (μ): %1\n").arg(mean);
    result.detailedLog +=
        QCoreApplication::translate("Analysis", "Standard Deviation (σ): %1\n")
            .arg(stddev);

    // 3. 计算 SNR 和 ENL
    if (stddev > 1e-9) {
      double snr = mean / stddev;
      double enl = snr * snr;

      result.detailedLog += QCoreApplication::translate(
          "Analysis", "\n--- Quality Metrics (Global) ---\n");
      result.detailedLog +=
          QCoreApplication::translate("Analysis",
                                      "Signal-to-Noise Ratio (SNR = μ/σ): %1\n")
              .arg(snr);
      result.detailedLog +=
          QCoreApplication::translate(
              "Analysis", "Equivalent Number of Looks (ENL = (μ/σ)²): %1\n")
              .arg(enl);

      result.overviewSummary =
          overviewPrefix +
          QCoreApplication::translate("Analysis", "SNR=%1, ENL=%2")
              .arg(snr, 0, 'f', 2)
              .arg(enl, 0, 'f', 2);
      result.detailedLog +=
          QCoreApplication::translate("Analysis",
                                      "Internal Log: SNR/ENL calculated: "
                                      "Mean=%1, StdDev=%2, SNR=%3, ENL=%4\n")
              .arg(mean)
              .arg(stddev)
              .arg(snr)
              .arg(enl);
    } else {
      result.detailedLog +=
          QCoreApplication::translate(
              "Analysis", "\nWarning: Standard deviation is close to zero (σ = "
                          "%1). Cannot calculate SNR/ENL reliably.\n")
              .arg(stddev);
      result.overviewSummary = overviewPrefix + QCoreApplication::translate(
                                                    "Analysis", "N/A (σ ≈ 0)");
      result.detailedLog += QCoreApplication::translate(
          "Analysis", "Internal Log: SNR/ENL calculation skipped: Standard "
                      "deviation is near zero.\n");
    }

    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\n\nNote: These metrics are calculated globally. For more meaningful "
        "assessment, calculate over a statistically homogeneous region.");
    result.success = true; // 标记成功

  } catch (const cv::Exception &e) {
    result.detailedLog +=
        QCoreApplication::translate(
            "Analysis",
            "\nError during mean/standard deviation calculation: %1")
            .arg(QString::fromStdString(e.what()));
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - Calculation Failed");
    result.detailedLog +=
        QCoreApplication::translate(
            "Analysis", "Internal Log: OpenCV Error during meanStdDev: %1\n")
            .arg(QString::fromStdString(e.what()));
    result.success = false;
  }

  return result;
}
