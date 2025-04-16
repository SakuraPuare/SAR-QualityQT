#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/imgproc.hpp> // cvtColor, magnitude, split
#include <opencv2/core.hpp>    // minMaxLoc, Mat, Scalar, CV_Assert, sum
#include <vector>
#include <cmath> // std::abs, std::sqrt, log2f
#include <limits> // std::numeric_limits

// 实现从 mainwindow.cpp 移动到这里
void MainWindow::performGLCMAnalysis() {
  QString resultLog = tr("GLCM Analysis Results:\n");
  QString overviewResult = tr("GLCM: ");

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded.");
    ui->method5ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Image"));
    logMessage(resultLog);
    return;
  }

  // 1. Prepare 8-bit single channel image
  QString prepareLog;
  cv::Mat glcmInputMat = prepareImageForGLCM(currentImage, prepareLog);
  resultLog += prepareLog;
  logMessage(tr("Preparing image for GLCM..."));

  if (glcmInputMat.empty() || glcmInputMat.type() != CV_8UC1) {
      resultLog += tr("\nError: Failed to prepare 8-bit single channel image for GLCM.");
      ui->method5ResultsTextEdit->setText(resultLog);
      ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
      logMessage(resultLog);
      return;
  }

  // 2. Define GLCM parameters
  int levels = 256; // For CV_8U image
  // Define offsets (e.g., 0 degrees, 1 pixel distance)
  int dx = 1;
  int dy = 0;
  // Could compute for multiple directions/distances and average, but start simple
  resultLog += tr("\nCalculating GLCM with offset (dx=%1, dy=%2), levels=%3, symmetric=true\n")
                   .arg(dx).arg(dy).arg(levels);
  logMessage(tr("Calculating GLCM with offset dx=%1, dy=%2...").arg(dx).arg(dy));


  try {
        // 3. Compute GLCM
        cv::Mat glcm;
        computeGLCM(glcmInputMat, glcm, dx, dy, levels, true, true); // Symmetric, Normalized

        // 4. Calculate features
        double contrast = 0.0, correlation = 0.0, energy = 0.0, homogeneity = 0.0;
        calculateGLCMFeatures(glcm, levels, contrast, energy, homogeneity, correlation);

        resultLog += tr("\n--- Texture Features (Offset dx=%1, dy=%2) ---\n").arg(dx).arg(dy);
        resultLog += tr("Contrast: %1\n").arg(contrast);
        resultLog += tr("Correlation: %1\n").arg(correlation);
        resultLog += tr("Energy (ASM): %1\n").arg(energy);
        resultLog += tr("Homogeneity (IDM): %1\n").arg(homogeneity);

        // Format overview string
        overviewResult += tr("Contr=%1, Corr=%2, Ener=%3, Homo=%4")
                             .arg(QString::number(contrast, 'f', 3))
                             .arg(QString::number(correlation, 'f', 3))
                             .arg(QString::number(energy, 'f', 3))
                             .arg(QString::number(homogeneity, 'f', 3));

        logMessage(tr("GLCM features calculated: Contrast=%1, Correlation=%2, Energy=%3, Homogeneity=%4")
                    .arg(contrast).arg(correlation).arg(energy).arg(homogeneity));


  } catch (const cv::Exception& e) {
        resultLog += tr("\nError during GLCM calculation or feature extraction: %1")
                       .arg(QString::fromStdString(e.msg));
        overviewResult += tr("Error - Calculation Failed");
        logMessage(tr("OpenCV Error during GLCM analysis: %1")
                     .arg(QString::fromStdString(e.msg)));
  }

  // 5. Update UI
  ui->method5ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}

