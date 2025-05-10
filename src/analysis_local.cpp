#include "analysis_utils.h"
#include <QCoreApplication> // For translate()
#include <QString>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <complex>
#include <cmath>

// 积分旁瓣比（Integrated Sidelobe Ratio, ISLR）分析
AnalysisResult performISLRAnalysis(const cv::Mat &inputImage) {
  AnalysisResult result;
  result.analysisName = QCoreApplication::translate("Analysis", "积分旁瓣比 (ISLR)");
  result.detailedLog = QCoreApplication::translate("Analysis", "积分旁瓣比 (ISLR) 分析结果:\n");
  QString overviewPrefix = QCoreApplication::translate("Analysis", "ISLR: ");

  if (inputImage.empty()) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：未提供有效的图像数据。");
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 无数据");
    result.success = false;
    return result;
  }

  try {
    // 准备单通道图像
    cv::Mat analysisMat;
    if (inputImage.channels() == 2) {
      // 复数图像转为幅度图
      std::vector<cv::Mat> channels;
      cv::split(inputImage, channels);
      if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
      }
      cv::magnitude(channels[0], channels[1], analysisMat);
    } else if (inputImage.channels() == 1) {
      analysisMat = inputImage.clone();
      if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
        analysisMat.convertTo(analysisMat, CV_32F);
      }
    } else {
      result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：不支持的通道数量 (%1)。需要单通道或双通道图像。").arg(inputImage.channels());
      result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 不支持的格式");
      result.success = false;
      return result;
    }

    // 执行 FFT 分析
    cv::Mat padded;
    int m = cv::getOptimalDFTSize(analysisMat.rows);
    int n = cv::getOptimalDFTSize(analysisMat.cols);
    cv::copyMakeBorder(analysisMat, padded, 0, m - analysisMat.rows, 0, n - analysisMat.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    // 优化的 FFT 实现
    cv::Mat planes[] = {padded, cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);
    cv::dft(complexI, complexI);
    
    // 分离实部和虚部
    cv::split(complexI, planes);
    cv::Mat magnitudeI;
    cv::magnitude(planes[0], planes[1], magnitudeI);
    
    // 主瓣和旁瓣区域定义
    int centerRow = magnitudeI.rows / 2;
    int centerCol = magnitudeI.cols / 2;
    int mainLobeWidth = std::min(magnitudeI.rows, magnitudeI.cols) / 10; // 假设主瓣宽度为总尺寸的 1/10
    
    // 查找主瓣峰值
    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(magnitudeI, nullptr, &maxVal, nullptr, &maxLoc);
    
    // 计算主瓣和旁瓣能量
    double mainLobeEnergy = 0.0;
    double sideLobeEnergy = 0.0;
    
    for (int i = 0; i < magnitudeI.rows; i++) {
      for (int j = 0; j < magnitudeI.cols; j++) {
        double distance = std::sqrt(std::pow(i - centerRow, 2) + std::pow(j - centerCol, 2));
        float pixelValue = magnitudeI.at<float>(i, j);
        double energy = pixelValue * pixelValue; // 能量与幅度平方成正比
        
        if (distance <= mainLobeWidth) {
          mainLobeEnergy += energy;
        } else {
          sideLobeEnergy += energy;
        }
      }
    }
    
    // 计算积分旁瓣比
    double islr = sideLobeEnergy / mainLobeEnergy;
    double islrDB = 10 * std::log10(islr); // 转换为分贝单位
    
    result.detailedLog += QCoreApplication::translate("Analysis", "\n--- 积分旁瓣比分析 ---\n");
    result.detailedLog += QCoreApplication::translate("Analysis", "主瓣能量：%1\n").arg(mainLobeEnergy);
    result.detailedLog += QCoreApplication::translate("Analysis", "旁瓣能量：%1\n").arg(sideLobeEnergy);
    result.detailedLog += QCoreApplication::translate("Analysis", "积分旁瓣比 (ISLR): %1\n").arg(islr);
    result.detailedLog += QCoreApplication::translate("Analysis", "积分旁瓣比 (ISLR, dB): %1 dB\n").arg(islrDB);
    
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\n解释:\n"
        "- ISLR 是旁瓣能量与主瓣能量之比\n"
        "- 较低的 ISLR 值表示主瓣包含了更多的能量，成像质量更好\n"
        "- 典型的良好 SAR 图像 ISLR 在 -20dB 左右");
    
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "%1 dB").arg(islrDB, 0, 'f', 2);
    result.success = true;
  } catch (const cv::Exception &e) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：ISLR 分析计算过程中出错：%1").arg(QString::fromStdString(e.what()));
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 计算失败");
    result.success = false;
  }
  
  return result;
}

