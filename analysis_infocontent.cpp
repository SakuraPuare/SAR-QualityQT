#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/imgproc.hpp> // calcHist, cvtColor, magnitude, split
#include <vector>
#include <cmath> // log2f

// Implement Information Content (Entropy) analysis
void MainWindow::performInfoContentAnalysis(const cv::Mat& inputImage) { // Accept inputImage parameter
  QString resultLog; // For detailed results text box
  QString overviewResult = tr("Information Content (Entropy): "); // For overview

  if (inputImage.empty()) { // Use inputImage parameter
    resultLog = tr("Error: No valid image data provided for entropy analysis."); // Updated error message
    ui->method2ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Data")); // Updated overview error
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat; // Single-channel image for entropy calculation

  // 1. Prepare single-channel image (magnitude or grayscale)
  logMessage(tr("Preparing single-channel image for entropy analysis..."));
  if (inputImage.channels() == 2) { // Use inputImage parameter
    // Handle complex data: calculate magnitude
    logMessage(tr("Input is complex (2-channel), calculating magnitude."));
    std::vector<cv::Mat> channels;
    cv::split(inputImage, channels); // Use inputImage parameter
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        logMessage(tr("Converting complex channels to CV_32F for magnitude."));
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat); // Magnitude image is single-channel float
    resultLog = tr("Calculated magnitude from complex data.\n");
  } else if (inputImage.channels() > 1) { // Use inputImage parameter
    // Handle multi-channel image: convert to grayscale
    logMessage(tr("Input is multi-channel (%1), converting to grayscale.").arg(inputImage.channels())); // Use inputImage parameter
     cv::Mat sourceForCvt = inputImage; // Use inputImage parameter
     if (inputImage.depth() != CV_8U && inputImage.depth() != CV_16U && inputImage.depth() != CV_32F) { // Use inputImage parameter
         logMessage(tr("Input depth not directly supported by cvtColor, converting to CV_8U first."));
         double minVal, maxVal;
         cv::minMaxLoc(inputImage, &minVal, &maxVal); // Use inputImage parameter
         int outputType = CV_MAKETYPE(CV_8U, inputImage.channels()); // Use inputImage parameter
         if (maxVal > minVal)
             inputImage.convertTo(sourceForCvt, outputType, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal)); // Use inputImage parameter
         else
             inputImage.convertTo(sourceForCvt, outputType); // Use inputImage parameter
     }

     if (sourceForCvt.channels() >= 3) {
          try {
            int code = (sourceForCvt.channels() == 4) ? cv::COLOR_BGRA2GRAY : cv::COLOR_BGR2GRAY;
            cv::cvtColor(sourceForCvt, analysisMat, code);
             resultLog = tr("Converted multi-channel image to grayscale using standard conversion.\n");
          } catch (const cv::Exception& e) {
             logMessage(tr("Standard grayscale conversion failed (%1), falling back to first channel.").arg(QString::fromStdString(e.what())));
             std::vector<cv::Mat> channels;
             cv::split(sourceForCvt, channels);
             analysisMat = channels[0].clone();
             resultLog = tr("Converted multi-channel image to single channel using the first channel.\n");
          }
     } else {
        logMessage(tr("Input has %1 channels, not standard BGR/BGRA. Using first channel.").arg(sourceForCvt.channels()));
        std::vector<cv::Mat> channels;
        cv::split(sourceForCvt, channels);
        analysisMat = channels[0].clone();
        resultLog = tr("Used the first channel of the multi-channel image.\n");
     }
  } else if (inputImage.channels() == 1){ // Use inputImage parameter
    // Already single-channel image
    analysisMat = inputImage.clone(); // Clone to prevent modification
    resultLog = tr("Input is already single-channel.\n");
  } else {
     logMessage(tr("Error: Input image has 0 channels, cannot calculate entropy."));
     resultLog = tr("Error: Input image has 0 channels.");
     ui->method2ResultsTextEdit->setText(resultLog);
     ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Invalid Channels"));
     return;
  }

  // --- Check if a single-channel image was successfully obtained ---
  if (analysisMat.empty() || analysisMat.channels() != 1) {
      logMessage(tr("Error: Could not obtain a valid single-channel image for entropy calculation."));
       resultLog += tr("\nError: Failed to get single-channel data for analysis.");
       ui->method2ResultsTextEdit->setText(resultLog);
       ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
       return;
  }


  // 2. Convert the single-channel image to CV_8U for histogram calculation
  // Entropy calculation is typically based on discrete grayscale level probabilities, and 8-bit (0-255) is a common standard.
  cv::Mat histMat; // 8-bit image for histogram calculation
  logMessage(tr("Converting single-channel image to 8-bit (CV_8U) for histogram calculation..."));
  if (analysisMat.depth() != CV_8U) {
    resultLog += tr("Normalizing data to 8-bit range (0-255) for histogram.\n");
    double minVal, maxVal;
    cv::minMaxLoc(analysisMat, &minVal, &maxVal);
    if (maxVal > minVal) {
      // Normalize and convert type
      analysisMat.convertTo(histMat, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    } else {
      // Image has constant value
      analysisMat.convertTo(histMat, CV_8U);
      resultLog += tr("Image has constant value; entropy is expected to be 0.\n");
    }
  } else {
    histMat = analysisMat; // Already CV_8U
    resultLog += tr("Image is already 8-bit (CV_8U), using directly for histogram.\n");
  }

  // Check if the converted histMat is valid
   if (histMat.empty() || histMat.type() != CV_8UC1) {
      logMessage(tr("Error: Failed to produce a valid CV_8UC1 image for histogram calculation."));
       resultLog += tr("\nError: Failed to get 8-bit data for histogram.");
       ui->method2ResultsTextEdit->setText(resultLog);
       ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Conversion Failed"));
       return;
   }

  // 3. Calculate the grayscale histogram
  cv::Mat hist;           // Output histogram (CV_32F type)
  int histSize = 256;       // Number of histogram bins (corresponding to 0-255 gray levels)
  float range[] = {0, 256}; // Grayscale value range [0, 256)
  const float *histRange[] = {range}; // Pointer to the range array
  bool uniform = true;      // Are the bin sizes uniform?
  bool accumulate = false;  // Accumulate to existing histogram? (No)
  float entropy = 0.0f;     // Initialize entropy to 0

  try {
    logMessage(tr("Calculating histogram..."));
    cv::calcHist(&histMat,    // Input image array (only one)
                 1,           // Number of images
                 0,           // Channel index to compute histogram for (channel 0)
                 cv::Mat(),   // Mask (none used)
                 hist,        // Output histogram Mat
                 1,           // Histogram dimensionality (1D for grayscale)
                 &histSize,   // Number of bins per dimension
                 histRange,   // Value range per dimension
                 uniform, accumulate);

    // 4. Normalize the histogram to get the probability p_i for each gray level
    // total() returns the total number of elements in the Mat (total pixels)
    double totalPixels = histMat.total();
    if (totalPixels > 0) {
        hist /= totalPixels; // hist now contains the probability for each bin
    } else {
         logMessage(tr("Warning: Image has zero pixels. Cannot calculate entropy."));
         resultLog += tr("\nWarning: Image contains no pixels.");
         ui->method2ResultsTextEdit->setText(resultLog);
         ui->overviewResultsTextEdit->append(overviewResult + tr("Warning - Empty Image"));
         return;
    }


    // 5. Calculate Shannon Entropy
    // H = - sum( p_i * log2(p_i) ) for all p_i > 0
    // Entropy measures the uncertainty or average information content of the image's grayscale distribution, in bits/pixel
    logMessage(tr("Calculating entropy from normalized histogram..."));
    for (int i = 0; i < histSize; i++) {
      float p = hist.at<float>(i); // Get probability for gray level i
      if (p > std::numeric_limits<float>::epsilon()) { // Process only probabilities > 0 to avoid log2(0)
        entropy -= p * log2f(p); // Use log2f for base-2 logarithm
      }
    }

    resultLog += tr("\n--- Entropy Calculation Result ---\n");
    resultLog += tr("Shannon Entropy: %1 bits/pixel").arg(entropy);
    resultLog += tr("\nInterpretation: Higher entropy generally indicates more complex texture or a wider range of gray levels used in the image.");

    overviewResult += tr("%.4f bits/pixel").arg(entropy); // Overview result with 4 decimal places
    logMessage(tr("Entropy calculation successful: %1 bits/pixel").arg(entropy));

  } catch (const cv::Exception &e) {
    resultLog += tr("\nError during histogram or entropy calculation: %1")
                     .arg(QString::fromStdString(e.what()));
    overviewResult += tr("Error - Calculation Failed");
    logMessage(tr("OpenCV Error during Entropy calculation: %1")
                   .arg(QString::fromStdString(e.what())));
     // Show log even if error occurred
     ui->method2ResultsTextEdit->setText(resultLog);
     ui->overviewResultsTextEdit->append(overviewResult);
     return; // Return early
  }

  // --- Update UI ---
  ui->method2ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}
