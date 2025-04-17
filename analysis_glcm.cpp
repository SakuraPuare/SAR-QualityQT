#include "analysis_utils.h" // 包含新的头文件
#include <opencv2/imgproc.hpp> // cvtColor, magnitude, split
#include <opencv2/core.hpp>    // minMaxLoc, Mat, Scalar, CV_Assert, sum
#include <vector>
#include <cmath> // std::abs, std::sqrt, log2f
#include <limits> // std::numeric_limits
#include <QString>
#include <QCoreApplication> // For translate()

// 独立的 GLCM 分析函数
AnalysisResult performGLCMAnalysis(const cv::Mat& inputImage) {
    AnalysisResult result;
    result.analysisName = QCoreApplication::translate("Analysis", "GLCM Texture");
    result.detailedLog = QCoreApplication::translate("Analysis", "GLCM Analysis Results:\n");
    QString overviewPrefix = QCoreApplication::translate("Analysis", "GLCM: ");

    if (inputImage.empty()) {
        result.detailedLog += QCoreApplication::translate("Analysis", "\nError: No valid image data provided.");
        result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "Error - No Data");
        result.success = false;
        return result;
    }

    // 1. 准备 8 位单通道图像
    QString prepareLog;
    result.detailedLog += QCoreApplication::translate("Analysis", "Preparing 8-bit grayscale image for GLCM...\n");
    cv::Mat glcmInputMat = prepareImageForGLCM(inputImage, prepareLog); // 调用独立函数
    result.detailedLog += prepareLog;

    if (glcmInputMat.empty() || glcmInputMat.type() != CV_8UC1) {
        result.detailedLog += QCoreApplication::translate("Analysis", "\nError: Failed to prepare 8-bit single channel image required for GLCM.");
        result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "Error - Prep Failed");
        result.success = false;
        return result;
    }

    // 2. 定义参数
    int levels = 256;
    int dx = 1;
    int dy = 0;
    result.detailedLog += QCoreApplication::translate("Analysis", "\nCalculating GLCM with offset (dx=%1, dy=%2), levels=%3, symmetric=true, normalized=true\n")
                           .arg(dx).arg(dy).arg(levels);
    QString calcLog; // 日志用于 compute 和 calculate

    try {
        // 3. 计算 GLCM
        cv::Mat glcm;
        computeGLCM(glcmInputMat, glcm, dx, dy, levels, true, true, calcLog); // 调用独立函数
        result.detailedLog += calcLog; // 添加计算日志

        // 4. 计算纹理特征
        double contrast = 0.0, correlation = 0.0, energy = 0.0, homogeneity = 0.0;
        QString featureLog; // 日志用于特征计算
        calculateGLCMFeatures(glcm, levels, contrast, energy, homogeneity, correlation, featureLog); // 调用独立函数
        result.detailedLog += featureLog; // 添加特征计算日志

        result.detailedLog += QCoreApplication::translate("Analysis", "\n--- Texture Features (Offset dx=%1, dy=%2) ---\n").arg(dx).arg(dy);
        result.detailedLog += QCoreApplication::translate("Analysis", "Contrast: %1\n").arg(contrast);
        result.detailedLog += QCoreApplication::translate("Analysis", "Correlation: %1\n").arg(correlation);
        result.detailedLog += QCoreApplication::translate("Analysis", "Energy (ASM): %1\n").arg(energy);
        result.detailedLog += QCoreApplication::translate("Analysis", "Homogeneity (IDM): %1\n").arg(homogeneity);

        // 格式化概览字符串
        result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "Contr=%.3f, Corr=%.3f, Ener=%.3f, Homo=%.3f")
                                             .arg(contrast).arg(correlation).arg(energy).arg(homogeneity);

        result.detailedLog += QCoreApplication::translate("Analysis", "Internal Log: GLCM features calculated: Contrast=%1, Correlation=%2, Energy=%3, Homogeneity=%4\n")
                            .arg(contrast).arg(correlation).arg(energy).arg(homogeneity);
        result.success = true;

    } catch (const cv::Exception& e) {
        result.detailedLog += QCoreApplication::translate("Analysis", "\nError during GLCM calculation or feature extraction: %1")
                               .arg(QString::fromStdString(e.what()));
        result.overviewSummary = overviewPrefix + QCoreApplication::translate("Analysis", "Error - Calculation Failed");
        result.detailedLog += QCoreApplication::translate("Analysis", "Internal Log: OpenCV Error during GLCM analysis: %1\n")
                             .arg(QString::fromStdString(e.what()));
        result.success = false;
    }

    return result;
}

// --- 独立的辅助函数 ---

