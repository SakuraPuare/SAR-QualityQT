#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/core.hpp> // minMaxLoc, meanStdDev
#include <opencv2/imgproc.hpp> // magnitude, split (如果需要处理复数)
#include <vector>

// 实现基础的辐射度量分析 (动态范围、均值、标准差)
void MainWindow::performRadiometricAnalysis() {
  QString resultLog = tr("Radiometric Analysis (Basic Statistics):\n");
  QString overviewResult = tr("Radiometric: ");

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded for radiometric analysis.");
    ui->method4ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Image"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat; // 用于分析的单通道图像

  // 1. 准备单通道图像，保留原始数据类型以获得准确的范围
  logMessage(tr("Preparing single-channel image for radiometric analysis..."));
  if (currentImage.channels() == 2) {
      logMessage(tr("Input is complex, calculating magnitude."));
      resultLog += tr("Using magnitude image calculated from complex data.\n");
      std::vector<cv::Mat> channels;
      cv::split(currentImage, channels);
      // 幅度计算需要浮点
      if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
          logMessage(tr("Converting complex channels to CV_32F for magnitude."));
          channels[0].convertTo(channels[0], CV_32F);
          channels[1].convertTo(channels[1], CV_32F);
      }
      cv::magnitude(channels[0], channels[1], analysisMat);
  } else if (currentImage.channels() == 1) {
       logMessage(tr("Using single-channel input directly."));
       resultLog += tr("Using single-channel input data.\n");
       analysisMat = currentImage.clone(); // 克隆以防修改
  } else {
       // 对于多通道非复数图像，使用第一个通道进行基础统计
       logMessage(tr("Input is multi-channel (%1), using first channel for basic stats.").arg(currentImage.channels()));
       resultLog += tr("Using the first channel of the multi-channel input.\n");
       std::vector<cv::Mat> channels;
       cv::split(currentImage, channels);
       analysisMat = channels[0].clone();
  }

   // 检查是否成功获取单通道图像
   if (analysisMat.empty() || analysisMat.channels() != 1) {
       logMessage(tr("Error: Failed to obtain single-channel image for radiometric analysis."));
       resultLog += tr("\nError: Failed to prepare single-channel data.");
       ui->method4ResultsTextEdit->setText(resultLog);
       ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
       return;
   }

  // 2. (可选) 确保数据类型为浮点型以进行精确统计计算
  // meanStdDev 和 minMaxLoc 可以处理多种类型，但转换为浮点可以避免整数运算的潜在问题
  if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
      logMessage(tr("Converting image data to 32-bit float (CV_32F) for precise radiometric statistics."));
      resultLog += tr("Converting data to floating-point (CV_32F) for analysis.\n");
      analysisMat.convertTo(analysisMat, CV_32F);
  }


  // 3. 计算基本统计量
  try {
        logMessage(tr("Calculating min, max, mean, and standard deviation..."));
        double minVal = 0.0, maxVal = 0.0;
        cv::minMaxLoc(analysisMat, &minVal, &maxVal); // 查找最小值和最大值

        cv::Scalar meanValue, stdDevValue;
        // 计算均值和标准差
        // 注意：对于非常大的图像，这可能需要一些时间
        cv::meanStdDev(analysisMat, meanValue, stdDevValue);
        double mean = meanValue[0];   // 均值 (灰度图像取第一个通道)
        double stddev = stdDevValue[0]; // 标准差 (灰度图像取第一个通道)

        resultLog += tr("\n--- Basic Radiometric Statistics ---\n");
        resultLog += tr("Minimum Value: %1\n").arg(minVal);
        resultLog += tr("Maximum Value: %1\n").arg(maxVal);
        // 动态范围：衡量图像像素值的跨度
        resultLog += tr("Dynamic Range (Max - Min): %1\n").arg(maxVal - minVal);
        // 均值：图像像素的平均强度
        resultLog += tr("Mean Value (μ): %1\n").arg(mean);
        // 标准差：衡量像素值相对于均值的离散程度，可反映噪声或纹理水平
        resultLog += tr("Standard Deviation (σ): %1\n").arg(stddev);

        resultLog += tr("\nInterpretation:\n"
                        "- Dynamic Range indicates the spread of intensity values.\n"
                        "- Mean represents the average brightness or intensity.\n"
                        "- Standard Deviation provides an estimate of noise level or texture variability.");
        // 可选：计算等效比特数 (ENOB) 估计，例如 log2(Dynamic Range / StdDev)，但这需要谨慎解释
        // double enob_estimate = (stddev > 1e-9) ? log2((maxVal - minVal) / stddev) : 0.0;
        // resultLog += tr("Estimated Number of Bits (ENOB proxy): %1\n").arg(enob_estimate);

        // 格式化概览结果 (使用 'g' 格式自动选择最佳表示)
        overviewResult += tr("Range=%1, StdDev=%2")
                             .arg(QString::number(maxVal - minVal, 'g', 4)) // 4 位有效数字
                             .arg(QString::number(stddev, 'g', 4));
        logMessage(tr("Radiometric stats calculated: Min=%1, Max=%2, Mean=%3, StdDev=%4")
                        .arg(minVal).arg(maxVal).arg(mean).arg(stddev));

    } catch (const cv::Exception& e) {
        resultLog += tr("\nError during radiometric statistics calculation: %1")
                       .arg(QString::fromStdString(e.what()));
        overviewResult += tr("Error - Calculation Failed");
        logMessage(tr("OpenCV Error during Radiometric calculation: %1")
                     .arg(QString::fromStdString(e.what())));
    }

  // 更新 UI
  ui->method4ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