// 峰值旁瓣比（Peak Sidelobe Ratio, PSLR）分析
AnalysisResult performPSLRAnalysis(const cv::Mat &inputImage) {
  AnalysisResult result;
  result.analysisName = QCoreApplication::translate("Analysis", "峰值旁瓣比 (PSLR)");
  result.detailedLog = QCoreApplication::translate("Analysis", "峰值旁瓣比 (PSLR) 分析结果:\n");
  QString overviewPrefix = QCoreApplication::translate("Analysis", "PSLR: ");

  if (inputImage.empty()) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：未提供有效的图像数据。");
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 无数据");
    result.success = false;
    return result;
  }

  try {
    // 准备单通道图像
    cv::Mat analysisMat;
    if (inputImage.channels() == 2) {
      // 复数图像转为幅度图
      std::vector<cv::Mat> channels;
      cv::split(inputImage, channels);
      if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
      }
      cv::magnitude(channels[0], channels[1], analysisMat);
    } else if (inputImage.channels() == 1) {
      analysisMat = inputImage.clone();
      if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
        analysisMat.convertTo(analysisMat, CV_32F);
      }
    } else {
      result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：不支持的通道数量 (%1)。需要单通道或双通道图像。").arg(inputImage.channels());
      result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 不支持的格式");
      result.success = false;
      return result;
    }

    // 执行 FFT 分析
    cv::Mat padded;
    int m = cv::getOptimalDFTSize(analysisMat.rows);
    int n = cv::getOptimalDFTSize(analysisMat.cols);
    cv::copyMakeBorder(analysisMat, padded, 0, m - analysisMat.rows, 0, n - analysisMat.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    cv::Mat planes[] = {padded, cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);
    cv::dft(complexI, complexI);
    
    // 分离实部和虚部并计算幅度
    cv::split(complexI, planes);
    cv::Mat magnitudeI;
    cv::magnitude(planes[0], planes[1], magnitudeI);
    
    // 寻找主瓣峰值
    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(magnitudeI, nullptr, &maxVal, nullptr, &maxLoc);
    
    // 主瓣区域定义
    int centerRow = magnitudeI.rows / 2;
    int centerCol = magnitudeI.cols / 2;
    int mainLobeWidth = std::min(magnitudeI.rows, magnitudeI.cols) / 10; // 假设主瓣宽度为总尺寸的 1/10
    
    // 屏蔽主瓣区域以寻找最大旁瓣
    cv::Mat maskedMagnitude = magnitudeI.clone();
    for (int i = 0; i < magnitudeI.rows; i++) {
      for (int j = 0; j < magnitudeI.cols; j++) {
        double distance = std::sqrt(std::pow(i - centerRow, 2) + std::pow(j - centerCol, 2));
        if (distance <= mainLobeWidth) {
          maskedMagnitude.at<float>(i, j) = 0;
        }
      }
    }
    
    // 查找最大旁瓣
    double maxSidelobeVal;
    cv::minMaxLoc(maskedMagnitude, nullptr, &maxSidelobeVal, nullptr, nullptr);
    
    // 计算峰值旁瓣比
    double pslr = maxSidelobeVal / maxVal;
    double pslrDB = 20 * std::log10(pslr); // 转换为分贝单位
    
    result.detailedLog += QCoreApplication::translate("Analysis", "\n--- 峰值旁瓣比分析 ---\n");
    result.detailedLog += QCoreApplication::translate("Analysis", "主瓣峰值：%1\n").arg(maxVal);
    result.detailedLog += QCoreApplication::translate("Analysis", "最大旁瓣峰值：%1\n").arg(maxSidelobeVal);
    result.detailedLog += QCoreApplication::translate("Analysis", "峰值旁瓣比 (PSLR): %1\n").arg(pslr);
    result.detailedLog += QCoreApplication::translate("Analysis", "峰值旁瓣比 (PSLR, dB): %1 dB\n").arg(pslrDB);
    
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\n解释:\n"
        "- PSLR 是最大旁瓣峰值与主瓣峰值之比\n"
        "- 典型的良好 SAR 图像 PSLR 在 -25dB 到 -30dB 之间\n"
        "- 较低的 PSLR 值表示目标识别能力更好");
    
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "%1 dB").arg(pslrDB, 0, 'f', 2);
    result.success = true;
  } catch (const cv::Exception &e) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：PSLR 分析计算过程中出错：%1").arg(QString::fromStdString(e.what()));
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 计算失败");
    result.success = false;
  }
  
  return result;
}

