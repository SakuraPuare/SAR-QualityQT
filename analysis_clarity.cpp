#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/imgproc.hpp> // Sobel, addWeighted, mean, convertScaleAbs
#include <vector>

// 实现从 mainwindow.cpp 移动到这里
void MainWindow::performClarityAnalysis() {
  // Basic implementation using average gradient magnitude
  QString resultLog = tr("Clarity Analysis (Average Gradient Magnitude):\n");
  QString overviewResult = tr("Clarity (GradMag): ");

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded.");
    ui->method3ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Image"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat;
  QString prepareLog;
  // Reuse GLCM preparation logic as it results in CV_8UC1 needed for Sobel
  analysisMat = prepareImageForGLCM(currentImage, prepareLog);
  resultLog += prepareLog;

  if (analysisMat.empty() || analysisMat.type() != CV_8UC1) {
      resultLog += tr("Error: Could not prepare 8-bit single channel image for gradient calculation.");
      ui->method3ResultsTextEdit->setText(resultLog);
      ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
      logMessage(resultLog);
      return;
  }

   try {
        cv::Mat grad_x, grad_y;
        cv::Mat abs_grad_x, abs_grad_y;
        cv::Mat grad;

        // Calculate gradients (Sobel)
        cv::Sobel(analysisMat, grad_x, CV_16S, 1, 0, 3); // Use 16S to avoid overflow
        cv::Sobel(analysisMat, grad_y, CV_16S, 0, 1, 3);

        // Convert gradients back to absolute 8U representation (optional but common)
        cv::convertScaleAbs(grad_x, abs_grad_x);
        cv::convertScaleAbs(grad_y, abs_grad_y);

        // Combine gradients (approximate magnitude)
        cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

        // Calculate average gradient magnitude
        cv::Scalar avgGrad = cv::mean(grad);
        double averageGradient = avgGrad[0];

        resultLog += tr("\n--- Gradient Analysis ---\n");
        resultLog += tr("Average Gradient Magnitude: %1\n").arg(averageGradient);
        resultLog += tr("\nNote: Higher values generally indicate sharper details or more texture/noise.");

        overviewResult += tr("%1").arg(QString::number(averageGradient, 'f', 2));
        logMessage(tr("Clarity (Avg Grad Mag) calculated: %1").arg(averageGradient));

    } catch (const cv::Exception& e) {
        resultLog += tr("\nError during gradient calculation: %1")
                       .arg(QString::fromStdString(e.msg));
        overviewResult += tr("Error - Calculation Failed");
        logMessage(tr("OpenCV Error during Clarity calculation: %1")
                     .arg(QString::fromStdString(e.msg)));
    }

  ui->method3ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
