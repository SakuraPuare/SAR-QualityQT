#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <opencv2/imgproc.hpp> // cvtColor, magnitude, split
#include <opencv2/core.hpp>    // minMaxLoc, Mat, Scalar, CV_Assert, sum
#include <vector>
#include <cmath> // std::abs, std::sqrt, log2f
#include <limits> // std::numeric_limits

// 实现 GLCM (灰度共生矩阵) 分析
void MainWindow::performGLCMAnalysis() {
  QString resultLog = tr("GLCM Analysis Results:\n");
  QString overviewResult = tr("GLCM: ");

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded for GLCM analysis.");
    ui->method5ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Image"));
    logMessage(resultLog);
    return;
  }

  // 1. 准备用于 GLCM 计算的 8 位单通道图像
  QString prepareLog;
  logMessage(tr("Preparing 8-bit grayscale image for GLCM analysis..."));
  cv::Mat glcmInputMat = prepareImageForGLCM(currentImage, prepareLog);
  resultLog += prepareLog;

  if (glcmInputMat.empty() || glcmInputMat.type() != CV_8UC1) {
      resultLog += tr("\nError: Failed to prepare 8-bit single channel image required for GLCM.");
      ui->method5ResultsTextEdit->setText(resultLog);
      ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Prep Failed"));
      logMessage(resultLog);
      return;
  }

  // 2. 定义 GLCM 计算参数
  int levels = 256; // 灰度级数，对于 CV_8U 图像通常是 256
  // 定义计算共生矩阵的像素对偏移量 (dx, dy)
  // (1, 0) 表示水平方向，距离为 1 像素
  int dx = 1;
  int dy = 0;
  // 注意：可以计算多个方向/距离的 GLCM 并对特征进行平均，以获得更鲁棒的纹理描述。这里仅使用一个方向。
  resultLog += tr("\nCalculating GLCM with offset (dx=%1, dy=%2), levels=%3, symmetric=true, normalized=true\n")
                   .arg(dx).arg(dy).arg(levels);
  logMessage(tr("Calculating GLCM matrix with offset dx=%1, dy=%2...").arg(dx).arg(dy));


  try {
        // 3. 计算 GLCM 矩阵
        cv::Mat glcm;
        // symmetric=true: 计算 (i,j) 时也考虑 (j,i)，使 GLCM 对称
        // normalize=true: 将 GLCM 归一化，使其元素表示概率 P(i,j)
        computeGLCM(glcmInputMat, glcm, dx, dy, levels, true, true);

        // 4. 从 GLCM 计算纹理特征
        double contrast = 0.0, correlation = 0.0, energy = 0.0, homogeneity = 0.0;
        calculateGLCMFeatures(glcm, levels, contrast, energy, homogeneity, correlation);

        resultLog += tr("\n--- Texture Features (Offset dx=%1, dy=%2) ---\n").arg(dx).arg(dy);
        resultLog += tr("Contrast: %1\n").arg(contrast);         // 局部变化量，值越大纹理越深，沟壑越深
        resultLog += tr("Correlation: %1\n").arg(correlation);   // 图像灰度值的线性相关性，值越大线性相关性越强
        resultLog += tr("Energy (ASM): %1\n").arg(energy);       // 图像灰度分布均匀程度和纹理粗细度，值越大越规则
        resultLog += tr("Homogeneity (IDM): %1\n").arg(homogeneity); // 图像局部灰度均匀性，值越大局部越均匀

        // 格式化概览字符串
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

  // 5. 更新 UI
  ui->method5ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}

/**
 * @brief 将输入图像准备成适合 GLCM 计算的格式 (8 位单通道灰度图)。
 *        处理复数、多通道彩色和单通道图像。
 * @param inputImage 输入的 OpenCV Mat 对象。
 * @param log 用于记录处理步骤的 QString 引用。
 * @return 准备好的 CV_8UC1 图像，如果失败则返回空 Mat。
 */
