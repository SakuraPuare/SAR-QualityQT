#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/imgproc.hpp> // Sobel, addWeighted, mean, convertScaleAbs
#include <vector>

// 实现清晰度分析 (基于平均梯度幅度)
void MainWindow::performClarityAnalysis() {
  QString resultLog = tr("Clarity Analysis (Average Gradient Magnitude):\n");
  QString overviewResult = tr("Clarity (GradMag): ");

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded for clarity analysis.");
    ui->method3ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Image"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat;
  QString prepareLog;
  // 复用 GLCM 的图像准备逻辑，因为它会生成 Sobel 需要的 CV_8UC1 灰度图像
  logMessage(tr("Preparing 8-bit grayscale image for clarity analysis..."));
  analysisMat = prepareImageForGLCM(currentImage, prepareLog);
  resultLog += prepareLog;

  // 检查准备后的图像是否有效
  if (analysisMat.empty() || analysisMat.type() != CV_8UC1) {
      resultLog += tr("\nError: Failed to prepare 8-bit single channel image for gradient calculation.");
      ui->method3ResultsTextEdit->setText(resultLog);
      ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
      logMessage(resultLog);
      return;
  }

   try {
        cv::Mat grad_x, grad_y; // 存储 X 和 Y 方向的梯度 (中间结果)
        cv::Mat abs_grad_x, abs_grad_y; // 存储梯度绝对值
        cv::Mat grad; // 存储合并后的梯度幅度近似值

        // 计算 X 和 Y 方向的梯度 (Sobel 算子)
        // 使用 CV_16S (16 位有符号整数) 作为输出类型，以防止计算过程中梯度值溢出 [-255*4, 255*4] 范围
        cv::Sobel(analysisMat, grad_x, CV_16S, 1, 0, 3); // ddepth=CV_16S, dx=1, dy=0, ksize=3
        cv::Sobel(analysisMat, grad_y, CV_16S, 0, 1, 3); // ddepth=CV_16S, dx=0, dy=1, ksize=3

        // 将梯度结果转换回 CV_8U (8 位无符号整数)
        // convertScaleAbs 计算绝对值并缩放回 [0, 255] 范围
        cv::convertScaleAbs(grad_x, abs_grad_x);
        cv::convertScaleAbs(grad_y, abs_grad_y);

        // 合并 X 和 Y 方向的梯度，得到近似的梯度幅度
        // 使用加权和 (这里权重各 0.5) 来近似 G = sqrt(Gx^2 + Gy^2) 或 G = |Gx| + |Gy|
        // addWeighted(src1, alpha, src2, beta, gamma, dst) -> dst = src1*alpha + src2*beta + gamma
        cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

        // 计算整个梯度幅度图像的平均值
        cv::Scalar avgGradScalar = cv::mean(grad); // 返回 Scalar，通常第一个元素是灰度图像的均值
        double averageGradient = avgGradScalar[0];

        resultLog += tr("\n--- Gradient Analysis Results ---\n");
        resultLog += tr("Average Gradient Magnitude: %1\n").arg(averageGradient);
        resultLog += tr("\nInterpretation: Higher values generally indicate sharper details, edges, or potentially more texture/noise in the image.");

        overviewResult += tr("%1").arg(QString::number(averageGradient, 'f', 2)); // 格式化概览结果
        logMessage(tr("Clarity analysis (Average Gradient Magnitude) completed: %1").arg(averageGradient));

    } catch (const cv::Exception& e) {
        resultLog += tr("\nError during gradient calculation: %1").arg(QString::fromStdString(e.what())); // 使用 e.what() 获取更详细错误
        overviewResult += tr("Error - Calculation Failed");
        logMessage(tr("OpenCV Error during Clarity (Gradient) calculation: %1").arg(QString::fromStdString(e.what())));
    }

  // 更新 UI 显示结果
  ui->method3ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
