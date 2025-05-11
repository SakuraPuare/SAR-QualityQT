#include "analysis_utils.h" // 包含新的头文件
#include <QCoreApplication> // For translate()
#include <QString>
#include <opencv2/core.hpp>    // minMaxLoc, meanStdDev
#include <opencv2/imgproc.hpp> // magnitude, split
#include <vector>
#include "radiometric.h"
#include <cmath>

// 独立的 Radiometric Analysis (Basic Statistics) 函数
SAR::Analysis::AnalysisResult performRadiometricAnalysis(const cv::Mat &inputImage) {
  SAR::Analysis::AnalysisResult result;
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
            .arg(maxVal - minVal, 0, 'g', 4)
            .arg(stddev, 0, 'g', 4);
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

namespace SAR {
namespace Analysis {

Radiometric::Radiometric() : lastResults() {
}

double Radiometric::calculateMeanIntensity(const cv::Mat& image) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算图像的平均强度
    cv::Scalar meanValue = cv::mean(floatImage);
    double meanIntensity = meanValue[0];
    
    // 保存结果
    lastResults["MeanIntensity"] = meanIntensity;
    
    return meanIntensity;
}

double Radiometric::calculateContrast(const cv::Mat& image) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算图像的标准差和均值
    cv::Scalar meanValue, stdDevValue;
    cv::meanStdDev(floatImage, meanValue, stdDevValue);
    
    // 计算对比度 (使用变异系数：标准差/均值)
    double contrast = 0.0;
    if (meanValue[0] > 1e-10) {
        contrast = stdDevValue[0] / meanValue[0];
    }
    
    // 保存结果
    lastResults["Contrast"] = contrast;
    
    return contrast;
}

double Radiometric::calculateDynamicRange(const cv::Mat& image) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 找到图像的最小值和最大值
    double minVal, maxVal;
    cv::minMaxLoc(floatImage, &minVal, &maxVal);
    
    // 计算动态范围 (dB)
    double dynamicRange = 0.0;
    if (minVal > 1e-10) {
        dynamicRange = 20.0 * std::log10(maxVal / minVal);
    } else {
        // 如果最小值接近 0，使用一个小的替代值
        dynamicRange = 20.0 * std::log10(maxVal / 1e-10);
    }
    
    // 保存结果
    lastResults["DynamicRange"] = dynamicRange;
    
    return dynamicRange;
}

double Radiometric::calculateRMS(const cv::Mat& image) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算平方
    cv::Mat squaredImage;
    cv::multiply(floatImage, floatImage, squaredImage);
    
    // 计算平均值
    cv::Scalar meanValue = cv::mean(squaredImage);
    
    // 计算 RMS (均方根)
    double rms = std::sqrt(meanValue[0]);
    
    // 保存结果
    lastResults["RMS"] = rms;
    
    return rms;
}

// 辐射精度计算
double Radiometric::calculateRadiometricAccuracy(const cv::Mat& image, const cv::Mat& reference) {
    if (image.empty() || reference.empty() || 
        image.size() != reference.size()) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage, floatReference;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }
    
    if (reference.type() != CV_32F) {
        reference.convertTo(floatReference, CV_32F);
    } else {
        floatReference = reference.clone();
    }

    // 计算两个图像的差异
    cv::Mat diffImage;
    cv::absdiff(floatImage, floatReference, diffImage);
    
    // 计算均方根误差
    cv::Scalar meanValue = cv::mean(diffImage.mul(diffImage));
    double rmse = std::sqrt(meanValue[0]);
    
    // 保存结果（单位为 dB）
    double radiometricAccuracy = -20.0 * std::log10(rmse);
    lastResults["RadiometricAccuracy"] = radiometricAccuracy;
    
    return radiometricAccuracy;
}

// 辐射分辨率计算
double Radiometric::calculateRadiometricResolution(const cv::Mat& image) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算图像的均值和标准差
    cv::Scalar meanValue, stdDevValue;
    cv::meanStdDev(floatImage, meanValue, stdDevValue);
    
    // 计算辐射分辨率（信噪比的倒数，单位为 dB）
    double radiometricResolution = 0.0;
    if (meanValue[0] > 1e-10) {
        double snr = meanValue[0] / stdDevValue[0];
        radiometricResolution = 20.0 * std::log10(1.0 / snr);
    }
    
    // 保存结果
    lastResults["RadiometricResolution"] = radiometricResolution;
    
    return radiometricResolution;
}

// 等效视数 (ENL) 计算
double Radiometric::calculateENL(const cv::Mat& image) {
    if (image.empty()) {
        return 0.0;
    }

    // 将图像转换为 32 位浮点类型进行处理
    cv::Mat floatImage;
    if (image.type() != CV_32F) {
        image.convertTo(floatImage, CV_32F);
    } else {
        floatImage = image.clone();
    }

    // 计算图像的均值和标准差
    cv::Scalar meanValue, stdDevValue;
    cv::meanStdDev(floatImage, meanValue, stdDevValue);
    
    // 计算 ENL (均值的平方 / 方差)
    double enl = 0.0;
    if (stdDevValue[0] > 1e-10) {
        enl = (meanValue[0] * meanValue[0]) / (stdDevValue[0] * stdDevValue[0]);
    }
    
    // 保存结果
    lastResults["ENL"] = enl;
    
    return enl;
}

std::map<QString, double> Radiometric::getAllFeatures(const cv::Mat& image) {
    // 清除之前的结果
    lastResults.clear();
    
    // 计算所有辐射特征
    calculateMeanIntensity(image);
    calculateContrast(image);
    calculateDynamicRange(image);
    calculateRMS(image);
    calculateRadiometricResolution(image);
    calculateENL(image);
    
    return lastResults;
}

QString Radiometric::getResultDescription() const {
    QString description = QString("辐射度分析结果：\n");
    
    // 添加各指标的结果描述
    if (lastResults.find("MeanIntensity") != lastResults.end()) {
        description += QString("平均强度：%1\n").arg(lastResults.at("MeanIntensity"), 0, 'f', 2);
    }
    
    if (lastResults.find("Contrast") != lastResults.end()) {
        description += QString("对比度：%1\n").arg(lastResults.at("Contrast"), 0, 'f', 2);
    }
    
    if (lastResults.find("DynamicRange") != lastResults.end()) {
        description += QString("动态范围：%1 dB\n").arg(lastResults.at("DynamicRange"), 0, 'f', 2);
    }
    
    if (lastResults.find("RMS") != lastResults.end()) {
        description += QString("均方根值：%1\n").arg(lastResults.at("RMS"), 0, 'f', 2);
    }
    
    if (lastResults.find("RadiometricAccuracy") != lastResults.end()) {
        description += QString("辐射精度：%1 dB\n").arg(lastResults.at("RadiometricAccuracy"), 0, 'f', 2);
    }
    
    if (lastResults.find("RadiometricResolution") != lastResults.end()) {
        description += QString("辐射分辨率：%1 dB\n").arg(lastResults.at("RadiometricResolution"), 0, 'f', 2);
    }
    
    if (lastResults.find("ENL") != lastResults.end()) {
        description += QString("等效视数 (ENL): %1\n").arg(lastResults.at("ENL"), 0, 'f', 2);
    }
    
    return description;
}

} // namespace Analysis
} // namespace SAR