cv::Mat MainWindow::prepareImageForGLCM(const cv::Mat& inputImage, QString& log)
{
    cv::Mat analysisMat; // 用于存储中间单通道图像
    log = ""; // 清空日志

    // 1. 获取单通道图像
    if (inputImage.channels() == 2) {
        // 处理复数图像：计算幅度
        log += tr("Input is complex (2-channel), calculating magnitude.\n");
        std::vector<cv::Mat> channels;
        cv::split(inputImage, channels);
        // 确保通道是浮点类型
        if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
            log += tr("Converting complex channels to CV_32F for magnitude calculation.\n");
            channels[0].convertTo(channels[0], CV_32F);
            channels[1].convertTo(channels[1], CV_32F);
        }
        cv::magnitude(channels[0], channels[1], analysisMat); // 幅度图是单通道浮点型
    } else if (inputImage.channels() > 1) {
         // 处理多通道图像 (如彩色图)：转换为灰度图
         log += tr("Input is multi-channel (%1), converting to grayscale.\n").arg(inputImage.channels());
         cv::Mat sourceForCvt = inputImage;
         // 确保输入类型适合 cvtColor (通常是 8U, 16U, 32F)
         if (inputImage.depth() != CV_8U && inputImage.depth() != CV_16U && inputImage.depth() != CV_32F) {
             log += tr("Input depth (%1) not directly supported by cvtColor, converting to CV_8U first.\n").arg(cv::typeToString(inputImage.type()));
             // 简单的归一化到 CV_8U
              double minVal, maxVal;
              cv::minMaxLoc(inputImage, &minVal, &maxVal);
              if (maxVal > minVal)
                inputImage.convertTo(sourceForCvt, CV_8U, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal));
              else
                inputImage.convertTo(sourceForCvt, CV_8U);
         }

         // 尝试标准的 BGR/BGRA 到灰度的转换
         if (sourceForCvt.channels() >= 3) {
              try {
                int code = (sourceForCvt.channels() == 4) ? cv::COLOR_BGRA2GRAY : cv::COLOR_BGR2GRAY;
                cv::cvtColor(sourceForCvt, analysisMat, code);
              } catch (const cv::Exception& e) {
                 log += tr("Standard grayscale conversion failed (%1), likely not BGR format. Falling back to first channel.\n").arg(QString::fromStdString(e.what()));
                 std::vector<cv::Mat> channels;
                 cv::split(sourceForCvt, channels);
                 analysisMat = channels[0].clone(); // 使用克隆确保数据独立
              }
         } else {
             // 如果不是 3/4 通道，无法确定颜色空间，使用第一个通道
            log += tr("Input has %1 channels, not standard BGR/BGRA. Using first channel directly.\n").arg(sourceForCvt.channels());
            std::vector<cv::Mat> channels;
            cv::split(sourceForCvt, channels);
            analysisMat = channels[0].clone();
         }
    } else if (inputImage.channels() == 1) {
        // 输入已经是单通道
        log += tr("Input is already single-channel.\n");
        analysisMat = inputImage.clone(); // 克隆以防修改原始图像
    } else {
         log += tr("Error: Input image has 0 channels.\n");
        return cv::Mat(); // 返回空 Mat 表示错误
    }

    // 检查是否成功获得了单通道图像
    if (analysisMat.empty()) {
         log += tr("Error: Failed to obtain a single channel image from the input.\n");
         return cv::Mat();
    }

    // 2. 将单通道图像转换为 8 位无符号整数 (CV_8U)
    // 这是 GLCM 计算的标准要求，因为它通常基于 0-255 的灰度级
    cv::Mat outputMat;
    if (analysisMat.depth() != CV_8U) {
        log += tr("Normalizing single-channel image (type: %1) to 8-bit (0-255) for GLCM.\n")
                   .arg(cv::typeToString(analysisMat.type()));
        double minVal, maxVal;
        cv::minMaxLoc(analysisMat, &minVal, &maxVal);
        if (maxVal > minVal) {
            // 使用 convertTo 进行归一化和类型转换
            analysisMat.convertTo(outputMat, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
        } else {
            // 图像值恒定
            analysisMat.convertTo(outputMat, CV_8U);
            log += tr("Image has constant value after conversion to single channel.\n");
        }
    } else {
         log += tr("Single-channel image is already 8-bit (CV_8U).\n");
        outputMat = analysisMat; // 直接赋值，因为已经是 CV_8U
    }

    // 最终检查输出是否有效
    if (outputMat.empty() || outputMat.type() != CV_8UC1) {
        log += tr("Error: Failed to produce a valid CV_8UC1 image for GLCM.\n");
        return cv::Mat();
    }

    log += tr("Successfully prepared CV_8UC1 image for GLCM analysis.\n");
    return outputMat;
}

/**
 * @brief 计算给定图像的灰度共生矩阵 (GLCM)。
 * @param img 输入图像，必须是 CV_8UC1 类型。
 * @param glcm 输出的 GLCM 矩阵，类型为 CV_64F (使用 double 累加避免溢出)。
 * @param dx 像素对的水平偏移量。
 * @param dy 像素对的垂直偏移量。
 * @param levels GLCM 的维度，等于灰度级数 (通常为 256)。
 * @param symmetric 如果为 true，则计算对称 GLCM，即同时考虑 (i,j) 和 (j,i) 对。
 * @param normalize 如果为 true，则将 GLCM 归一化为概率矩阵 (元素和为 1)。
 */
