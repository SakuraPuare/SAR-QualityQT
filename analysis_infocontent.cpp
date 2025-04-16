#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/imgproc.hpp> // calcHist, cvtColor, magnitude, split
#include <vector>
#include <cmath> // log2f

// 实现信息量 (熵) 分析
void MainWindow::performInfoContentAnalysis() {
  QString resultLog; // 用于详细结果文本框
  QString overviewResult = tr("Information Content (Entropy): "); // 用于概览

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded for entropy analysis.");
    ui->method2ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Image"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat; // 用于计算熵的单通道图像

  // 1. 准备单通道图像 (幅度或灰度)
  logMessage(tr("Preparing single-channel image for entropy analysis..."));
  if (currentImage.channels() == 2) {
    // 处理复数数据：计算幅度
    logMessage(tr("Input is complex (2-channel), calculating magnitude."));
    std::vector<cv::Mat> channels;
    cv::split(currentImage, channels);
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        logMessage(tr("Converting complex channels to CV_32F for magnitude."));
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat); // 幅度图是单通道浮点型
    resultLog = tr("Calculated magnitude from complex data.\n");
  } else if (currentImage.channels() > 1) {
    // 处理多通道图像：转换为灰度图
    logMessage(tr("Input is multi-channel (%1), converting to grayscale.").arg(currentImage.channels()));
     cv::Mat sourceForCvt = currentImage;
     if (currentImage.depth() != CV_8U && currentImage.depth() != CV_16U && currentImage.depth() != CV_32F) {
         logMessage(tr("Input depth not directly supported by cvtColor, converting to CV_8U first."));
         double minVal, maxVal;
         cv::minMaxLoc(currentImage, &minVal, &maxVal);
         int outputType = CV_MAKETYPE(CV_8U, currentImage.channels()); // 保持通道数，仅改变深度
         if (maxVal > minVal)
             currentImage.convertTo(sourceForCvt, outputType, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal));
         else
             currentImage.convertTo(sourceForCvt, outputType);
     }

     if (sourceForCvt.channels() >= 3) {
          try {
            int code = (sourceForCvt.channels() == 4) ? cv::COLOR_BGRA2GRAY : cv::COLOR_BGR2GRAY;
            cv::cvtColor(sourceForCvt, analysisMat, code);
             resultLog = tr("Converted multi-channel image to grayscale using standard conversion.\n");
          } catch (const cv::Exception& e) {
             logMessage(tr("Standard grayscale conversion failed (%1), falling back to first channel.").arg(QString::fromStdString(e.what())));
             std::vector<cv::Mat> channels;
             cv::split(sourceForCvt, channels);
             analysisMat = channels[0].clone();
             resultLog = tr("Converted multi-channel image to single channel using the first channel.\n");
          }
     } else {
        logMessage(tr("Input has %1 channels, not standard BGR/BGRA. Using first channel.").arg(sourceForCvt.channels()));
        std::vector<cv::Mat> channels;
        cv::split(sourceForCvt, channels);
        analysisMat = channels[0].clone();
        resultLog = tr("Used the first channel of the multi-channel image.\n");
     }
  } else if (currentImage.channels() == 1){
    // 已经是单通道图像
    analysisMat = currentImage.clone(); // 克隆以防修改
    resultLog = tr("Input is already single-channel.\n");
  } else {
     logMessage(tr("Error: Input image has 0 channels, cannot calculate entropy."));
     resultLog = tr("Error: Input image has 0 channels.");
     ui->method2ResultsTextEdit->setText(resultLog);
     ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Invalid Channels"));
     return;
  }

  // --- 检查是否成功获得单通道图像 ---
  if (analysisMat.empty() || analysisMat.channels() != 1) {
      logMessage(tr("Error: Could not obtain a valid single-channel image for entropy calculation."));
       resultLog += tr("\nError: Failed to get single-channel data for analysis.");
       ui->method2ResultsTextEdit->setText(resultLog);
       ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
       return;
  }


  // 2. 将单通道图像转换为 CV_8U，以便计算直方图
  // 熵的计算通常基于离散的灰度级概率，8 位 (0-255) 是常用标准。
  cv::Mat histMat; // 用于计算直方图的 8 位图像
  logMessage(tr("Converting single-channel image to 8-bit (CV_8U) for histogram calculation..."));
  if (analysisMat.depth() != CV_8U) {
    resultLog += tr("Normalizing data to 8-bit range (0-255) for histogram.\n");
    double minVal, maxVal;
    cv::minMaxLoc(analysisMat, &minVal, &maxVal);
    if (maxVal > minVal) {
      // 归一化并转换类型
      analysisMat.convertTo(histMat, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    } else {
      // 图像值恒定
      analysisMat.convertTo(histMat, CV_8U);
      resultLog += tr("Image has constant value; entropy is expected to be 0.\n");
    }
  } else {
    histMat = analysisMat; // 已经是 CV_8U
    resultLog += tr("Image is already 8-bit (CV_8U), using directly for histogram.\n");
  }

  // 检查转换后的 histMat 是否有效
   if (histMat.empty() || histMat.type() != CV_8UC1) {
      logMessage(tr("Error: Failed to produce a valid CV_8UC1 image for histogram calculation."));
       resultLog += tr("\nError: Failed to get 8-bit data for histogram.");
       ui->method2ResultsTextEdit->setText(resultLog);
       ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Conversion Failed"));
       return;
   }

  // 3. 计算灰度直方图
  cv::Mat hist;           // 输出直方图 (CV_32F 类型)
  int histSize = 256;       // 直方图的 bin 数量 (对应 0-255 灰度级)
  float range[] = {0, 256}; // 灰度值范围 [0, 256)
  const float *histRange[] = {range}; // 指向范围数组的指针
  bool uniform = true;      // bin 大小是否均匀
  bool accumulate = false;  // 是否累加到现有直方图 (否)
  float entropy = 0.0f;     // 初始化熵为 0

  try {
    logMessage(tr("Calculating histogram..."));
    cv::calcHist(&histMat,    // 输入图像数组 (只有一个)
                 1,           // 图像数量
                 0,           // 要计算直方图的通道索引 (通道 0)
                 cv::Mat(),   // 掩码 (不使用)
                 hist,        // 输出的直方图 Mat
                 1,           // 直方图维度 (灰度图为 1D)
                 &histSize,   // 每个维度的 bin 数量
                 histRange,   // 每个维度的值范围
                 uniform, accumulate);

    // 4. 归一化直方图，得到每个灰度级的概率 p_i
    // total() 返回 Mat 中的元素总数 (像素总数)
    double totalPixels = histMat.total();
    if (totalPixels > 0) {
        hist /= totalPixels; // hist 现在包含每个 bin 的概率
    } else {
         logMessage(tr("Warning: Image has zero pixels. Cannot calculate entropy."));
         resultLog += tr("\nWarning: Image contains no pixels.");
         ui->method2ResultsTextEdit->setText(resultLog);
         ui->overviewResultsTextEdit->append(overviewResult + tr("Warning - Empty Image"));
         return;
    }


    // 5. 计算香农熵 (Shannon Entropy)
    // H = - sum( p_i * log2(p_i) ) for all p_i > 0
    // 熵衡量了图像灰度分布的不确定性或信息的平均量，单位是 bits/pixel
    logMessage(tr("Calculating entropy from normalized histogram..."));
    for (int i = 0; i < histSize; i++) {
      float p = hist.at<float>(i); // 获取灰度级 i 的概率
      if (p > std::numeric_limits<float>::epsilon()) { // 仅处理概率大于 0 的项，避免 log2(0)
        entropy -= p * log2f(p); // 使用 log2f 计算以 2 为底的对数
      }
    }

    resultLog += tr("\n--- Entropy Calculation Result ---\n");
    resultLog += tr("Shannon Entropy: %1 bits/pixel").arg(entropy);
    resultLog += tr("\nInterpretation: Higher entropy generally indicates more complex texture or a wider range of gray levels used in the image.");

    overviewResult += tr("%.4f bits/pixel").arg(entropy); // 概览结果保留 4 位小数
    logMessage(tr("Entropy calculation successful: %1 bits/pixel").arg(entropy));

  } catch (const cv::Exception &e) {
    resultLog += tr("\nError during histogram or entropy calculation: %1")
                     .arg(QString::fromStdString(e.what()));
    overviewResult += tr("Error - Calculation Failed");
    logMessage(tr("OpenCV Error during Entropy calculation: %1")
                   .arg(QString::fromStdString(e.what())));
     // 即使出错，也要显示日志
     ui->method2ResultsTextEdit->setText(resultLog);
     ui->overviewResultsTextEdit->append(overviewResult);
     return; // 提前返回
  }

  // --- 更新 UI ---
  ui->method2ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
