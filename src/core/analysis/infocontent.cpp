#include "analysis_utils.h" // 包含新的头文件
#include <QCoreApplication> // For translate()
#include <QString>
#include <cmath>               // log2f
#include <limits>              // numeric_limits
#include <opencv2/core.hpp>    // minMaxLoc, Mat, Scalar
#include <opencv2/imgproc.hpp> // calcHist, cvtColor, magnitude, split
#include <vector>

// 独立的 Information Content (Entropy) 分析函数
SAR::Analysis::AnalysisResult performInfoContentAnalysis(const cv::Mat &inputImage) {
  SAR::Analysis::AnalysisResult result;
  result.analysisName =
      QCoreApplication::translate("Analysis", "Information Content (Entropy)");
  QString overviewPrefix =
      QCoreApplication::translate("Analysis", "Info Content (Entropy): ");

  if (inputImage.empty()) {
    result.detailedLog = QCoreApplication::translate(
        "Analysis",
        "Error: No valid image data provided for entropy analysis.");
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - No Data");
    result.success = false;
    return result;
  }

  cv::Mat analysisMat; // 单通道图像
  QString prepLog;

  // 1. 准备单通道图像 (与之前逻辑类似，记录到 prepLog)
  prepLog += QCoreApplication::translate(
      "Analysis", "Preparing single-channel image for entropy...\n");
  if (inputImage.channels() == 2) {
    prepLog += QCoreApplication::translate(
        "Analysis", "Input is complex (2-channel), calculating magnitude.\n");
    std::vector<cv::Mat> channels;
    cv::split(inputImage, channels);
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
      prepLog += QCoreApplication::translate(
          "Analysis", "Converting complex channels to CV_32F for magnitude.\n");
      channels[0].convertTo(channels[0], CV_32F);
      channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat);
    prepLog += QCoreApplication::translate(
        "Analysis", "Calculated magnitude from complex data.\n");
  } else if (inputImage.channels() > 1) {
    prepLog += QCoreApplication::translate(
                   "Analysis",
                   "Input is multi-channel (%1), converting to grayscale.\n")
                   .arg(inputImage.channels());
    cv::Mat sourceForCvt = inputImage;
    if (inputImage.depth() != CV_8U && inputImage.depth() != CV_16U &&
        inputImage.depth() != CV_32F) {
      prepLog += QCoreApplication::translate(
          "Analysis", "Input depth not directly supported by cvtColor, "
                      "converting to CV_8U first.\n");
      double minVal, maxVal;
      cv::minMaxLoc(inputImage, &minVal, &maxVal);
      int outputType = CV_MAKETYPE(CV_8U, inputImage.channels());
      if (maxVal > minVal)
        inputImage.convertTo(sourceForCvt, outputType,
                             255.0 / (maxVal - minVal),
                             -minVal * 255.0 / (maxVal - minVal));
      else
        inputImage.convertTo(sourceForCvt, outputType);
    }
    if (sourceForCvt.channels() >= 3) {
      try {
        int code = (sourceForCvt.channels() == 4) ? cv::COLOR_BGRA2GRAY
                                                  : cv::COLOR_BGR2GRAY;
        cv::cvtColor(sourceForCvt, analysisMat, code);
        prepLog += QCoreApplication::translate(
            "Analysis", "Converted multi-channel to grayscale using standard "
                        "conversion.\n");
      } catch (const cv::Exception &e) {
        prepLog += QCoreApplication::translate(
                       "Analysis", "Standard grayscale conversion failed (%1), "
                                   "falling back to first channel.\n")
                       .arg(QString::fromStdString(e.what()));
        std::vector<cv::Mat> channels;
        cv::split(sourceForCvt, channels);
        analysisMat = channels[0].clone();
        prepLog += QCoreApplication::translate("Analysis",
                                               "Used the first channel.\n");
      }
    } else {
      prepLog += QCoreApplication::translate(
                     "Analysis", "Input has %1 channels, not standard "
                                 "BGR/BGRA. Using first channel.\n")
                     .arg(sourceForCvt.channels());
      std::vector<cv::Mat> channels;
      cv::split(sourceForCvt, channels);
      analysisMat = channels[0].clone();
      prepLog +=
          QCoreApplication::translate("Analysis", "Used the first channel.\n");
    }
  } else if (inputImage.channels() == 1) {
    prepLog += QCoreApplication::translate(
        "Analysis", "Input is already single-channel.\n");
    analysisMat = inputImage.clone();
  } else {
    result.detailedLog =
        prepLog + QCoreApplication::translate(
                      "Analysis", "Error: Input image has 0 channels.");
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - Invalid Channels");
    result.success = false;
    return result;
  }
  result.detailedLog += prepLog; // 添加准备日志

  if (analysisMat.empty() || analysisMat.channels() != 1) {
    result.detailedLog += QCoreApplication::translate(
        "Analysis", "\nError: Could not obtain a valid single-channel image.");
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - Prep Failed");
    result.success = false;
    return result;
  }

  // 2. 转换为 CV_8U 用于直方图
  cv::Mat histMat;
  result.detailedLog += QCoreApplication::translate(
      "Analysis",
      "Converting single-channel image to 8-bit (CV_8U) for histogram...\n");
  if (analysisMat.depth() != CV_8U) {
    result.detailedLog += QCoreApplication::translate(
        "Analysis", "Normalizing data to 8-bit range (0-255).\n");
    double minVal, maxVal;
    cv::minMaxLoc(analysisMat, &minVal, &maxVal);
    if (maxVal > minVal) {
      analysisMat.convertTo(histMat, CV_8U, 255.0 / (maxVal - minVal),
                            -minVal * 255.0 / (maxVal - minVal));
    } else {
      analysisMat.convertTo(histMat, CV_8U);
      result.detailedLog += QCoreApplication::translate(
          "Analysis",
          "Image has constant value; entropy is expected to be 0.\n");
    }
  } else {
    histMat = analysisMat;
    result.detailedLog += QCoreApplication::translate(
        "Analysis", "Image is already 8-bit (CV_8U).\n");
  }

  if (histMat.empty() || histMat.type() != CV_8UC1) {
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\nError: Failed to produce a valid CV_8UC1 image for histogram.");
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - Conversion Failed");
    result.success = false;
    return result;
  }

  // 3. 计算直方图和熵
  cv::Mat hist;
  int histSize = 256;
  float range[] = {0, 256};
  const float *histRange[] = {range};
  bool uniform = true, accumulate = false;
  float entropy = 0.0f;

  try {
    result.detailedLog +=
        QCoreApplication::translate("Analysis", "Calculating histogram...\n");
    cv::calcHist(&histMat, 1, 0, cv::Mat(), hist, 1, &histSize, histRange,
                 uniform, accumulate);

    double totalPixels = histMat.total();
    if (totalPixels > 0) {
      hist /= totalPixels; // 归一化
    } else {
      result.detailedLog += QCoreApplication::translate(
          "Analysis",
          "\nWarning: Image contains no pixels. Cannot calculate entropy.");
      result.overviewSummary =
          overviewPrefix +
          QCoreApplication::translate("Analysis", "Warning - Empty Image");
      result.success = true; // Not an error, but entropy is undefined/0
      return result;
    }

    result.detailedLog += QCoreApplication::translate(
        "Analysis", "Calculating entropy from normalized histogram...\n");
    for (int i = 0; i < histSize; i++) {
      float p = hist.at<float>(i);
      if (p > std::numeric_limits<float>::epsilon()) {
        entropy -= p * log2f(p);
      }
    }

    result.detailedLog += QCoreApplication::translate(
        "Analysis", "\n--- Entropy Calculation Result ---\n");
    result.detailedLog += QCoreApplication::translate(
                              "Analysis", "Shannon Entropy: %1 bits/pixel\n")
                              .arg(entropy);
    result.detailedLog += QCoreApplication::translate(
        "Analysis", "\nInterpretation: Higher entropy generally indicates more "
                    "complexity.");

    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "%1 bits/pixel").arg(entropy, 0, 'f', 4);
    result.detailedLog +=
        QCoreApplication::translate(
            "Analysis",
            "Internal Log: Entropy calculation successful: %1 bits/pixel\n")
            .arg(entropy);
    result.success = true;

  } catch (const cv::Exception &e) {
    result.detailedLog +=
        QCoreApplication::translate(
            "Analysis", "\nError during histogram or entropy calculation: %1")
            .arg(QString::fromStdString(e.what()));
    result.overviewSummary =
        overviewPrefix +
        QCoreApplication::translate("Analysis", "Error - Calculation Failed");
    result.detailedLog +=
        QCoreApplication::translate(
            "Analysis",
            "Internal Log: OpenCV Error during Entropy calculation: %1\n")
            .arg(QString::fromStdString(e.what()));
    result.success = false;
  }

  return result;
}