void MainWindow::computeGLCM(const cv::Mat& img, cv::Mat& glcm, int dx, int dy, int levels, bool symmetric /* = true */, bool normalize /* = true */) {
    CV_Assert(img.type() == CV_8UC1 && "Input image must be 8-bit single-channel (CV_8UC1)");
    CV_Assert(levels > 0 && levels <= 256 && "Levels must be between 1 and 256");

    // 初始化 GLCM 矩阵为零，使用 CV_64F 存储计数值或概率
    glcm = cv::Mat::zeros(levels, levels, CV_64F);
    double totalPairs = 0; // 用于归一化的总像素对数

    // 遍历图像像素 (除了边缘，根据偏移量 dx, dy)
    for (int y = 0; y < img.rows; ++y) {
        for (int x = 0; x < img.cols; ++x) {
            // 计算邻居像素的坐标
            int nx = x + dx;
            int ny = y + dy;

            // 检查邻居像素是否在图像边界内
            if (nx >= 0 && nx < img.cols && ny >= 0 && ny < img.rows) {
                // 获取中心像素 (i) 和邻居像素 (j) 的灰度值
                uchar i = img.at<uchar>(y, x);
                uchar j = img.at<uchar>(ny, nx);

                // 确保灰度值在 [0, levels-1] 范围内 (对于 CV_8U 和 levels=256 应该总是满足)
                if (i < levels && j < levels) {
                     // 增加对应 GLCM 位置的计数
                     glcm.at<double>(i, j)++;
                     totalPairs++; // 计数有效像素对

                     // 如果计算对称 GLCM，也增加 (j, i) 位置的计数
                     if (symmetric) {
                         // 注意：这里简单地在 (j, i) 位置也加 1。
                         // 这意味着 (i, j) 和 (j, i) 都被计数了。
                         // 归一化时 totalPairs 也会相应增加。
                         // 如果 i != j，这对像素对贡献了两次计数。如果 i == j，贡献一次。
                         // 一些定义可能不同，例如只计算一次然后加转置，或者归一化分母不同。
                         glcm.at<double>(j, i)++;
                         if (i != j) { // 只有当 i!=j 时，对称操作才引入新的计数点
                            totalPairs++; // 在归一化时也要考虑这个增加的计数
                         }
                     }
                }
                // else { // 理论上不应发生
                //     qWarning("GLCM: Pixel value %d or %d out of expected range [0, %d)", i, j, levels);
                // }
            }
        }
    }

     // 根据需要进行归一化
    if (normalize) {
        if (totalPairs > 0) {
            glcm /= totalPairs; // 将计数值转换为概率 P(i,j)
        } else {
            // 处理没有有效像素对的情况 (例如，图像太小或偏移量太大)
            glcm = cv::Mat::zeros(levels, levels, CV_64F); // 保持为零矩阵
            logMessage(tr("Warning: No valid pixel pairs found for GLCM calculation with the given offset. GLCM is zero."));
            // qWarning("GLCM: No valid pixel pairs found for the given offset (dx=%d, dy=%d). GLCM is zero.", dx, dy);
        }
    }
    // 如果 normalize 为 false，glcm 将包含原始计数值。
}


/**
 * @brief 从归一化或未归一化的 GLCM 计算常见的纹理特征。
 * @param glcm 输入的 GLCM 矩阵 (CV_64F)。
 * @param levels GLCM 的维度 (灰度级数)。
 * @param[out] contrast 对比度：衡量图像局部变化的量，值越大纹理越深。
 * @param[out] energy 能量 (角二阶矩，ASM)：衡量图像灰度分布均匀性和纹理粗细度，值越大越规则。
 * @param[out] homogeneity 同质性 (逆差矩，IDM)：衡量图像局部灰度均匀性，值越大局部越均匀。
 * @param[out] correlation 相关性：衡量图像灰度值的线性相关程度，值越大线性相关性越强。
 */