// 准备用于 GLCM 的 8 位单通道图像
cv::Mat prepareImageForGLCM(const cv::Mat& inputImage, QString& log)
{
    cv::Mat analysisMat;
    log = ""; // Clear log initially for this function

    // 1. 获取单通道图像
    if (inputImage.channels() == 2) {
        log += QCoreApplication::translate("AnalysisHelper", "Input is complex (2-channel), calculating magnitude.\n");
        std::vector<cv::Mat> channels;
        cv::split(inputImage, channels);
        if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
            log += QCoreApplication::translate("AnalysisHelper", "Converting complex channels to CV_32F for magnitude calculation.\n");
            channels[0].convertTo(channels[0], CV_32F);
            channels[1].convertTo(channels[1], CV_32F);
        }
        cv::magnitude(channels[0], channels[1], analysisMat);
    } else if (inputImage.channels() > 1) {
         log += QCoreApplication::translate("AnalysisHelper", "Input is multi-channel (%1), converting to grayscale.\n").arg(inputImage.channels());
         cv::Mat sourceForCvt = inputImage;
         if (inputImage.depth() != CV_8U && inputImage.depth() != CV_16U && inputImage.depth() != CV_32F) {
             log += QCoreApplication::translate("AnalysisHelper", "Input depth (%1) not directly supported by cvtColor, converting to CV_8U first.\n").arg(cv::typeToString(inputImage.type()));
             double minVal, maxVal;
             cv::minMaxLoc(inputImage, &minVal, &maxVal);
             int outputType = CV_MAKETYPE(CV_8U, inputImage.channels());
             if (maxVal > minVal)
                inputImage.convertTo(sourceForCvt, outputType, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal));
             else
                inputImage.convertTo(sourceForCvt, outputType);
         }
         if (sourceForCvt.channels() >= 3) {
              try {
                int code = (sourceForCvt.channels() == 4) ? cv::COLOR_BGRA2GRAY : cv::COLOR_BGR2GRAY;
                cv::cvtColor(sourceForCvt, analysisMat, code);
              } catch (const cv::Exception& e) {
                 log += QCoreApplication::translate("AnalysisHelper", "Standard grayscale conversion failed (%1), likely not BGR format. Falling back to first channel.\n").arg(QString::fromStdString(e.what()));
                 std::vector<cv::Mat> channels;
                 cv::split(sourceForCvt, channels);
                 analysisMat = channels[0].clone();
              }
         } else {
            log += QCoreApplication::translate("AnalysisHelper", "Input has %1 channels, not standard BGR/BGRA. Using first channel directly.\n").arg(sourceForCvt.channels());
            std::vector<cv::Mat> channels;
            cv::split(sourceForCvt, channels);
            analysisMat = channels[0].clone();
         }
    } else if (inputImage.channels() == 1) {
        log += QCoreApplication::translate("AnalysisHelper", "Input is already single-channel.\n");
        analysisMat = inputImage.clone();
    } else {
         log += QCoreApplication::translate("AnalysisHelper", "Error: Input image has 0 channels.\n");
        return cv::Mat();
    }

    if (analysisMat.empty()) {
         log += QCoreApplication::translate("AnalysisHelper", "Error: Failed to obtain a single channel image from the input.\n");
         return cv::Mat();
    }

    // 2. 转换为 CV_8U
    cv::Mat outputMat;
    if (analysisMat.depth() != CV_8U) {
        log += QCoreApplication::translate("AnalysisHelper", "Normalizing single-channel image (type: %1) to 8-bit (0-255) for GLCM.\n")
                   .arg(cv::typeToString(analysisMat.type()));
        double minVal, maxVal;
        cv::minMaxLoc(analysisMat, &minVal, &maxVal);
        if (maxVal > minVal) {
            analysisMat.convertTo(outputMat, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
        } else {
            analysisMat.convertTo(outputMat, CV_8U);
            log += QCoreApplication::translate("AnalysisHelper", "Image has constant value after conversion to single channel.\n");
        }
    } else {
         log += QCoreApplication::translate("AnalysisHelper", "Single-channel image is already 8-bit (CV_8U).\n");
        outputMat = analysisMat;
    }

    if (outputMat.empty() || outputMat.type() != CV_8UC1) {
        log += QCoreApplication::translate("AnalysisHelper", "Error: Failed to produce a valid CV_8UC1 image for GLCM.\n");
        return cv::Mat();
    }

    log += QCoreApplication::translate("AnalysisHelper", "Successfully prepared CV_8UC1 image for GLCM analysis.\n");
    return outputMat;
}