// 距离模糊度（Range Resolution）分析
AnalysisResult performRangeResolutionAnalysis(const cv::Mat &inputImage) {
  AnalysisResult result;
  result.analysisName = QCoreApplication::translate("Analysis", "距离模糊度");
  result.detailedLog = QCoreApplication::translate("Analysis", "距离模糊度分析结果:\n");
  QString overviewPrefix = QCoreApplication::translate("Analysis", "距离模糊度：");

  if (inputImage.empty()) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：未提供有效的图像数据。");
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 无数据");
    result.success = false;
    return result;
  }

  try {
    // 准备单通道图像
    cv::Mat analysisMat;
    if (inputImage.channels() == 2) {
      // 复数图像转为幅度图
      std::vector<cv::Mat> channels;
      cv::split(inputImage, channels);
      if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
      }
      cv::magnitude(channels[0], channels[1], analysisMat);
    } else if (inputImage.channels() == 1) {
      analysisMat = inputImage.clone();
      if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
        analysisMat.convertTo(analysisMat, CV_32F);
      }
    } else {
      result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：不支持的通道数量 (%1)。需要单通道或双通道图像。").arg(inputImage.channels());
      result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 不支持的格式");
      result.success = false;
      return result;
    }

    // 执行边缘检测以识别目标边缘
    cv::Mat edgeImage;
    cv::Sobel(analysisMat, edgeImage, CV_32F, 1, 0); // x 方向的 Sobel 算子

    // 计算距离方向的脉冲响应函数宽度
    double totalEdgeWidth = 0;
    int edgeCount = 0;
    int threshold = 100; // 阈值可根据实际图像调整

    // 逐行分析边缘宽度
    for (int i = 0; i < edgeImage.rows; i++) {
      std::vector<int> edgePositions;
      for (int j = 1; j < edgeImage.cols; j++) {
        float curr = edgeImage.at<float>(i, j);
        float prev = edgeImage.at<float>(i, j-1);
        
        // 检测边缘位置（值变化显著的点）
        if (std::abs(curr) > threshold && std::signbit(curr) != std::signbit(prev)) {
          edgePositions.push_back(j);
        }
      }
      
      // 如果找到足够的边缘点，计算平均宽度
      if (edgePositions.size() >= 2) {
        for (size_t j = 1; j < edgePositions.size(); j++) {
          int width = edgePositions[j] - edgePositions[j-1];
          if (width > 0 && width < edgeImage.cols / 4) { // 忽略异常宽边缘
            totalEdgeWidth += width;
            edgeCount++;
          }
        }
      }
    }

    double avgResolution = 0;
    if (edgeCount > 0) {
      avgResolution = totalEdgeWidth / edgeCount;
    } else {
      // 如果无法通过边缘检测计算，采用理论估计
      // 假设为实际分辨率的 1/2 到 1/3（因为理论分辨率通常优于实际分辨率）
      avgResolution = std::max(analysisMat.cols, analysisMat.rows) / 100.0;
    }
    
    // 假设图像是地面校正的，单位为米
    // 分辨率单位转换为米（实际需根据图像元数据提供的地面采样距离计算）
    double pixelSize = 1.0; // 假设 1 像素 = 1 米（实际应从元数据获取）
    double rangeResolution = avgResolution * pixelSize;
    
    result.detailedLog += QCoreApplication::translate("Analysis", "\n--- 距离模糊度分析 ---\n");
    if (edgeCount > 0) {
      result.detailedLog += QCoreApplication::translate("Analysis", "检测到的边缘数量：%1\n").arg(edgeCount);
      result.detailedLog += QCoreApplication::translate("Analysis", "平均边缘宽度 (像素): %1\n").arg(avgResolution);
    } else {
      result.detailedLog += QCoreApplication::translate("Analysis", "未检测到足够的边缘，使用理论估计值\n");
    }
    result.detailedLog += QCoreApplication::translate("Analysis", "估计的距离分辨率：%1 m\n").arg(rangeResolution);
    
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\n解释:\n"
        "- 距离分辨率表示在距离方向上能够分辨的最小距离\n"
        "- 较小的值表示更好的分辨能力\n"
        "- 典型的高分辨率 SAR 系统距离分辨率在 1-5 米范围");
    
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "%1 m").arg(rangeResolution, 0, 'f', 2);
    result.success = true;
  } catch (const cv::Exception &e) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：距离模糊度分析计算过程中出错：%1").arg(QString::fromStdString(e.what()));
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 计算失败");
    result.success = false;
  }
  
  return result;
}