void MainWindow::calculateGLCMFeatures(const cv::Mat& glcm, int levels,
                           double& contrast, double& energy, double& homogeneity, double& correlation)
{
    CV_Assert(glcm.type() == CV_64F && "GLCM must be of type CV_64F");
    CV_Assert(glcm.rows == levels && glcm.cols == levels && "GLCM dimensions must match levels");

    // 初始化特征值为 0
    contrast = 0.0;
    energy = 0.0;    // Angular Second Moment (ASM)
    homogeneity = 0.0; // Inverse Difference Moment (IDM)
    correlation = 0.0;

    double mean_i = 0.0, mean_j = 0.0;   // 行、列方向的均值
    double stddev_i = 0.0, stddev_j = 0.0; // 行、列方向的标准差
    double glcmSum = cv::sum(glcm)[0];   // 计算 GLCM 的总和，用于检查归一化和后续计算

    // 定义边际概率 Px(i) 和 Py(j)
    std::vector<double> px(levels, 0.0); // Px(i) = Sum_j P(i,j)
    std::vector<double> py(levels, 0.0); // Py(j) = Sum_i P(i,j)

    // --- 第一次遍历 GLCM: 计算 Px, Py, mean_i, mean_j, contrast, energy, homogeneity ---
    for (int i = 0; i < levels; ++i) {
        for (int j = 0; j < levels; ++j) {
            double p_ij = glcm.at<double>(i, j); // 获取 GLCM 元素值 P(i,j)

            // 累加边际概率
            px[i] += p_ij;
            py[j] += p_ij; // 如果 GLCM 是对称的，px 应该等于 py

            // 计算可以在此循环中直接累加的特征
            // 对比度：Sum_i Sum_j (i-j)^2 * P(i,j)
            contrast += (i - j) * (i - j) * p_ij;
            // 能量 (ASM): Sum_i Sum_j P(i,j)^2
            energy += p_ij * p_ij;
            // 同质性 (IDM): Sum_i Sum_j P(i,j) / (1 + |i-j|)
            homogeneity += p_ij / (1.0 + std::abs(i - j)); // 分母加 1 避免 i=j 时除以零
        }
    }

    // 计算均值 mean_i 和 mean_j
    // mean_i = Sum_i i * Px(i)
    // mean_j = Sum_j j * Py(j)
    for (int i = 0; i < levels; ++i) {
        mean_i += i * px[i];
        mean_j += i * py[i]; // 注意这里用 py[i] 而不是 py[j]，因为 py 的索引也代表灰度级
    }

    // 如果 GLCM 未归一化 (sum 不接近 1)，需要调整均值和边际概率用于后续标准差和相关性计算
    bool needsNormalizationAdjustment = std::abs(glcmSum - 1.0) > 1e-6 && glcmSum > 1e-9;
    if (needsNormalizationAdjustment) {
        logMessage(tr("GLCM sum (%1) is not close to 1. Adjusting means and standard deviations based on sum.").arg(glcmSum));
        // qWarning("GLCM sum is %f, features might be scaled if GLCM was not normalized.", glcmSum);
        mean_i /= glcmSum;
        mean_j /= glcmSum;
        // 归一化 Px 和 Py 以便正确计算标准差
        for(int i=0; i<levels; ++i) {
            px[i] /= glcmSum;
            py[i] /= glcmSum;
        }
    }

    // --- 第二次遍历 (或者基于 Px, Py 计算): 计算标准差 stddev_i, stddev_j ---
    // stddev_i = sqrt( Sum_i (i - mean_i)^2 * Px(i) )
    // stddev_j = sqrt( Sum_j (j - mean_j)^2 * Py(j) )
    for (int i = 0; i < levels; ++i) {
        stddev_i += (i - mean_i) * (i - mean_i) * px[i];
        stddev_j += (i - mean_j) * (i - mean_j) * py[i]; // 同样用 py[i]
    }
    // 开方得到标准差
    stddev_i = std::sqrt(stddev_i);
    stddev_j = std::sqrt(stddev_j);

    // --- 第三次遍历 GLCM: 计算相关性 Correlation ---
    // Correlation = Sum_i Sum_j [ (i - mean_i) * (j - mean_j) * P(i,j) ] / (stddev_i * stddev_j)
    if (stddev_i > 1e-9 && stddev_j > 1e-9) { // 避免除以零或非常小的标准差导致数值不稳定
         for (int i = 0; i < levels; ++i) {
            for (int j = 0; j < levels; ++j) {
                double p_ij = glcm.at<double>(i, j);
                // 如果 GLCM 未归一化，使用归一化后的 p_ij 进行计算
                 if (needsNormalizationAdjustment) {
                    p_ij /= glcmSum;
                 }
                 correlation += (i - mean_i) * (j - mean_j) * p_ij;
            }
        }
        correlation /= (stddev_i * stddev_j); // 除以标准差的乘积
    } else {
        // 如果任一标准差接近零 (例如图像是恒定灰度)，相关性未定义
        correlation = std::numeric_limits<double>::quiet_NaN(); // 使用 NaN 表示未定义
        logMessage(tr("Warning: GLCM Correlation is undefined because standard deviation of i or j is near zero."));
        // qWarning("GLCM Correlation is undefined because standard deviation is (near) zero (std_i=%.3e, std_j=%.3e).", stddev_i, stddev_j);
    }

    // 注意：不同文献或库对这些特征的精确定义可能略有差异，尤其是在处理未归一化 GLCM 或边界情况时。
}
