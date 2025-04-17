#include "analysis_utils.h" // 包含新的头文件
#include <QCoreApplication> // For translate()
#include <QString>
#include <opencv2/core.hpp>    // minMaxLoc, meanStdDev
#include <opencv2/imgproc.hpp> // magnitude, split
#include <vector>

// 独立的 Radiometric Analysis (Basic Statistics) 函数
AnalysisResult performRadiometricAnalysis(const cv::Mat &inputImage) {
  AnalysisResult result;
  result.analysisName =
      QCoreApplication::translate("Analysis", "Radiometric Statistics");
  result.detailedLog = QCoreApplication::translate(
      "Analysis", "Radiometric Analysis (Basic Statistics):\n");
  QString overviewPrefix =
      QCoreApplication::translate("Analysis", "Radiometric: ");

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
  QString prepLog;

  // 1. 准备单通道图像，保留原始类型或幅度
  prepLog += QCoreApplication::translate(
      "Analysis", "Preparing single-channel image for analysis...\n");
  if (inputImage.channels() == 2) {
    prepLog += QCoreApplication::translate(
        "Analysis", "Input is complex, calculating magnitude.\n");
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "Using magnitude image calculated from complex data.\n"); // Keep this
                                                                  // in main log
    std::vector<cv::Mat> channels;
    cv::split(inputImage, channels);
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
      prepLog += QCoreApplication::translate(
          "Analysis", "Converting complex channels to CV_32F for magnitude.\n");
      channels[0].convertTo(channels[0], CV_32F);
      channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat);
  } else if (inputImage.channels() == 1) {
    prepLog += QCoreApplication::translate(
        "Analysis", "Using single-channel input directly.\n");
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "Using single-channel input data.\n"); // Keep this in main log
    analysisMat = inputImage.clone();
  } else {
    prepLog += QCoreApplication::translate("Analysis",
                                           "Input is multi-channel (%1), using "
                                           "first channel for basic stats.\n")
                   .arg(inputImage.channels());
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "Using the first channel of the multi-channel input.\n"); // Keep this
                                                                  // in main log
    std::vector<cv::Mat> channels;
    cv::split(inputImage, channels);
    analysisMat = channels[0].clone();
  }
  result.detailedLog += prepLog; // Add prep log

  if (analysisMat.empty() || analysisMat.channels() != 1) {
    result.detailedLog += QCoreApplication::translate(
        "Analysis", "\nError: Failed to prepare single-channel data.");
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - Prep Failed");
    result.success = false;
    return result;
  }

  // 2. 转换为浮点数进行精确计算
  if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
    result.detailedLog += QCoreApplication::translate(
        "Analysis", "Converting image data to 32-bit float (CV_32F) for "
                    "precise statistics.\n");
    analysisMat.convertTo(analysisMat, CV_32F);
  }

  // 3. 计算基本统计量
  try {
    result.detailedLog += QCoreApplication::translate(
        "Analysis", "Calculating min, max, mean, and standard deviation...\n");
    double minVal = 0.0, maxVal = 0.0;
    cv::minMaxLoc(analysisMat, &minVal, &maxVal);

    cv::Scalar meanValue, stdDevValue;
    cv::meanStdDev(analysisMat, meanValue, stdDevValue);
    double mean = meanValue[0];
    double stddev = stdDevValue[0];

    result.detailedLog += QCoreApplication::translate(
        "Analysis", "\n--- Basic Radiometric Statistics ---\n");
    result.detailedLog +=
        QCoreApplication::translate("Analysis", "Minimum Value: %1\n")
            .arg(minVal);
    result.detailedLog +=
        QCoreApplication::translate("Analysis", "Maximum Value: %1\n")
            .arg(maxVal);
    result.detailedLog += QCoreApplication::translate(
                              "Analysis", "Dynamic Range (Max - Min): %1\n")
                              .arg(maxVal - minVal);
    result.detailedLog +=
        QCoreApplication::translate("Analysis", "Mean Value (μ): %1\n")
            .arg(mean);
    result.detailedLog +=
        QCoreApplication::translate("Analysis", "Standard Deviation (σ): %1\n")
            .arg(stddev);

    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\nInterpretation:\n"
        "- Dynamic Range indicates the spread of intensity values.\n"
        "- Mean represents the average brightness.\n"
        "- Standard Deviation estimates noise or texture variability.");

    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Range=%1, StdDev=%2")
            .arg(QString::number(maxVal - minVal, 'g', 4))
            .arg(QString::number(stddev, 'g', 4));
    result.detailedLog +=
        QCoreApplication::translate(
            "Analysis", "Internal Log: Radiometric stats calculated: Min=%1, "
                        "Max=%2, Mean=%3, StdDev=%4\n")
            .arg(minVal)
            .arg(maxVal)
            .arg(mean)
            .arg(stddev);
    result.success = true;

  } catch (const cv::Exception &e) {
    result.detailedLog +=
        QCoreApplication::translate(
            "Analysis", "\nError during radiometric statistics calculation: %1")
            .arg(QString::fromStdString(e.what()));
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - Calculation Failed");
    result.detailedLog +=
        QCoreApplication::translate(
            "Analysis",
            "Internal Log: OpenCV Error during Radiometric calculation: %1\n")
            .arg(QString::fromStdString(e.what()));
    result.success = false;
  }

  return result;
}
