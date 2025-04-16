#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/imgproc.hpp> // Sobel, addWeighted, mean, convertScaleAbs
#include <vector>

// Implement Clarity Analysis (based on Average Gradient Magnitude)
void MainWindow::performClarityAnalysis(const cv::Mat& inputImage) { // Accept inputImage parameter
  QString resultLog = tr("Clarity Analysis (Average Gradient Magnitude):\n");
  QString overviewResult = tr("Clarity (GradMag): ");

  if (inputImage.empty()) { // Use inputImage parameter
    resultLog = tr("Error: No valid image data provided for clarity analysis."); // Updated error message
    ui->method3ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Data")); // Updated overview error
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat;
  QString prepareLog;
  // Reuse GLCM's image preparation logic, as it generates the CV_8UC1 grayscale image needed for Sobel
  logMessage(tr("Preparing 8-bit grayscale image for clarity analysis..."));
  // Note: prepareImageForGLCM needs the original image data, not a potentially modified one.
  // We pass the inputImage received by this function.
  analysisMat = prepareImageForGLCM(inputImage, prepareLog); // Use inputImage parameter
  resultLog += prepareLog;

  // Check if the prepared image is valid
  if (analysisMat.empty() || analysisMat.type() != CV_8UC1) {
      resultLog += tr("\nError: Failed to prepare 8-bit single channel image for gradient calculation.");
      ui->method3ResultsTextEdit->setText(resultLog);
      ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
      logMessage(resultLog);
      return;
  }

   try {
        cv::Mat grad_x, grad_y; // Store gradients in X and Y directions (intermediate results)
        cv::Mat abs_grad_x, abs_grad_y; // Store absolute values of gradients
        cv::Mat grad; // Store the combined approximate gradient magnitude

        // Calculate gradients in X and Y directions (Sobel operator)
        // Use CV_16S (16-bit signed integer) as output type to prevent overflow during gradient calculation [-255*4, 255*4] range
        cv::Sobel(analysisMat, grad_x, CV_16S, 1, 0, 3); // ddepth=CV_16S, dx=1, dy=0, ksize=3
        cv::Sobel(analysisMat, grad_y, CV_16S, 0, 1, 3); // ddepth=CV_16S, dx=0, dy=1, ksize=3

        // Convert gradient results back to CV_8U (8-bit unsigned integer)
        // convertScaleAbs calculates the absolute value and scales it back to the [0, 255] range
        cv::convertScaleAbs(grad_x, abs_grad_x);
        cv::convertScaleAbs(grad_y, abs_grad_y);

        // Combine gradients from X and Y directions to approximate the gradient magnitude
        // Use a weighted sum (here with weights 0.5 each) to approximate G = sqrt(Gx^2 + Gy^2) or G = |Gx| + |Gy|
        // addWeighted(src1, alpha, src2, beta, gamma, dst) -> dst = src1*alpha + src2*beta + gamma
        cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

        // Calculate the average value of the entire gradient magnitude image
        cv::Scalar avgGradScalar = cv::mean(grad); // Returns a Scalar, usually the first element is the mean for grayscale images
        double averageGradient = avgGradScalar[0];

        resultLog += tr("\n--- Gradient Analysis Results ---\n");
        resultLog += tr("Average Gradient Magnitude: %1\n").arg(averageGradient);
        resultLog += tr("\nInterpretation: Higher values generally indicate sharper details, edges, or potentially more texture/noise in the image.");

        overviewResult += tr("%1").arg(QString::number(averageGradient, 'f', 2)); // Format overview result
        logMessage(tr("Clarity analysis (Average Gradient Magnitude) completed: %1").arg(averageGradient));

    } catch (const cv::Exception& e) {
        resultLog += tr("\nError during gradient calculation: %1").arg(QString::fromStdString(e.what())); // Use e.what() for more detailed error
        overviewResult += tr("Error - Calculation Failed");
        logMessage(tr("OpenCV Error during Clarity (Gradient) calculation: %1").arg(QString::fromStdString(e.what())));
    }

  // Update UI to display results
  ui->method3ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
