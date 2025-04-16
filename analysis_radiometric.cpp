#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/core.hpp> // minMaxLoc, meanStdDev
#include <opencv2/imgproc.hpp> // magnitude, split (如果需要处理复数)
#include <vector>

// Implement basic Radiometric Analysis (Dynamic Range, Mean, Standard Deviation)
void MainWindow::performRadiometricAnalysis(const cv::Mat& inputImage) { // Accept inputImage parameter
  QString resultLog = tr("Radiometric Analysis (Basic Statistics):\n");
  QString overviewResult = tr("Radiometric: ");

  if (inputImage.empty()) { // Use inputImage parameter
    resultLog = tr("Error: No valid image data provided for radiometric analysis."); // Updated error message
    ui->method4ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Data")); // Updated overview error
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat; // Single-channel image for analysis

  // 1. Prepare single-channel image, preserving original data type for accurate range
  logMessage(tr("Preparing single-channel image for radiometric analysis..."));
  if (inputImage.channels() == 2) { // Use inputImage parameter
      logMessage(tr("Input is complex, calculating magnitude."));
      resultLog += tr("Using magnitude image calculated from complex data.\n");
      std::vector<cv::Mat> channels;
      cv::split(inputImage, channels); // Use inputImage parameter
      // Magnitude calculation requires float
      if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
          logMessage(tr("Converting complex channels to CV_32F for magnitude."));
          channels[0].convertTo(channels[0], CV_32F);
          channels[1].convertTo(channels[1], CV_32F);
      }
      cv::magnitude(channels[0], channels[1], analysisMat);
  } else if (inputImage.channels() == 1) { // Use inputImage parameter
       logMessage(tr("Using single-channel input directly."));
       resultLog += tr("Using single-channel input data.\n");
       analysisMat = inputImage.clone(); // Clone to prevent modification
  } else {
       // For multi-channel non-complex images, use the first channel for basic stats
       logMessage(tr("Input is multi-channel (%1), using first channel for basic stats.").arg(inputImage.channels())); // Use inputImage parameter
       resultLog += tr("Using the first channel of the multi-channel input.\n");
       std::vector<cv::Mat> channels;
       cv::split(inputImage, channels); // Use inputImage parameter
       analysisMat = channels[0].clone();
  }

   // Check if single-channel image was successfully obtained
   if (analysisMat.empty() || analysisMat.channels() != 1) {
       logMessage(tr("Error: Failed to obtain single-channel image for radiometric analysis."));
       resultLog += tr("\nError: Failed to prepare single-channel data.");
       ui->method4ResultsTextEdit->setText(resultLog);
       ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
       return;
   }

  // 2. (Optional) Ensure data type is float for precise statistical calculation
  // meanStdDev and minMaxLoc can handle various types, but converting to float avoids potential integer arithmetic issues
  if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
      logMessage(tr("Converting image data to 32-bit float (CV_32F) for precise radiometric statistics."));
      resultLog += tr("Converting data to floating-point (CV_32F) for analysis.\n");
      analysisMat.convertTo(analysisMat, CV_32F);
  }


  // 3. Calculate basic statistics
  try {
        logMessage(tr("Calculating min, max, mean, and standard deviation..."));
        double minVal = 0.0, maxVal = 0.0;
        cv::minMaxLoc(analysisMat, &minVal, &maxVal); // Find minimum and maximum values

        cv::Scalar meanValue, stdDevValue;
        // Calculate mean and standard deviation
        // Note: For very large images, this might take some time
        cv::meanStdDev(analysisMat, meanValue, stdDevValue);
        double mean = meanValue[0];   // Mean (take first channel for grayscale)
        double stddev = stdDevValue[0]; // Standard deviation (take first channel for grayscale)

        resultLog += tr("\n--- Basic Radiometric Statistics ---\n");
        resultLog += tr("Minimum Value: %1\n").arg(minVal);
        resultLog += tr("Maximum Value: %1\n").arg(maxVal);
        // Dynamic Range: Measures the span of pixel values in the image
        resultLog += tr("Dynamic Range (Max - Min): %1\n").arg(maxVal - minVal);
        // Mean: Average intensity of the image pixels
        resultLog += tr("Mean Value (μ): %1\n").arg(mean);
        // Standard Deviation: Measures the dispersion of pixel values relative to the mean, can reflect noise or texture level
        resultLog += tr("Standard Deviation (σ): %1\n").arg(stddev);

        resultLog += tr("\nInterpretation:\n"
                        "- Dynamic Range indicates the spread of intensity values.\n"
                        "- Mean represents the average brightness or intensity.\n"
                        "- Standard Deviation provides an estimate of noise level or texture variability.");
        // Optional: Calculate Equivalent Number of Bits (ENOB) estimate, e.g., log2(Dynamic Range / StdDev), but interpret cautiously
        // double enob_estimate = (stddev > 1e-9) ? log2((maxVal - minVal) / stddev) : 0.0;
        // resultLog += tr("Estimated Number of Bits (ENOB proxy): %1\n").arg(enob_estimate);

        // Format overview result (use 'g' format for automatic best representation)
        overviewResult += tr("Range=%1, StdDev=%2")
                             .arg(QString::number(maxVal - minVal, 'g', 4)) // 4 significant digits
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

  // Update UI
  ui->method4ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
