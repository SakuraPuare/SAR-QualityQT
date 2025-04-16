#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/imgproc.hpp> // cvtColor, magnitude, split
#include <opencv2/core.hpp>    // minMaxLoc, Mat, Scalar, CV_Assert, sum
#include <vector>
#include <cmath> // std::abs, std::sqrt, log2f
#include <limits> // std::numeric_limits

// Implement GLCM (Gray-Level Co-occurrence Matrix) analysis
void MainWindow::performGLCMAnalysis(const cv::Mat& inputImage) {
  QString resultLog = tr("GLCM Analysis Results:\n");
  QString overviewResult = tr("GLCM: ");

  if (inputImage.empty()) {
    resultLog = tr("Error: No valid image data provided for GLCM analysis.");
    ui->method5ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Data"));
    logMessage(resultLog);
    return;
  }

  // 1. Prepare 8-bit single-channel image required for GLCM calculation
  QString prepareLog;
  logMessage(tr("Preparing 8-bit grayscale image for GLCM analysis..."));
  cv::Mat glcmInputMat = prepareImageForGLCM(inputImage, prepareLog);
  resultLog += prepareLog;

  if (glcmInputMat.empty() || glcmInputMat.type() != CV_8UC1) {
      resultLog += tr("\nError: Failed to prepare 8-bit single channel image required for GLCM.");
      ui->method5ResultsTextEdit->setText(resultLog);
      ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
      logMessage(resultLog);
      return;
  }

  // 2. Define GLCM calculation parameters
  int levels = 256; // Number of gray levels, typically 256 for CV_8U images
  // Define the offset (dx, dy) for pixel pairs used in co-occurrence matrix calculation
  // (1, 0) represents horizontal direction, distance of 1 pixel
  int dx = 1;
  int dy = 0;
  // Note: Can calculate GLCM for multiple directions/distances and average the features for more robust texture description. Here, only one direction is used.
  resultLog += tr("\nCalculating GLCM with offset (dx=%1, dy=%2), levels=%3, symmetric=true, normalized=true\n")
                   .arg(dx).arg(dy).arg(levels);
  logMessage(tr("Calculating GLCM matrix with offset dx=%1, dy=%2...").arg(dx).arg(dy));


  try {
        // 3. Calculate the GLCM matrix
        cv::Mat glcm;
        // symmetric=true: Considers (j,i) when calculating for (i,j), making GLCM symmetric
        // normalize=true: Normalizes the GLCM so its elements represent probabilities P(i,j)
        computeGLCM(glcmInputMat, glcm, dx, dy, levels, true, true);

        // 4. Calculate texture features from the GLCM
        double contrast = 0.0, correlation = 0.0, energy = 0.0, homogeneity = 0.0;
        calculateGLCMFeatures(glcm, levels, contrast, energy, homogeneity, correlation);

        resultLog += tr("\n--- Texture Features (Offset dx=%1, dy=%2) ---\n").arg(dx).arg(dy);
        resultLog += tr("Contrast: %1\n").arg(contrast);         // Measure of local variations, higher value means deeper texture/grooves
        resultLog += tr("Correlation: %1\n").arg(correlation);   // Linear dependency of grayscale values, higher value means stronger linear correlation
        resultLog += tr("Energy (ASM): %1\n").arg(energy);       // Uniformity of grayscale distribution and texture coarseness, higher value means more regular
        resultLog += tr("Homogeneity (IDM): %1\n").arg(homogeneity); // Local homogeneity of the image, higher value means more locally uniform

        // Format overview string
        overviewResult += tr("Contr=%.3f, Corr=%.3f, Ener=%.3f, Homo=%.3f")
                             .arg(contrast).arg(correlation).arg(energy).arg(homogeneity);

        logMessage(tr("GLCM features calculated: Contrast=%1, Correlation=%2, Energy=%3, Homogeneity=%4")
                    .arg(contrast).arg(correlation).arg(energy).arg(homogeneity));

  } catch (const cv::Exception& e) {
        resultLog += tr("\nError during GLCM calculation or feature extraction: %1")
                       .arg(QString::fromStdString(e.what()));
        overviewResult += tr("Error - Calculation Failed");
        logMessage(tr("OpenCV Error during GLCM analysis: %1")
                     .arg(QString::fromStdString(e.what())));
  }

  // 5. Update UI
  ui->method5ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}

