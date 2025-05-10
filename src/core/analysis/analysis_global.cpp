#include "analysis_utils.h"
#include <QCoreApplication> // For translate()
#include <QString>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <cmath>

// 噪声等效后向散射系数（Noise Equivalent Sigma Zero, NESZ）分析
AnalysisResult performNESZAnalysis(const cv::Mat &inputImage) {
  AnalysisResult result;
  result.analysisName = QCoreApplication::translate("Analysis", "噪声等效后向散射系数 (NESZ)");
  result.detailedLog = QCoreApplication::translate("Analysis", "噪声等效后向散射系数 (NESZ) 分析结果:\n");
  QString overviewPrefix = QCoreApplication::translate("Analysis", "NESZ: ");

  if (inputImage.empty()) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误: 未提供有效的图像数据。");
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
      result.detailedLog += QCoreApplication::translate("Analysis", "\n错误: 不支持的通道数量 (%1)。需要单通道或双通道图像。").arg(inputImage.channels());
      result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 不支持的格式");
      result.success = false;
      return result;
    }

    // 计算均匀区域的噪声水平
    // 方法1: 通过标准差估计
    cv::Mat stdMap;
    cv::Mat meanMap;
    
    // 使用局部窗口计算标准差
    int windowSize = 7; // 窗口大小
    cv::Mat localMean, localStdDev;
    cv::blur(analysisMat, localMean, cv::Size(windowSize, windowSize));
    cv::Mat squaredDiff;
    cv::pow(analysisMat - localMean, 2.0, squaredDiff);
    cv::blur(squaredDiff, localStdDev, cv::Size(windowSize, windowSize));
    cv::sqrt(localStdDev, localStdDev);
    
    // 查找最小标准差区域 (假设为暗区/噪声区)
    double minStdVal, maxStdVal;
    cv::Point minStdLoc, maxStdLoc;
    cv::minMaxLoc(localStdDev, &minStdVal, &maxStdVal, &minStdLoc, &maxStdLoc);
    
    // 计算该区域的均值作为噪声等效值
    cv::Rect noiseRegion(std::max(0, minStdLoc.x - windowSize/2),
                         std::max(0, minStdLoc.y - windowSize/2),
                         windowSize, windowSize);
    noiseRegion.width = std::min(noiseRegion.width, analysisMat.cols - noiseRegion.x);
    noiseRegion.height = std::min(noiseRegion.height, analysisMat.rows - noiseRegion.y);
    
    cv::Mat noiseROI = analysisMat(noiseRegion);
    cv::Scalar noiseMean, noiseStdDev;
    cv::meanStdDev(noiseROI, noiseMean, noiseStdDev);
    
    // 计算整体图像的均值和最大值
    cv::Scalar globalMean, globalStdDev;
    cv::meanStdDev(analysisMat, globalMean, globalStdDev);
    double maxVal;
    cv::minMaxLoc(analysisMat, nullptr, &maxVal);
    
    // 计算噪声等效后向散射系数
    double nesz = noiseMean[0] / maxVal;
    double neszDB = 10 * std::log10(nesz);
    
    result.detailedLog += QCoreApplication::translate("Analysis", "\n--- 噪声等效后向散射系数分析 ---\n");
    result.detailedLog += QCoreApplication::translate("Analysis", "噪声区域均值: %1\n").arg(noiseMean[0]);
    result.detailedLog += QCoreApplication::translate("Analysis", "全局最大值: %1\n").arg(maxVal);
    result.detailedLog += QCoreApplication::translate("Analysis", "噪声等效后向散射系数 (NESZ): %1\n").arg(nesz);
    result.detailedLog += QCoreApplication::translate("Analysis", "噪声等效后向散射系数 (NESZ, dB): %1 dB\n").arg(neszDB);
    
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\n解释:\n"
        "- NESZ 是测量 SAR 系统噪声水平的参数\n"
        "- 较低的 NESZ 值表示系统噪声较低，能够检测到较弱的雷达回波\n"
        "- 高质量 SAR 图像的 NESZ 通常在 -20dB 到 -25dB 之间");
    
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "%1 dB").arg(neszDB, 0, 'f', 2);
    result.success = true;
  } catch (const cv::Exception &e) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误: NESZ 分析计算过程中出错: %1").arg(QString::fromStdString(e.what()));
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 计算失败");
    result.success = false;
  }
  
  return result;
}