// 计算 GLCM 矩阵
void computeGLCM(const cv::Mat& img, cv::Mat& glcm, int dx, int dy, int levels, bool symmetric, bool normalize, QString& log) {
    log = ""; // Clear log for this function
    CV_Assert(img.type() == CV_8UC1 && "Input image must be 8-bit single-channel (CV_8UC1)");
    CV_Assert(levels > 0 && levels <= 256 && "Levels must be between 1 and 256");

    glcm = cv::Mat::zeros(levels, levels, CV_64F);
    double totalPairs = 0;

    for (int y = 0; y < img.rows; ++y) {
        for (int x = 0; x < img.cols; ++x) {
            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < img.cols && ny >= 0 && ny < img.rows) {
                uchar i = img.at<uchar>(y, x);
                uchar j = img.at<uchar>(ny, nx);

                if (i < levels && j < levels) {
                     glcm.at<double>(i, j)++;
                     totalPairs++;
                     if (symmetric) {
                         glcm.at<double>(j, i)++;
                         if (i != j) {
                            totalPairs++;
                         }
                     }
                }
            }
        }
    }

    if (normalize) {
        if (totalPairs > 0) {
            glcm /= totalPairs;
            log += QCoreApplication::translate("AnalysisHelper", "GLCM calculation complete. Normalized by %1 pairs.\n").arg(totalPairs);
        } else {
            glcm = cv::Mat::zeros(levels, levels, CV_64F);
            log += QCoreApplication::translate("AnalysisHelper", "Warning: No valid pixel pairs found for GLCM calculation. GLCM is zero.\n");
        }
    } else {
         log += QCoreApplication::translate("AnalysisHelper", "GLCM calculation complete. Contains raw counts (%1 total pairs).\n").arg(totalPairs);
    }
}

// 计算 GLCM 特征
void calculateGLCMFeatures(const cv::Mat& glcm, int levels,
                           double& contrast, double& energy, double& homogeneity, double& correlation, QString& log)
{
    log = ""; // Clear log for this function
    CV_Assert(glcm.type() == CV_64F && "GLCM must be of type CV_64F");
    CV_Assert(glcm.rows == levels && glcm.cols == levels && "GLCM dimensions must match levels");

    contrast = 0.0;
    energy = 0.0;
    homogeneity = 0.0;
    correlation = 0.0;

    double mean_i = 0.0, mean_j = 0.0;
    double stddev_i = 0.0, stddev_j = 0.0;
    double glcmSum = cv::sum(glcm)[0];

    std::vector<double> px(levels, 0.0);
    std::vector<double> py(levels, 0.0);

    // Pass 1: Calculate marginal probabilities, means, and direct features
    for (int i = 0; i < levels; ++i) {
        for (int j = 0; j < levels; ++j) {
            double p_ij = glcm.at<double>(i, j);
            px[i] += p_ij;
            py[j] += p_ij;
            contrast += (i - j) * (i - j) * p_ij;
            energy += p_ij * p_ij;
            homogeneity += p_ij / (1.0 + std::abs(i - j));
        }
    }

    for (int i = 0; i < levels; ++i) {
        mean_i += i * px[i];
        mean_j += i * py[i];
    }

    bool needsNormalizationAdjustment = std::abs(glcmSum - 1.0) > 1e-6 && glcmSum > 1e-9;
    if (needsNormalizationAdjustment) {
        log += QCoreApplication::translate("AnalysisHelper", "Warning: GLCM sum (%1) is not close to 1. Adjusting means/stddev based on sum.\n").arg(glcmSum);
        mean_i /= glcmSum;
        mean_j /= glcmSum;
        for(int i=0; i<levels; ++i) {
            px[i] /= glcmSum;
            py[i] /= glcmSum;
        }
    } else {
        // If sum is close to 1 or 0, means are already correct relative to probabilities
    }


    // Pass 2: Calculate standard deviations
    for (int i = 0; i < levels; ++i) {
        stddev_i += (i - mean_i) * (i - mean_i) * px[i];
        stddev_j += (i - mean_j) * (i - mean_j) * py[i];
    }
    stddev_i = std::sqrt(stddev_i);
    stddev_j = std::sqrt(stddev_j);


    // Pass 3: Calculate Correlation
    if (stddev_i > 1e-9 && stddev_j > 1e-9) {
         for (int i = 0; i < levels; ++i) {
            for (int j = 0; j < levels; ++j) {
                double p_ij = glcm.at<double>(i, j);
                 if (needsNormalizationAdjustment) {
                    p_ij /= glcmSum; // Use normalized p_ij if adjustment was needed
                 }
                 correlation += (i - mean_i) * (j - mean_j) * p_ij;
            }
        }
        correlation /= (stddev_i * stddev_j);
    } else {
        correlation = std::numeric_limits<double>::quiet_NaN(); // Undefined
        log += QCoreApplication::translate("AnalysisHelper", "Warning: GLCM Correlation is undefined because stddev_i or stddev_j is near zero.\n");
    }
     log += QCoreApplication::translate("AnalysisHelper", "GLCM feature calculation complete.\n");
}