/**
 * @brief Prepares the input image into a format suitable for GLCM calculation (8-bit single-channel grayscale).
 *        Handles complex, multi-channel color, and single-channel images.
 * @param inputImage The input OpenCV Mat object.
 * @param log Reference to a QString for logging processing steps.
 * @return The prepared CV_8UC1 image, or an empty Mat on failure.
 */
cv::Mat MainWindow::prepareImageForGLCM(const cv::Mat& inputImage, QString& log)
{
    cv::Mat analysisMat; // To store the intermediate single-channel image
    log = ""; // Clear log

    // 1. Obtain a single-channel image
    if (inputImage.channels() == 2) {
        // Handle complex image: calculate magnitude
        log += tr("Input is complex (2-channel), calculating magnitude.\n");
        std::vector<cv::Mat> channels;
        cv::split(inputImage, channels);
        // Ensure channels are float type for magnitude calculation
        if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
            log += tr("Converting complex channels to CV_32F for magnitude calculation.\n");
            channels[0].convertTo(channels[0], CV_32F);
            channels[1].convertTo(channels[1], CV_32F);
        }
        cv::magnitude(channels[0], channels[1], analysisMat); // Magnitude image is single-channel float
    } else if (inputImage.channels() > 1) {
         // Handle multi-channel image (e.g., color): convert to grayscale
         log += tr("Input is multi-channel (%1), converting to grayscale.\n").arg(inputImage.channels());
         cv::Mat sourceForCvt = inputImage;
         // Ensure input type is suitable for cvtColor (typically 8U, 16U, 32F)
         if (inputImage.depth() != CV_8U && inputImage.depth() != CV_16U && inputImage.depth() != CV_32F) {
             log += tr("Input depth (%1) not directly supported by cvtColor, converting to CV_8U first.\n").arg(cv::typeToString(inputImage.type()));
             // Simple normalization to CV_8U
              double minVal, maxVal;
              cv::minMaxLoc(inputImage, &minVal, &maxVal);
              // Maintain original channels during conversion before cvtColor
              int outputType = CV_MAKETYPE(CV_8U, inputImage.channels());
              if (maxVal > minVal)
                inputImage.convertTo(sourceForCvt, outputType, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal));
              else
                inputImage.convertTo(sourceForCvt, outputType);
         }

         // Try standard BGR/BGRA to grayscale conversion
         if (sourceForCvt.channels() >= 3) {
              try {
                int code = (sourceForCvt.channels() == 4) ? cv::COLOR_BGRA2GRAY : cv::COLOR_BGR2GRAY;
                cv::cvtColor(sourceForCvt, analysisMat, code);
              } catch (const cv::Exception& e) {
                 log += tr("Standard grayscale conversion failed (%1), likely not BGR format. Falling back to first channel.\n").arg(QString::fromStdString(e.what()));
                 std::vector<cv::Mat> channels;
                 cv::split(sourceForCvt, channels);
                 analysisMat = channels[0].clone(); // Use clone to ensure data independence
              }
         } else {
             // If not 3/4 channels, cannot assume color space, use the first channel
            log += tr("Input has %1 channels, not standard BGR/BGRA. Using first channel directly.\n").arg(sourceForCvt.channels());
            std::vector<cv::Mat> channels;
            cv::split(sourceForCvt, channels);
            analysisMat = channels[0].clone();
         }
    } else if (inputImage.channels() == 1) {
        // Input is already single-channel
        log += tr("Input is already single-channel.\n");
        analysisMat = inputImage.clone(); // Clone to prevent modifying the original image
    } else {
         log += tr("Error: Input image has 0 channels.\n");
        return cv::Mat(); // Return empty Mat to indicate error
    }

    // Check if obtaining a single-channel image was successful
    if (analysisMat.empty()) {
         log += tr("Error: Failed to obtain a single channel image from the input.\n");
         return cv::Mat();
    }

    // 2. Convert the single-channel image to 8-bit unsigned integer (CV_8U)
    // This is the standard requirement for GLCM calculation, typically based on 0-255 gray levels
    cv::Mat outputMat;
    if (analysisMat.depth() != CV_8U) {
        log += tr("Normalizing single-channel image (type: %1) to 8-bit (0-255) for GLCM.\n")
                   .arg(cv::typeToString(analysisMat.type()));
        double minVal, maxVal;
        cv::minMaxLoc(analysisMat, &minVal, &maxVal);
        if (maxVal > minVal) {
            // Use convertTo for normalization and type conversion
            analysisMat.convertTo(outputMat, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
        } else {
            // Image has constant value
            analysisMat.convertTo(outputMat, CV_8U);
            log += tr("Image has constant value after conversion to single channel.\n");
        }
    } else {
         log += tr("Single-channel image is already 8-bit (CV_8U).\n");
        outputMat = analysisMat; // Assign directly as it's already CV_8U
    }

    // Final check if output is valid
    if (outputMat.empty() || outputMat.type() != CV_8UC1) {
        log += tr("Error: Failed to produce a valid CV_8UC1 image for GLCM.\n");
        return cv::Mat();
    }

    log += tr("Successfully prepared CV_8UC1 image for GLCM analysis.\n");
    return outputMat;
}