// Helper function to prepare image for GLCM (single channel, 8-bit)
// Implementation moved from mainwindow.cpp
cv::Mat MainWindow::prepareImageForGLCM(const cv::Mat& inputImage, QString& log)
{
    cv::Mat analysisMat;
    log = ""; // Clear log for this function

    // 1. Get single channel image (Magnitude for complex, Grayscale for color, direct for mono)
    if (inputImage.channels() == 2) {
        log += tr("Input is complex (2-channel), calculating magnitude.\n");
        std::vector<cv::Mat> channels;
        cv::split(inputImage, channels);
        if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
            log += tr("Converting complex channels to CV_32F for magnitude.\n");
            channels[0].convertTo(channels[0], CV_32F);
            channels[1].convertTo(channels[1], CV_32F);
        }
        cv::magnitude(channels[0], channels[1], analysisMat);
    } else if (inputImage.channels() > 1) {
         log += tr("Input is multi-channel (%1), converting to grayscale.\n").arg(inputImage.channels());
         cv::Mat sourceForCvt;
         // Ensure source type is suitable for cvtColor (e.g., CV_8U, CV_16U, CV_32F)
         if (inputImage.depth() != CV_8U && inputImage.depth() != CV_16U && inputImage.depth() != CV_32F) {
             log += tr("Converting multi-channel image to CV_8U before grayscale.\n");
             // Simple normalization/conversion to CV_8U for compatibility
              double minVal, maxVal;
              cv::minMaxLoc(inputImage, &minVal, &maxVal); // Crude range estimate
              if (maxVal > minVal)
                inputImage.convertTo(sourceForCvt, CV_8U, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal));
              else
                inputImage.convertTo(sourceForCvt, CV_8U); // Constant value
         } else {
             sourceForCvt = inputImage;
         }

         // Assuming BGR(A) for color conversion, otherwise fallback
         if (sourceForCvt.channels() >= 3) {
              try {
                // Try standard conversion first
                int code = cv::COLOR_BGR2GRAY; // Assume BGR
                if (sourceForCvt.channels() == 4) code = cv::COLOR_BGRA2GRAY;
                cv::cvtColor(sourceForCvt, analysisMat, code);
              } catch (const cv::Exception& e) { // Catch potential errors if not BGR/BGRA
                 log += tr("Standard grayscale conversion failed (%1), using first channel.\n").arg(QString::fromStdString(e.msg));
                 std::vector<cv::Mat> channels;
                 cv::split(sourceForCvt, channels);
                 analysisMat = channels[0];
              }
         } else {
            log += tr("Non-standard multi-channel (%1 channels), using first channel.\n").arg(sourceForCvt.channels());
            std::vector<cv::Mat> channels;
            cv::split(sourceForCvt, channels);
            analysisMat = channels[0];
         }
    } else if (inputImage.channels() == 1) {
        log += tr("Input is single-channel.\n");
        analysisMat = inputImage.clone();
    } else {
         log += tr("Error: Input image has 0 channels.\n");
        return cv::Mat(); // Return empty Mat on error
    }

    // Make sure analysisMat is not empty before proceeding
    if (analysisMat.empty()) {
         log += tr("Error: Failed to obtain single channel image.\n");
         return cv::Mat();
    }

    // 2. Convert to 8-bit unsigned integer (CV_8U)
    cv::Mat outputMat;
    if (analysisMat.depth() != CV_8U) {
        log += tr("Normalizing single-channel image (type: %1) to 8-bit for GLCM.\n")
                   .arg(cv::typeToString(analysisMat.type()));
        double minVal, maxVal;
        cv::minMaxLoc(analysisMat, &minVal, &maxVal);
        if (maxVal > minVal) {
            // Use CV_64F for intermediate precision during normalization
            cv::Mat tempMat;
            analysisMat.convertTo(tempMat, CV_64F);
            tempMat = (tempMat - minVal) * (255.0 / (maxVal - minVal));
            tempMat.convertTo(outputMat, CV_8U);
            // analysisMat.convertTo(outputMat, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
        } else {
            analysisMat.convertTo(outputMat, CV_8U); // Constant value image
            log += tr("Image has constant value.\n");
        }
    } else {
         log += tr("Image is already 8-bit single-channel.\n");
        outputMat = analysisMat; // Already CV_8U, just assign
    }

    return outputMat;
}

// Helper function to compute GLCM
// Implementation moved from mainwindow.cpp
void MainWindow::computeGLCM(const cv::Mat& img, cv::Mat& glcm, int dx, int dy, int levels, bool symmetric /* = true */, bool normalize /* = true */) {
    CV_Assert(img.type() == CV_8UC1);
    CV_Assert(levels > 0 && levels <= 256);

    glcm = cv::Mat::zeros(levels, levels, CV_64F); // Use double for accumulation
    double totalPairs = 0;

    for (int y = 0; y < img.rows; ++y) {
        for (int x = 0; x < img.cols; ++x) {
            int nx = x + dx;
            int ny = y + dy;

            // Check if neighbor is inside image bounds
            if (nx >= 0 && nx < img.cols && ny >= 0 && ny < img.rows) {
                uchar i = img.at<uchar>(y, x);
                uchar j = img.at<uchar>(ny, nx);

                // Ensure indices are within bounds (though they should be for CV_8U and levels=256)
                if (i < levels && j < levels) {
                     glcm.at<double>(i, j)++;
                     totalPairs++; // Count this pair

                     if (symmetric) {
                         // Also count the symmetric pair (j, i)
                         // Note: This assumes the definition of symmetric GLCM means adding the transpose
                         // Some definitions might handle it differently during normalization.
                         glcm.at<double>(j, i)++;
                         totalPairs++; // Count symmetric pair as well for normalization
                     }
                } else {
                    // This case should ideally not happen for CV_8U image and levels=256
                    qWarning("GLCM: Pixel value %d or %d out of bounds (0-%d)", i, j, levels-1);
                }
            }
            // Consider boundary conditions for the symmetric case if dx or dy are non-zero
            // If symmetric=true, we also need to check the pixel at (x-dx, y-dy)
            // But the current loop structure handles this by eventually visiting (nx, ny)
            // and checking back to (x, y). Let's keep the symmetric implementation inside
            // the primary check for simplicity, effectively double-counting pairs.

        }
    }

     // Normalization based on the total number of pairs counted.
     // If symmetric, totalPairs reflects counting both (i,j) and (j,i).
     // If not symmetric, it only counts (i,j) where the neighbor (nx,ny) is valid.
    if (normalize && totalPairs > 0) {
        glcm /= totalPairs; // Normalize to get probabilities P_ij
    } else if (normalize && totalPairs == 0) {
        // Handle case of empty image or no valid pairs, avoid division by zero
         glcm = cv::Mat::zeros(levels, levels, CV_64F);
         qWarning("GLCM: No valid pixel pairs found for the given offset. GLCM is zero.");
    }
    // If not normalized, glcm contains the counts.
}