// 方位模糊度（Azimuth Resolution）分析
AnalysisResult performAzimuthResolutionAnalysis(const cv::Mat &inputImage) {
  AnalysisResult result;
  result.analysisName = QCoreApplication::translate("Analysis", "方位模糊度");
  result.detailedLog = QCoreApplication::translate("Analysis", "方位模糊度分析结果:\n");
  QString overviewPrefix = QCoreApplication::translate("Analysis", "方位模糊度：");

  if (inputImage.empty()) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：未提供有效的图像数据。");
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 无数据");
    result.success = false;
    return result;
  }

  try {
    // 准备单通道图像
    cv::Mat analysisMat;
    if (inputImage.channels() == 2) {
      // 复数图像转为幅度图
      std::vector<cv::Mat> channels;
      cv::split(inputImage, channels);
      if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
      }
      cv::magnitude(channels[0], channels[1], analysisMat);
    } else if (inputImage.channels() == 1) {
      analysisMat = inputImage.clone();
      if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
        analysisMat.convertTo(analysisMat, CV_32F);
      }
    } else {
      result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：不支持的通道数量 (%1)。需要单通道或双通道图像。").arg(inputImage.channels());
      result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 不支持的格式");
      result.success = false;
      return result;
    }

    // 执行边缘检测以识别目标边缘
    cv::Mat edgeImage;
    cv::Sobel(analysisMat, edgeImage, CV_32F, 0, 1); // y 方向的 Sobel 算子

    // 计算方位方向的脉冲响应函数宽度
    double totalEdgeWidth = 0;
    int edgeCount = 0;
    int threshold = 100; // 阈值可根据实际图像调整

    // 逐列分析边缘宽度
    for (int j = 0; j < edgeImage.cols; j++) {
      std::vector<int> edgePositions;
      for (int i = 1; i < edgeImage.rows; i++) {
        float curr = edgeImage.at<float>(i, j);
        float prev = edgeImage.at<float>(i-1, j);
        
        // 检测边缘位置（值变化显著的点）
        if (std::abs(curr) > threshold && std::signbit(curr) != std::signbit(prev)) {
          edgePositions.push_back(i);
        }
      }
      
      // 如果找到足够的边缘点，计算平均宽度
      if (edgePositions.size() >= 2) {
        for (size_t i = 1; i < edgePositions.size(); i++) {
          int width = edgePositions[i] - edgePositions[i-1];
          if (width > 0 && width < edgeImage.rows / 4) { // 忽略异常宽边缘
            totalEdgeWidth += width;
            edgeCount++;
          }
        }
      }
    }

    double avgResolution = 0;
    if (edgeCount > 0) {
      avgResolution = totalEdgeWidth / edgeCount;
    } else {
      // 如果无法通过边缘检测计算，采用理论估计
      avgResolution = std::max(analysisMat.cols, analysisMat.rows) / 100.0;
    }
    
    // 假设图像是地面校正的，单位为米
    double pixelSize = 1.0; // 假设 1 像素 = 1 米（实际应从元数据获取）
    double azimuthResolution = avgResolution * pixelSize;
    
    result.detailedLog += QCoreApplication::translate("Analysis", "\n--- 方位模糊度分析 ---\n");
    if (edgeCount > 0) {
      result.detailedLog += QCoreApplication::translate("Analysis", "检测到的边缘数量：%1\n").arg(edgeCount);
      result.detailedLog += QCoreApplication::translate("Analysis", "平均边缘宽度 (像素): %1\n").arg(avgResolution);
    } else {
      result.detailedLog += QCoreApplication::translate("Analysis", "未检测到足够的边缘，使用理论估计值\n");
    }
    result.detailedLog += QCoreApplication::translate("Analysis", "估计的方位分辨率：%1 m\n").arg(azimuthResolution);
    
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\n解释:\n"
        "- 方位分辨率表示在方位方向上能够分辨的最小距离\n"
        "- 较小的值表示更好的分辨能力\n"
        "- 典型的高分辨率 SAR 系统方位分辨率在 1-5 米范围");
    
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "%1 m").arg(azimuthResolution, 0, 'f', 2);
    result.success = true;
  } catch (const cv::Exception &e) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误：方位模糊度分析计算过程中出错：%1").arg(QString::fromStdString(e.what()));
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 计算失败");
    result.success = false;
  }
  
  return result;
} 