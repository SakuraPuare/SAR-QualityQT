#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/core.hpp> // minMaxLoc, meanStdDev
#include <opencv2/imgproc.hpp> // magnitude, split (needed if handling complex)
#include <vector>

// 实现从 mainwindow.cpp 移动到这里
void MainWindow::performRadiometricAnalysis() {
  // Basic implementation: Report dynamic range and standard deviation
  QString resultLog = tr("Radiometric Analysis (Basic):\n");
  QString overviewResult = tr("Radiometric: ");

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded.");
    ui->method4ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Image"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat;
  // Use original data if possible, avoid normalization to 8-bit here for range
   if (currentImage.channels() == 2) {
      logMessage(tr("Radiometric Analysis: Input is complex, calculating magnitude."));
      resultLog += tr("Using magnitude image calculated from complex data.\n");
      std::vector<cv::Mat> channels;
      cv::split(currentImage, channels);
      if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
          channels[0].convertTo(channels[0], CV_32F);
          channels[1].convertTo(channels[1], CV_32F);
      }
      cv::magnitude(channels[0], channels[1], analysisMat);
  } else if (currentImage.channels() == 1) {
       logMessage(tr("Radiometric Analysis: Using single-channel input."));
       resultLog += tr("Using single-channel input data.\n");
       analysisMat = currentImage.clone();
  } else {
       logMessage(tr("Radiometric Analysis: Input is multi-channel (%1), using first channel.")
                    .arg(currentImage.channels()));
       resultLog += tr("Using first channel of multi-channel input.\n");
       std::vector<cv::Mat> channels;
       cv::split(currentImage, channels);
       analysisMat = channels[0];
  }

  // Ensure float type for stats if not already
  if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
      logMessage(tr("Converting image to CV_32F for radiometric stats."));
      resultLog += tr("Converting data to floating point (CV_32F) for analysis.\n");
      analysisMat.convertTo(analysisMat, CV_32F);
  }


  try {
        double minVal, maxVal;
        cv::minMaxLoc(analysisMat, &minVal, &maxVal);

        cv::Scalar meanValue, stdDevValue;
        cv::meanStdDev(analysisMat, meanValue, stdDevValue);
        double mean = meanValue[0];
        double stddev = stdDevValue[0];

        resultLog += tr("\n--- Basic Radiometric Stats ---\n");
        resultLog += tr("Minimum Value: %1\n").arg(minVal);
        resultLog += tr("Maximum Value: %1\n").arg(maxVal);
        resultLog += tr("Dynamic Range (Max - Min): %1\n").arg(maxVal - minVal);
        resultLog += tr("Mean Value (μ): %1\n").arg(mean);
        resultLog += tr("Standard Deviation (σ): %1\n").arg(stddev);

        resultLog += tr("\nNote: Standard deviation provides an estimate of noise/texture level. Dynamic range indicates the spread of intensity values.");
        // Could add ENOB estimate if reasonable: e.g., log2((maxVal - minVal) / stddev)

        overviewResult += tr("Range=%1, StdDev=%2")
                             .arg(QString::number(maxVal - minVal, 'g', 3))
                             .arg(QString::number(stddev, 'g', 3));
        logMessage(tr("Radiometric stats calculated: Min=%1, Max=%2, Mean=%3, StdDev=%4")
                        .arg(minVal).arg(maxVal).arg(mean).arg(stddev));

    } catch (const cv::Exception& e) {
        resultLog += tr("\nError during radiometric calculation: %1")
                       .arg(QString::fromStdString(e.msg));
        overviewResult += tr("Error - Calculation Failed");
        logMessage(tr("OpenCV Error during Radiometric calculation: %1")
                     .arg(QString::fromStdString(e.msg)));
    }


  ui->method4ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