// Helper function to calculate GLCM features
// Implementation moved from mainwindow.cpp
void MainWindow::calculateGLCMFeatures(const cv::Mat& glcm, int levels,
                           double& contrast, double& energy, double& homogeneity, double& correlation)
{
    CV_Assert(glcm.type() == CV_64F);
    CV_Assert(glcm.rows == levels && glcm.cols == levels);

    contrast = 0.0;
    energy = 0.0; // Also known as Angular Second Moment (ASM)
    homogeneity = 0.0; // Also known as Inverse Difference Moment (IDM)
    correlation = 0.0;

    double mean_i = 0.0, mean_j = 0.0;
    double stddev_i = 0.0, stddev_j = 0.0;
    double glcmSum = cv::sum(glcm)[0]; // Check sum for normalization (should be ~1.0 if normalized)

    // Calculate marginal probabilities (px, py) and means (mean_i, mean_j) first
    std::vector<double> px(levels, 0.0); // P_x(i) = Sum_j P(i,j)
    std::vector<double> py(levels, 0.0); // P_y(j) = Sum_i P(i,j)

    for (int i = 0; i < levels; ++i) {
        for (int j = 0; j < levels; ++j) {
            double p_ij = glcm.at<double>(i, j);
            px[i] += p_ij;
            py[j] += p_ij; // Note: if GLCM is symmetric, px should equal py

            // Features that can be calculated directly in this loop
            contrast += (i - j) * (i - j) * p_ij;
            energy += p_ij * p_ij;
            // IDM uses abs(i-j)
            homogeneity += p_ij / (1.0 + std::abs(i - j)); // Avoid division by zero when i=j
        }
        mean_i += i * px[i];
        mean_j += i * py[i]; // Use 'i' as index for py sum as well, since index maps to level
    }

    // If GLCM was not normalized, means might need adjustment
    if (std::abs(glcmSum - 1.0) > 1e-6 && glcmSum > 1e-9) {
         //qWarning("GLCM sum is %f, adjusting means/stddevs.", glcmSum);
         mean_i /= glcmSum;
         mean_j /= glcmSum;
         // Re-normalize px and py if needed for stddev calculation
         for(int i=0; i<levels; ++i) {
            px[i] /= glcmSum;
            py[i] /= glcmSum;
         }
    }


    // Calculate standard deviations (stddev_i, stddev_j) using the potentially normalized px, py
    for (int i = 0; i < levels; ++i) {
        stddev_i += (i - mean_i) * (i - mean_i) * px[i];
        stddev_j += (i - mean_j) * (i - mean_j) * py[i];
    }
    // Convert variance sum to standard deviation
    stddev_i = std::sqrt(stddev_i);
    stddev_j = std::sqrt(stddev_j);

    // Calculate correlation
    // Correlation = Sum_i Sum_j [ (i - mean_i)(j - mean_j) * P(i,j) ] / (stddev_i * stddev_j)
    if (stddev_i > 1e-9 && stddev_j > 1e-9) { // Avoid division by zero/instability
         for (int i = 0; i < levels; ++i) {
            for (int j = 0; j < levels; ++j) {
                double p_ij = glcm.at<double>(i, j);
                // If GLCM wasn't normalized, use p_ij / glcmSum here?
                // Assuming glcm *is* normalized or px/py were adjusted
                 if (std::abs(glcmSum - 1.0) > 1e-6 && glcmSum > 1e-9) {
                    p_ij /= glcmSum; // Use normalized p_ij for correlation formula
                 }
                 correlation += (i - mean_i) * (j - mean_j) * p_ij;
            }
        }
        correlation /= (stddev_i * stddev_j);
    } else {
        correlation = std::numeric_limits<double>::quiet_NaN(); // Indicate undefined if stddev is zero
        qWarning("GLCM Correlation is undefined because standard deviation is zero.");
    }

    // Note: Some definitions might differ slightly, e.g., in normalization constants or handling of stddev=0.
    // Verify against a known standard if specific values are critical.
}