/**
 * @brief Calculates the Gray-Level Co-occurrence Matrix (GLCM) for a given image.
 * @param img Input image, must be of type CV_8UC1.
 * @param glcm Output GLCM matrix, type CV_64F (using double to accumulate avoids overflow).
 * @param dx Horizontal offset for pixel pairs.
 * @param dy Vertical offset for pixel pairs.
 * @param levels Dimension of the GLCM, equal to the number of gray levels (typically 256).
 * @param symmetric If true, calculates a symmetric GLCM, considering both (i,j) and (j,i) pairs.
 * @param normalize If true, normalizes the GLCM into a probability matrix (sum of elements is 1).
 */
void MainWindow::computeGLCM(const cv::Mat& img, cv::Mat& glcm, int dx, int dy, int levels, bool symmetric /* = true */, bool normalize /* = true */) {
    CV_Assert(img.type() == CV_8UC1 && "Input image must be 8-bit single-channel (CV_8UC1)");
    CV_Assert(levels > 0 && levels <= 256 && "Levels must be between 1 and 256");

    // Initialize GLCM matrix with zeros, use CV_64F to store counts or probabilities
    glcm = cv::Mat::zeros(levels, levels, CV_64F);
    double totalPairs = 0; // Total number of pixel pairs for normalization

    // Iterate over image pixels (excluding edges, based on offset dx, dy)
    for (int y = 0; y < img.rows; ++y) {
        for (int x = 0; x < img.cols; ++x) {
            // Calculate coordinates of the neighbor pixel
            int nx = x + dx;
            int ny = y + dy;

            // Check if the neighbor pixel is within image boundaries
            if (nx >= 0 && nx < img.cols && ny >= 0 && ny < img.rows) {
                // Get grayscale values of the center pixel (i) and neighbor pixel (j)
                uchar i = img.at<uchar>(y, x);
                uchar j = img.at<uchar>(ny, nx);

                // Ensure grayscale values are within [0, levels-1] range (should always hold for CV_8U and levels=256)
                if (i < levels && j < levels) {
                     // Increment count at the corresponding GLCM position
                     glcm.at<double>(i, j)++;
                     totalPairs++; // Count valid pixel pair

                     // If calculating symmetric GLCM, also increment count at (j, i) position
                     if (symmetric) {
                         // Note: Simply incrementing at (j, i) as well.
                         // This means pairs (i, j) and (j, i) are both counted.
                         // The totalPairs count is also increased accordingly for normalization.
                         // If i != j, this pixel pair contributes two counts. If i == j, it contributes one.
                         // Some definitions might differ, e.g., compute once then add transpose, or use a different normalization denominator.
                         glcm.at<double>(j, i)++;
                         if (i != j) { // Only if i!=j does the symmetric operation introduce a new count point
                            totalPairs++; // Also consider this added count during normalization
                         }
                     }
                }
                // else { // Should theoretically not happen
                //     qWarning("GLCM: Pixel value %d or %d out of expected range [0, %d)", i, j, levels);
                // }
            }
        }
    }

     // Perform normalization if requested
    if (normalize) {
        if (totalPairs > 0) {
            glcm /= totalPairs; // Convert counts to probabilities P(i,j)
        } else {
            // Handle case with no valid pixel pairs (e.g., image too small or offset too large)
            glcm = cv::Mat::zeros(levels, levels, CV_64F); // Keep as zero matrix
            logMessage(tr("Warning: No valid pixel pairs found for GLCM calculation with the given offset. GLCM is zero."));
            // qWarning("GLCM: No valid pixel pairs found for the given offset (dx=%d, dy=%d). GLCM is zero.", dx, dy);
        }
    }
    // If normalize is false, glcm will contain the raw counts.
}


