#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/imgproc.hpp> // calcHist, cvtColor
#include <vector>
#include <cmath> // log2f

// 实现从 mainwindow.cpp 移动到这里
void MainWindow::performInfoContentAnalysis() {
  QString resultLog; // 用于在文本框中显示详细信息
  QString overviewResult = tr("Information Content: "); // 用于在总览中显示结果

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded.");
    ui->method2ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat; // 用于计算熵的矩阵

  // 检查数据是否为复数 (这里假设 2 通道表示之前加载的复数数据)
  // 注意：更可靠的方法是在 openImageFile 中存储 isComplex 标志并在需要时访问它
  if (currentImage.channels() == 2) {
    logMessage(tr("Input is 2-channel, calculating magnitude for entropy "
                  "analysis."));
    std::vector<cv::Mat> channels;
    cv::split(currentImage, channels);
    // 确保通道是浮点类型，如果不是则转换 (幅度计算通常需要浮点)
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        logMessage(tr("Converting complex channels to CV_32F for magnitude calculation."));
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat); // analysisMat 现在是单通道浮点型
    resultLog = tr("Calculated magnitude from complex data.\n");
  } else if (currentImage.channels() > 1) {
    // 对于非复数的多通道数据，转换为灰度图
    // 注意：如果原始多通道图像不是常见的 BGR/RGB 顺序，转换可能需要调整
    logMessage(tr("Input is multi-channel (%1), converting to grayscale for "
                  "entropy analysis.")
                   .arg(currentImage.channels()));
    // 尝试使用 COLOR_BGR2GRAY，如果失败或类型不支持，则用第一通道
    try {
         // 确保输入类型与 cvtColor 兼容，通常是 CV_8U, CV_16U, CV_32F
         cv::Mat sourceForCvt;
         if (currentImage.depth() != CV_8U && currentImage.depth() != CV_16U && currentImage.depth() != CV_32F) {
             // 选择一个合适的转换类型，例如归一化到 CV_8U 或转换为 CV_32F
             logMessage(tr("Converting multi-channel image to CV_8UC3 before grayscale conversion."));
             double minVal, maxVal;
             // 需要对每个通道找到全局 min/max 进行归一化，这里简化处理
             // find min/max across all channels? Or per channel? Assume global for now
             cv::minMaxLoc(currentImage, &minVal, &maxVal);
             if (maxVal > minVal)
                 currentImage.convertTo(sourceForCvt, CV_8UC3, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal));
             else
                 currentImage.convertTo(sourceForCvt, CV_8UC3); // Constant value
         } else {
             sourceForCvt = currentImage;
         }
         // 假设输入是 3 或 4 通道的 BGR(A) 图像，否则可能需要其他转换码
         if (sourceForCvt.channels() >= 3)
            cv::cvtColor(sourceForCvt, analysisMat, cv::COLOR_BGR2GRAY);
         else { // 如果不是标准的 3/4 通道，回退到第一通道
             logMessage(tr("Cannot determine standard color format, using first channel."));
             std::vector<cv::Mat> channels;
             cv::split(sourceForCvt, channels);
             analysisMat = channels[0];
         }

    } catch (const cv::Exception& e) {
        logMessage(tr("cvtColor failed (%1), falling back to first channel.")
                        .arg(QString::fromStdString(e.msg)));
         std::vector<cv::Mat> channels;
         cv::split(currentImage, channels);
         analysisMat = channels[0];
    }
     resultLog = tr("Converted multi-channel image to single channel.\n");

  } else {
    // 已经是单通道图像
    analysisMat = currentImage.clone(); // 克隆以防修改原始数据
    resultLog = tr("Input is single-channel.\n");
  }

  // --- 确保 analysisMat 是单通道 ---
  if (analysisMat.channels() != 1) {
      logMessage(tr("Error: Could not obtain single-channel image for entropy calculation."));
       resultLog += tr("\nError: Failed to get single-channel data.");
       ui->method2ResultsTextEdit->setText(resultLog);
       ui->overviewResultsTextEdit->append(overviewResult + tr("Error"));
       return;
  }


  // --- 将 analysisMat (单通道) 转换为 CV_8U 以计算直方图 ---
  cv::Mat histMat;
  if (analysisMat.depth() != CV_8U) {
    logMessage(tr("Normalizing single-channel image (type: %1) to 8-bit for histogram.")
                   .arg(cv::typeToString(analysisMat.type())));
    resultLog += tr("Normalized data to 8-bit range (0-255) for calculation.\n");
    // 归一化到 0-255
    double minVal, maxVal;
    cv::minMaxLoc(analysisMat, &minVal, &maxVal);
    if (maxVal > minVal) {
      // 使用 CV_64F 进行中间计算以提高精度
      cv::Mat tempMat;
      analysisMat.convertTo(tempMat, CV_64F);
      tempMat = (tempMat - minVal) * (255.0 / (maxVal - minVal));
      tempMat.convertTo(histMat, CV_8U); // 最后转回 CV_8U
    } else {
      // 处理图像值恒定的情况
      analysisMat.convertTo(histMat, CV_8U); // 结果将是单个值 (通常是 0)
      resultLog += tr("Image has constant value, entropy will be 0.\n");
    }
  } else {
    histMat = analysisMat; // 已经是 CV_8U
    resultLog += tr("Using existing 8-bit data directly for calculation.\n");
  }

  // --- 计算直方图 ---
  cv::Mat hist;
  int histSize = 256;       // 对于 CV_8U
  float range[] = {0, 256}; // CV_8U 的范围
  const float *histRange = {range};
  bool uniform = true;
  bool accumulate = false;
  float entropy = 0.0f; // 初始化熵

  try {
    cv::calcHist(&histMat, 1, // images
                 0,          // channels (use channel 0)
                 cv::Mat(),  // mask
                 hist,       // output histogram
                 1,          // histogram dimensions
                 &histSize,  // histogram size for each dimension
                 &histRange, // range for each dimension
                 uniform, accumulate);

    // 归一化直方图 (得到概率 p_i)
    hist /= histMat.total(); // histMat.total() 是像素总数

    // --- 计算熵 H = -sum(p_i * log2(p_i)) ---
    for (int i = 0; i < histSize; i++) {
      float p = hist.at<float>(i);
      if (p > 0) { // 避免 log2(0) 导致 -inf
        entropy -= p * log2f(p); // 使用 log2f 处理 float
      }
    }

    resultLog += tr("\nCalculated Entropy: %1 bits/pixel").arg(entropy);
    overviewResult +=
        tr("%1 bits/pixel").arg(QString::number(entropy, 'f', 4)); // 格式化概览结果
    logMessage(tr("Entropy calculation successful: %1 bits/pixel").arg(entropy));

  } catch (const cv::Exception &e) {
    resultLog += tr("\nError during histogram/entropy calculation: %1")
                     .arg(QString::fromStdString(e.msg));
    overviewResult += tr("Error");
    logMessage(tr("OpenCV Error during entropy calculation: %1")
                   .arg(QString::fromStdString(e.msg)));
     // 即使出错，也要显示日志
     ui->method2ResultsTextEdit->setText(resultLog);
     ui->overviewResultsTextEdit->append(overviewResult);
     return; // 提前返回
  }

  // --- 更新 UI ---
  ui->method2ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