// 辐射分辨率（Radiometric Resolution）分析
AnalysisResult performRadiometricResolutionAnalysis(const cv::Mat &inputImage) {
  AnalysisResult result;
  result.analysisName = QCoreApplication::translate("Analysis", "辐射分辨率");
  result.detailedLog = QCoreApplication::translate("Analysis", "辐射分辨率分析结果:\n");
  QString overviewPrefix = QCoreApplication::translate("Analysis", "辐射分辨率: ");

  if (inputImage.empty()) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误: 未提供有效的图像数据。");
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
      result.detailedLog += QCoreApplication::translate("Analysis", "\n错误: 不支持的通道数量 (%1)。需要单通道或双通道图像。").arg(inputImage.channels());
      result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 不支持的格式");
      result.success = false;
      return result;
    }

    // 计算整体图像的均值和标准差
    cv::Scalar meanVal, stdDevVal;
    cv::meanStdDev(analysisMat, meanVal, stdDevVal);
    double mean = meanVal[0];
    double stddev = stdDevVal[0];
    
    // 计算辐射分辨率 (基于对数)
    double enl = (mean / stddev) * (mean / stddev); // 等效视数
    double radiometricResolution = 10 * std::log10(1 + 1 / std::sqrt(enl));
    
    result.detailedLog += QCoreApplication::translate("Analysis", "\n--- 辐射分辨率分析 ---\n");
    result.detailedLog += QCoreApplication::translate("Analysis", "均值: %1\n").arg(mean);
    result.detailedLog += QCoreApplication::translate("Analysis", "标准差: %1\n").arg(stddev);
    result.detailedLog += QCoreApplication::translate("Analysis", "估计的等效视数 (ENL): %1\n").arg(enl);
    result.detailedLog += QCoreApplication::translate("Analysis", "辐射分辨率: %1 dB\n").arg(radiometricResolution);
    
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\n解释:\n"
        "- 辐射分辨率描述系统区分不同后向散射强度的能力\n"
        "- 较低的值表示系统能够区分更细微的反射差异\n"
        "- 通常与等效视数 (ENL) 相关，ENL 越高，辐射分辨率越好\n"
        "- 典型的辐射分辨率值在 1-3 dB 之间");
    
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "%1 dB").arg(radiometricResolution, 0, 'f', 2);
    result.success = true;
  } catch (const cv::Exception &e) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误: 辐射分辨率分析计算过程中出错: %1").arg(QString::fromStdString(e.what()));
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 计算失败");
    result.success = false;
  }
  
  return result;
}

// 等效视数（Equivalent Number of Looks, ENL）分析
AnalysisResult performENLAnalysis(const cv::Mat &inputImage) {
  AnalysisResult result;
  result.analysisName = QCoreApplication::translate("Analysis", "等效视数 (ENL)");
  result.detailedLog = QCoreApplication::translate("Analysis", "等效视数 (ENL) 分析结果:\n");
  QString overviewPrefix = QCoreApplication::translate("Analysis", "ENL: ");

  if (inputImage.empty()) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误: 未提供有效的图像数据。");
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
      result.detailedLog += QCoreApplication::translate("Analysis", "\n错误: 不支持的通道数量 (%1)。需要单通道或双通道图像。").arg(inputImage.channels());
      result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 不支持的格式");
      result.success = false;
      return result;
    }

    // 方法1: 全局 ENL 计算
    cv::Scalar globalMean, globalStdDev;
    cv::meanStdDev(analysisMat, globalMean, globalStdDev);
    double globalENL = (globalMean[0] * globalMean[0]) / (globalStdDev[0] * globalStdDev[0]);
    
    // 方法2: 使用滑动窗口计算局部 ENL 并找出最大值（假设为同质区域）
    int windowSize = 15; // 窗口大小
    double maxLocalENL = 0.0;
    cv::Point maxENLLocation;
    
    // 为提高效率，步长可以设置大一些
    int stepSize = 5;
    
    for (int i = 0; i <= analysisMat.rows - windowSize; i += stepSize) {
      for (int j = 0; j <= analysisMat.cols - windowSize; j += stepSize) {
        cv::Rect roi(j, i, windowSize, windowSize);
        cv::Mat roiMat = analysisMat(roi);
        
        cv::Scalar localMean, localStdDev;
        cv::meanStdDev(roiMat, localMean, localStdDev);
        
        // 避免除以零
        if (localStdDev[0] > 1e-6) {
          double localENL = (localMean[0] * localMean[0]) / (localStdDev[0] * localStdDev[0]);
          if (localENL > maxLocalENL) {
            maxLocalENL = localENL;
            maxENLLocation = cv::Point(j + windowSize/2, i + windowSize/2);
          }
        }
      }
    }
    
    result.detailedLog += QCoreApplication::translate("Analysis", "\n--- 等效视数分析 ---\n");
    result.detailedLog += QCoreApplication::translate("Analysis", "全局均值: %1\n").arg(globalMean[0]);
    result.detailedLog += QCoreApplication::translate("Analysis", "全局标准差: %1\n").arg(globalStdDev[0]);
    result.detailedLog += QCoreApplication::translate("Analysis", "全局等效视数 (ENL): %1\n").arg(globalENL);
    result.detailedLog += QCoreApplication::translate("Analysis", "检测到的最大局部 ENL: %1 (位置: %2, %3)\n")
                              .arg(maxLocalENL)
                              .arg(maxENLLocation.x)
                              .arg(maxENLLocation.y);
    
    // 确定最终报告的 ENL 值 (优先使用局部最大值，因为全局计算往往低估 ENL)
    double reportedENL = maxLocalENL > 0 ? maxLocalENL : globalENL;
    
    result.detailedLog += QCoreApplication::translate(
        "Analysis",
        "\n解释:\n"
        "- 等效视数 (ENL) 表示多视处理中的有效视数\n"
        "- 更高的 ENL 值表示更好的斑点噪声抑制效果\n"
        "- ENL = (均值/标准差)²\n"
        "- 局部最大 ENL 值通常来自图像中最均匀的区域\n"
        "- 单视复图像的理论 ENL = 1, 多视处理后 ENL 通常显著提高");
    
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "%1").arg(reportedENL, 0, 'f', 2);
    result.success = true;
  } catch (const cv::Exception &e) {
    result.detailedLog += QCoreApplication::translate("Analysis", "\n错误: ENL 分析计算过程中出错: %1").arg(QString::fromStdString(e.what()));
    result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "错误 - 计算失败");
    result.success = false;
  }
  
  return result;
} 