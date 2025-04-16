#include "mainwindow.h" // 需要包含 MainWindow 头文件以访问成员和 UI
#include "./ui_mainwindow.h" // 访问 UI 元素
#include <opencv2/core.hpp>    // meanStdDev, split, magnitude, convertTo
#include <opencv2/imgproc.hpp> // (可能需要，如果用到其他处理)
#include <vector> // std::vector
#include <cmath> // std::abs, std::sqrt

// 实现 SNR (信噪比) 和 ENL (等效视数) 分析
// 注意：这些指标通常在均匀区域计算才有意义。全局计算可能受场景内容影响。
void MainWindow::performSNRAnalysis(const cv::Mat& inputImage) {
  QString resultLog = tr("SNR/ENL Analysis Results (Global):\n");
  QString overviewResult = tr("SNR/ENL (Global): "); // 指明是全局计算

  if (inputImage.empty()) {
    resultLog = tr("Error: No valid image data provided for SNR/ENL analysis.");
    ui->method1ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Data"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat; // 用于分析的单通道浮点图像 (幅度或强度)

  // 1. 准备用于分析的单通道浮点图像
  logMessage(tr("Preparing single-channel float image for SNR/ENL analysis..."));
  if (inputImage.channels() == 2) {
    // 处理复数数据：计算幅度
    logMessage(tr("Input is complex (2-channel), calculating magnitude."));
    std::vector<cv::Mat> channels;
    cv::split(inputImage, channels);
    // 确保通道是浮点类型
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        logMessage(tr("Converting complex channels to CV_32F for magnitude calculation."));
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat); // 幅度图是单通道浮点型
    resultLog += tr("Using magnitude image calculated from complex data.\n");
  } else if (inputImage.channels() == 1) {
    // 输入已经是单通道（例如强度图像）
    logMessage(tr("Input is single-channel."));
    analysisMat = inputImage.clone(); // 克隆以防修改
    // 确保是浮点类型，因为 SNR/ENL 计算涉及除法，需要精度
    if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
        logMessage(tr("Converting single-channel image (type: %1) to 32-bit float (CV_32F) for analysis.")
                       .arg(cv::typeToString(analysisMat.type())));
        analysisMat.convertTo(analysisMat, CV_32F);
        resultLog += tr("Converted input to floating-point type (CV_32F).\n");
    } else {
         resultLog += tr("Using existing single-channel floating-point data.\n");
    }
  } else {
    // 不支持其他通道数的输入进行此分析
     resultLog = tr("Error: Unsupported channel count (%1) for SNR/ENL analysis. Expected 1 (intensity/magnitude) or 2 (complex).")
                .arg(inputImage.channels());
     ui->method1ResultsTextEdit->setText(resultLog);
     ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Unsupported Channels"));
     logMessage(resultLog);
     return;
  }

   // 检查准备步骤是否成功
   if (analysisMat.empty() || analysisMat.channels() != 1 || (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F)) {
       logMessage(tr("Error: Failed to prepare a valid single-channel float image for SNR/ENL analysis."));
       resultLog += tr("\nError: Failed to prepare suitable data for analysis.");
       ui->method1ResultsTextEdit->setText(resultLog);
       ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
       return;
   }

  // 2. 计算全局均值和标准差
  cv::Scalar meanValue, stdDevValue;
  try {
      logMessage(tr("Calculating global mean and standard deviation..."));
      // 计算整个图像的均值和标准差
      cv::meanStdDev(analysisMat, meanValue, stdDevValue);

      double mean = meanValue[0];   // 均值 μ
      double stddev = stdDevValue[0]; // 标准差 σ

      resultLog += tr("\n--- Global Statistics ---\n");
      resultLog += tr("Mean (μ): %1\n").arg(mean);
      resultLog += tr("Standard Deviation (σ): %1\n").arg(stddev);

      // 3. 计算 SNR 和 ENL
      // SNR = μ / σ  (信号均值 / 噪声标准差)
      // ENL = (μ / σ)^2 = SNR^2 (等效视数，衡量相干斑抑制程度)
      if (stddev > 1e-9) { // 避免除以零或非常小的标准差导致结果无意义或溢出
          double snr = mean / stddev;
          double enl = snr * snr; // 或 (mean * mean) / (stddev * stddev)

          resultLog += tr("\n--- Quality Metrics (Global) ---\n");
          resultLog += tr("Signal-to-Noise Ratio (SNR = μ/σ): %1\n").arg(snr);
          resultLog += tr("Equivalent Number of Looks (ENL = (μ/σ)²): %1\n").arg(enl);

          // 格式化概览结果
          overviewResult += tr("SNR=%.2f, ENL=%.2f")
                               .arg(snr)
                               .arg(enl);
          logMessage(tr("SNR/ENL calculated (Global): Mean=%1, StdDev=%2, SNR=%3, ENL=%4")
                         .arg(mean).arg(stddev).arg(snr).arg(enl));
      } else {
          // 标准差接近零，通常发生在图像值恒定的情况下
          resultLog += tr("\nWarning: Standard deviation is close to zero (σ = %1). Cannot calculate SNR/ENL reliably.").arg(stddev);
          overviewResult += tr("N/A (σ ≈ 0)");
          logMessage(tr("SNR/ENL calculation skipped: Standard deviation is near zero."));
      }

       resultLog += tr("\n\nNote: These metrics are calculated globally across the entire image. "
                       "For more meaningful assessment of speckle noise characteristics, "
                       "it is recommended to calculate SNR/ENL over a statistically homogeneous region "
                       "(e.g., calm water body, flat field).");

  } catch (const cv::Exception& e) {
      resultLog += tr("\nError during mean/standard deviation calculation: %1")
                       .arg(QString::fromStdString(e.what()));
      overviewResult += tr("Error - Calculation Failed");
      logMessage(tr("OpenCV Error during SNR/ENL (meanStdDev) calculation: %1")
                     .arg(QString::fromStdString(e.what())));
  }


  // 4. 更新 UI
  ui->method1ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