/**
 * @brief Calculates common texture features from a normalized or unnormalized GLCM.
 * @param glcm Input GLCM matrix (CV_64F).
 * @param levels Dimension of the GLCM (number of gray levels).
 * @param[out] contrast Contrast: Measures local variations in the image, higher value means deeper texture.
 * @param[out] energy Energy (Angular Second Moment, ASM): Measures uniformity of grayscale distribution and texture coarseness, higher value means more regular.
 * @param[out] homogeneity Homogeneity (Inverse Difference Moment, IDM): Measures local grayscale uniformity, higher value means more locally uniform.
 * @param[out] correlation Correlation: Measures the linear dependency of grayscale values, higher value means stronger linear correlation.
 */
void MainWindow::calculateGLCMFeatures(const cv::Mat& glcm, int levels,
                           double& contrast, double& energy, double& homogeneity, double& correlation)
{
    CV_Assert(glcm.type() == CV_64F && "GLCM must be of type CV_64F");
    CV_Assert(glcm.rows == levels && glcm.cols == levels && "GLCM dimensions must match levels");

    // Initialize feature values to 0
    contrast = 0.0;
    energy = 0.0;    // Angular Second Moment (ASM)
    homogeneity = 0.0; // Inverse Difference Moment (IDM)
    correlation = 0.0;

    double mean_i = 0.0, mean_j = 0.0;   // Mean in row and column directions
    double stddev_i = 0.0, stddev_j = 0.0; // Standard deviation in row and column directions
    double glcmSum = cv::sum(glcm)[0];   // Calculate sum of GLCM, used for checking normalization and subsequent calculations

    // Define marginal probabilities Px(i) and Py(j)
    std::vector<double> px(levels, 0.0); // Px(i) = Sum_j P(i,j)
    std::vector<double> py(levels, 0.0); // Py(j) = Sum_i P(i,j)

    // --- First pass through GLCM: Calculate Px, Py, mean_i, mean_j, contrast, energy, homogeneity ---
    for (int i = 0; i < levels; ++i) {
        for (int j = 0; j < levels; ++j) {
            double p_ij = glcm.at<double>(i, j); // Get GLCM element value P(i,j)

            // Accumulate marginal probabilities
            px[i] += p_ij;
            py[j] += p_ij; // If GLCM is symmetric, px should equal py

            // Calculate features that can be summed directly in this loop
            // Contrast: Sum_i Sum_j (i-j)^2 * P(i,j)
            contrast += (i - j) * (i - j) * p_ij;
            // Energy (ASM): Sum_i Sum_j P(i,j)^2
            energy += p_ij * p_ij;
            // Homogeneity (IDM): Sum_i Sum_j P(i,j) / (1 + |i-j|)
            homogeneity += p_ij / (1.0 + std::abs(i - j)); // Add 1 to denominator to avoid division by zero when i=j
        }
    }

    // Calculate means mean_i and mean_j
    // mean_i = Sum_i i * Px(i)
    // mean_j = Sum_j j * Py(j)
    for (int i = 0; i < levels; ++i) {
        mean_i += i * px[i];
        mean_j += i * py[i]; // Note: use py[i] here, not py[j], as py index also represents gray level
    }

    // If GLCM was not normalized (sum is not close to 1), adjust means and marginal probabilities
    // for subsequent standard deviation and correlation calculations
    bool needsNormalizationAdjustment = std::abs(glcmSum - 1.0) > 1e-6 && glcmSum > 1e-9;
    if (needsNormalizationAdjustment) {
        logMessage(tr("GLCM sum (%1) is not close to 1. Adjusting means and standard deviations based on sum.").arg(glcmSum));
        // qWarning("GLCM sum is %f, features might be scaled if GLCM was not normalized.", glcmSum);
        mean_i /= glcmSum;
        mean_j /= glcmSum;
        // Normalize Px and Py to correctly calculate standard deviations
        for(int i=0; i<levels; ++i) {
            px[i] /= glcmSum;
            py[i] /= glcmSum;
        }
    }

    // --- Second pass (or based on Px, Py): Calculate standard deviations stddev_i, stddev_j ---
    // stddev_i = sqrt( Sum_i (i - mean_i)^2 * Px(i) )
    // stddev_j = sqrt( Sum_j (j - mean_j)^2 * Py(j) )
    for (int i = 0; i < levels; ++i) {
        stddev_i += (i - mean_i) * (i - mean_i) * px[i];
        stddev_j += (i - mean_j) * (i - mean_j) * py[i]; // Also use py[i]
    }
    // Take square root to get standard deviation
    stddev_i = std::sqrt(stddev_i);
    stddev_j = std::sqrt(stddev_j);

    // --- Third pass through GLCM: Calculate Correlation ---
    // Correlation = Sum_i Sum_j [ (i - mean_i) * (j - mean_j) * P(i,j) ] / (stddev_i * stddev_j)
    if (stddev_i > 1e-9 && stddev_j > 1e-9) { // Avoid division by zero or very small stddev causing numerical instability
         for (int i = 0; i < levels; ++i) {
            for (int j = 0; j < levels; ++j) {
                double p_ij = glcm.at<double>(i, j);
                // If GLCM was not normalized, use the normalized p_ij for calculation
                 if (needsNormalizationAdjustment) {
                    p_ij /= glcmSum;
                 }
                 correlation += (i - mean_i) * (j - mean_j) * p_ij;
            }
        }
        correlation /= (stddev_i * stddev_j); // Divide by product of standard deviations
    } else {
        // If either standard deviation is near zero (e.g., image is constant gray), correlation is undefined
        correlation = std::numeric_limits<double>::quiet_NaN(); // Use NaN to represent undefined
        logMessage(tr("Warning: GLCM Correlation is undefined because standard deviation of i or j is near zero."));
        // qWarning("GLCM Correlation is undefined because standard deviation is (near) zero (std_i=%.3e, std_j=%.3e).", stddev_i, stddev_j);
    }

    // Note: Precise definitions of these features might slightly vary in different literature or libraries,
    // especially regarding handling unnormalized GLCMs or edge cases.
}
