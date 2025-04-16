#include "mainwindow.h" // 需要包含 MainWindow 头文件以访问成员和 UI
#include "./ui_mainwindow.h" // 访问 UI 元素
#include <opencv2/imgproc.hpp> // meanStdDev
#include <vector> // std::vector
#include <cmath> // std::abs

// 实现从 mainwindow.cpp 移动到这里
void MainWindow::performSNRAnalysis() {
  QString resultLog = tr("SNR/ENL Analysis Results:\n");
  QString overviewResult = tr("SNR/ENL: ");

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded.");
    ui->method1ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Image"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat; // 用于分析的矩阵 (通常是强度或幅度)

  // 1. 准备用于分析的单通道幅度/强度图像
  if (currentImage.channels() == 2) {
    // 处理复数数据：计算幅度
    logMessage(tr("SNR Analysis: Input is complex (2-channel), calculating magnitude."));
    std::vector<cv::Mat> channels;
    cv::split(currentImage, channels);
    // 幅度计算需要浮点型
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        logMessage(tr("Converting complex channels to CV_32F for magnitude calculation."));
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat); // analysisMat 现在是单通道浮点型
    resultLog += tr("Using magnitude image calculated from complex data.\n");
  } else if (currentImage.channels() == 1) {
    // 已经是单通道（通常是强度或已处理的幅度）
    logMessage(tr("SNR Analysis: Input is single-channel."));
    analysisMat = currentImage.clone(); // 克隆以防修改
    // 如果不是浮点型，转换为浮点型以进行精确计算
    if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
        logMessage(tr("Converting single-channel image to CV_32F for analysis."));
        analysisMat.convertTo(analysisMat, CV_32F);
        resultLog += tr("Converted input to floating-point type (CV_32F).\n");
    } else {
         resultLog += tr("Using existing single-channel floating-point data.\n");
    }
  } else {
    // 不支持的通道数
     resultLog = tr("Error: Unsupported channel count (%1) for SNR analysis. Expected 1 (intensity/amplitude) or 2 (complex).")
                .arg(currentImage.channels());
     ui->method1ResultsTextEdit->setText(resultLog);
     ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Unsupported Channels"));
     logMessage(resultLog);
     return;
  }

  // 2. 计算均值和标准差
  cv::Scalar meanValue, stdDevValue;
  try {
      // 计算整个图像的均值和标准差
      // 注意：对于大图像，这可能比较耗时
      cv::meanStdDev(analysisMat, meanValue, stdDevValue);

      double mean = meanValue[0]; // 取第一个通道的值
      double stddev = stdDevValue[0];

      resultLog += tr("\n--- Global Statistics ---\n");
      resultLog += tr("Mean (μ): %1\n").arg(mean);
      resultLog += tr("Standard Deviation (σ): %1\n").arg(stddev);

      // 3. 计算 SNR 和 ENL
      if (stddev > 1e-9) { // 避免除以零或非常小的值
          double snr = mean / stddev;
          double enl = snr * snr; // ENL = (μ/σ)^2

          resultLog += tr("\n--- Quality Metrics (Global) ---\n");
          resultLog += tr("Signal-to-Noise Ratio (SNR = μ/σ): %1\n").arg(snr);
          resultLog += tr("Equivalent Number of Looks (ENL = (μ/σ)²): %1\n").arg(enl);

          overviewResult += tr("SNR=%1, ENL=%2 (Global)")
                               .arg(QString::number(snr, 'f', 2))
                               .arg(QString::number(enl, 'f', 2));
          logMessage(tr("SNR/ENL calculated (Global): Mean=%1, StdDev=%2, SNR=%3, ENL=%4")
                         .arg(mean).arg(stddev).arg(snr).arg(enl));
      } else {
          resultLog += tr("\nWarning: Standard deviation is close to zero. Cannot calculate SNR/ENL reliably.");
          overviewResult += tr("N/A (StdDev near zero)");
          logMessage(tr("SNR/ENL calculation skipped: Standard deviation is near zero."));
      }

       resultLog += tr("\nNote: These metrics are calculated globally. For more accurate results, select a homogeneous region.");

  } catch (const cv::Exception& e) {
      resultLog += tr("\nError during mean/stddev calculation: %1")
                       .arg(QString::fromStdString(e.msg));
      overviewResult += tr("Error - Calculation Failed");
      logMessage(tr("OpenCV Error during SNR/ENL calculation: %1")
                     .arg(QString::fromStdString(e.msg)));
  }


  // 4. 更新 UI
  ui->method1ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
