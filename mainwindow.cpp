#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <vector>
#include <cmath>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setAcceptDrops(true); // 启用拖放接收
  GDALAllRegister();    // 初始化 GDAL 驱动
  logMessage(tr("Application started. GDAL initialized. Drag & Drop enabled."));
  // 可以选择在此显式连接信号和槽，但遵循命名约定通常足够
  // connect(ui->actionOpenImage, &QAction::triggered, this,
  // &MainWindow::on_actionOpenImage_triggered);
  // connect(ui->startAnalysisButton, &QPushButton::clicked, this,
  // &MainWindow::on_startAnalysisButton_clicked);
  // connect(ui->checkBoxSelectAll, &QCheckBox::toggled, this,
  // &MainWindow::on_checkBoxSelectAll_toggled);

  // 初始状态下禁用分析按钮，直到图像被加载
  ui->startAnalysisButton->setEnabled(false);
  // 初始化分析方法复选框状态 (根据 UI 文件设置，可以省略)
  on_checkBoxSelectAll_toggled(ui->checkBoxSelectAll->isChecked());

  // Initial default text for labels etc. should use tr()
  ui->valueFilename->setText(tr("N/A"));
  ui->valueDimensions->setText(tr("N/A"));
  ui->valueDataType->setText(tr("N/A"));
  ui->imageDisplayLabel->setText(tr("Image Display Area"));
  ui->overviewResultsTextEdit->setPlaceholderText(
      tr("Summary of all analysis results..."));
  ui->method1ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Method 1 (e.g., SNR: 15.2)"));
  ui->method2ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Method 2 (e.g., Information Content: 8.5 "
         "bits/pixel)"));
  ui->method3ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Method 3 (e.g., Clarity: 0.75)"));
  ui->method4ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Method 4 (e.g., Radiometric Accuracy: 95%)"));
  ui->method5ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Method 5 (e.g., GLCM Features: Contrast: 0.25, "
         "Correlation: 0.5)"));
  ui->logTextEdit->setPlaceholderText(
      tr("Analysis log messages will appear here..."));
}

MainWindow::~MainWindow() {
  closeCurrentImage(); // 确保关闭数据集
  delete ui;
}

void MainWindow::logMessage(const QString &message) {
  QString timestamp =
      QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
  ui->logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::closeCurrentImage() {
  if (poDataset != nullptr) {
    GDALClose(poDataset);
    poDataset = nullptr;
    logMessage(tr("Closed current image dataset."));
  }
  currentImage.release();
  currentFilename.clear();
  // 清空图像信息
  ui->valueFilename->setText(tr("N/A"));
  ui->valueDimensions->setText(tr("N/A"));
  ui->valueDataType->setText(tr("N/A"));
  // 清空图像显示
  ui->imageDisplayLabel->clear();
  ui->imageDisplayLabel->setText(tr("Image Display Area"));
  // 禁用分析按钮
  ui->startAnalysisButton->setEnabled(false);
  // 清空结果区域
  ui->overviewResultsTextEdit->clear();
  ui->method1ResultsTextEdit->clear();
  ui->method2ResultsTextEdit->clear();
  ui->method3ResultsTextEdit->clear();
  ui->method4ResultsTextEdit->clear();
  ui->method5ResultsTextEdit->clear();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  // 检查拖动的数据是否包含 URL (通常是文件路径)
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
      QString filePath = urls.first().toLocalFile(); // 获取第一个文件的本地路径
      QFileInfo fileInfo(filePath);
      QString suffix = fileInfo.suffix().toLower();
      // 检查文件后缀是否是支持的格式
      if (suffix == "tif" || suffix == "tiff" || suffix == "img" ||
          suffix == "hdr" || suffix == "dat") {
        event->acceptProposedAction(); // 如果是支持的文件，接受拖动操作
        logMessage(tr("Drag entered with supported file: %1")
                       .arg(fileInfo.fileName()));
        // 可以添加视觉反馈，比如改变边框颜色
        return;
      } else {
        logMessage(
            tr("Drag entered with unsupported file type: %1").arg(suffix));
      }
    }
  }
  event->ignore(); // 如果不是期望的数据，忽略该事件
}

void MainWindow::dropEvent(QDropEvent *event) {
  const QMimeData *mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    QList<QUrl> urls = mimeData->urls();
    if (!urls.isEmpty()) {
      QString filePath = urls.first().toLocalFile(); // 获取第一个拖放的文件路径
      logMessage(tr("File dropped: %1").arg(filePath));
      openImageFile(filePath); // 使用新的函数打开文件
      event->acceptProposedAction();
      // 恢复可能的视觉反馈
      return;
    }
  }
  event->ignore();
  // 恢复可能的视觉反馈
}

void MainWindow::openImageFile(const QString &filePath) {
  if (filePath.isEmpty()) {
    logMessage(tr("Image opening cancelled or invalid path."));
    return;
  }

  logMessage(tr("Attempting to open image: %1").arg(filePath));

  // 先关闭之前打开的图像
  closeCurrentImage();

  // 使用 GDAL 打开文件
  poDataset =
      (GDALDataset *)GDALOpen(filePath.toStdString().c_str(), GA_ReadOnly);

  if (poDataset == nullptr) {
    logMessage(tr("Error: Could not open image file: %1").arg(filePath));
    QMessageBox::critical(this, tr("Error"),
                          tr("Could not open the selected image file. Check "
                             "GDAL configuration and file format."));
    return;
  }

  currentFilename = QFileInfo(filePath).fileName();
  logMessage(tr("Successfully opened: %1").arg(currentFilename));

  // 更新图像信息
  updateImageInfo();

  // 读取图像数据到 OpenCV Mat
  int width = poDataset->GetRasterXSize();
  int height = poDataset->GetRasterYSize();
  int numBands = poDataset->GetRasterCount(); // SAR 通常是单波段

  if (numBands < 1) {
    logMessage(tr("Error: Image has no raster bands."));
    QMessageBox::critical(this, tr("Error"), tr("Image has no raster bands."));
    closeCurrentImage();
    return;
  }

  GDALRasterBand *poBand = poDataset->GetRasterBand(1); // 读取第一个波段
  if (!poBand) {
    logMessage(tr("Error: Could not get raster band 1."));
    QMessageBox::critical(this, tr("Error"),
                          tr("Could not get raster band 1."));
    closeCurrentImage();
    return;
  }

  GDALDataType dataType = poBand->GetRasterDataType();

  // 为 OpenCV Mat 选择合适的数据类型
  int cvType = -1;
  bool isComplex = false; // 标记是否为复数
  switch (dataType) {
  case GDT_Byte:
    cvType = CV_8U;
    break;
  case GDT_UInt16:
    cvType = CV_16U;
    break;
  case GDT_Int16:
    cvType = CV_16S;
    break;
  case GDT_UInt32:
    cvType = CV_32S;
    break; // 注意转换风险
  case GDT_Int32:
    cvType = CV_32S;
    break;
  case GDT_Float32:
    cvType = CV_32F;
    break;
  case GDT_Float64:
    cvType = CV_64F;
    break;
  case GDT_CInt16:
    cvType = CV_16S;
    isComplex = true;
    break; // 底层类型 + 标记
  case GDT_CInt32:
    cvType = CV_32S;
    isComplex = true;
    break;
  case GDT_CFloat32:
    cvType = CV_32F;
    isComplex = true;
    break;
  case GDT_CFloat64:
    cvType = CV_64F;
    isComplex = true;
    break;
  default:
    logMessage(tr("Error: Unsupported GDAL data type: %1")
                   .arg(GDALGetDataTypeName(dataType)));
    QMessageBox::critical(this, tr("Error"),
                          tr("Unsupported image data type."));
    closeCurrentImage();
    return;
  }

  // 处理复数和非复数数据
  if (isComplex) {
    logMessage(tr("Complex data type detected. Reading as 2-channel matrix "
                  "(real, imag)."));
    int elementSize = GDALGetDataTypeSizeBytes(dataType) / 2; // 每个分量的大小
    currentImage =
        cv::Mat(height, width, CV_MAKETYPE(cvType, 2)); // 创建双通道 Mat

    // RasterIO 需要知道每个像素的总大小 (bytesPerPixel) 和每行的大小
    // (bytesPerLine) 对于复数，GDALDataType 已经代表了复数类型的大小
    CPLErr err = poBand->RasterIO(
        GF_Read, 0, 0, width, height, currentImage.ptr(), width, height,
        dataType, // 使用复数类型读取
        GDALGetDataTypeSizeBytes(
            dataType), // Pixel spacing (bytes per complex pixel)
        GDALGetDataTypeSizeBytes(dataType) * width); // Line spacing

    if (err != CE_None) {
      logMessage(tr("Error reading complex raster data."));
      QMessageBox::critical(this, tr("Error"),
                            tr("Failed to read complex image data."));
      closeCurrentImage();
      return;
    }

    // 分析时通常需要幅度或强度图像，这里先将复数数据存储起来
    // displayImage 将需要处理双通道输入，或者我们在这里计算幅度
    logMessage(tr("Stored complex data (real, imag). Analysis functions need "
                  "to handle this."));
    // 为了显示，我们计算幅度图
    // cv::Mat magnitudeImage;
    // std::vector<cv::Mat> channels(2);
    // cv::split(currentImage, channels); // 分离实部和虚部
    // cv::magnitude(channels[0], channels[1], magnitudeImage);
    // displayImage(magnitudeImage); // 显示幅度图
    // 注意：如果后续分析需要原始复数数据，不要覆盖 currentImage

  } else {
    // 读取非复数数据
    logMessage(tr("Reading non-complex data."));
    currentImage = cv::Mat(height, width, cvType);
    CPLErr err =
        poBand->RasterIO(GF_Read, 0, 0, width, height, currentImage.ptr(),
                         width, height, dataType, 0, 0); // 使用自动像素和行间距
    if (err != CE_None) {
      logMessage(tr("Error reading raster data."));
      QMessageBox::critical(this, tr("Error"),
                            tr("Failed to read image data."));
      closeCurrentImage();
      return;
    }
    // displayImage(currentImage); // 直接显示
  }

  logMessage(tr("Image data read into cv::Mat. Dimensions: %1x%2, Type: %3, "
                "Channels: %4")
                 .arg(width)
                 .arg(height)
                 .arg(cv::typeToString(currentImage.type()))
                 .arg(currentImage.channels()));

  // 显示图像 (需要调整 displayImage 以处理可能的复数数据或其幅度)
  if (!currentImage.empty()) {
    if (isComplex) {
      // 决定如何显示复数数据，这里显示幅度
      cv::Mat magnitudeImage;
      std::vector<cv::Mat> channels(2);
      cv::split(currentImage, channels); // 分离实部和虚部
      cv::magnitude(channels[0], channels[1], magnitudeImage);
      logMessage(tr("Displaying magnitude of complex image."));
      displayImage(magnitudeImage);
    } else {
      displayImage(currentImage); // 显示非复数图像
    }
    // 图像加载成功后启用分析按钮
    ui->startAnalysisButton->setEnabled(true);
  } else {
    logMessage(tr("Error: cv::Mat is empty after reading."));
    QMessageBox::critical(this, tr("Error"),
                          tr("Failed to process image data after reading."));
    closeCurrentImage();
  }
}

void MainWindow::updateImageInfo() {
  if (!poDataset)
    return;

  ui->valueFilename->setText(currentFilename);
  int width = poDataset->GetRasterXSize();
  int height = poDataset->GetRasterYSize();
  ui->valueDimensions->setText(QString("%1 x %2").arg(width).arg(height));

  GDALRasterBand *poBand = poDataset->GetRasterBand(1);
  if (poBand) {
    GDALDataType dataType = poBand->GetRasterDataType();
    ui->valueDataType->setText(GDALGetDataTypeName(dataType));
  } else {
    ui->valueDataType->setText(tr("Error reading type"));
  }
}

void MainWindow::displayImage(const cv::Mat &image) {
  if (image.empty()) {
    logMessage(tr("Cannot display empty image."));
    return;
  }

  logMessage(tr("Displaying image with type: %1, channels: %2")
                 .arg(cv::typeToString(image.type()))
                 .arg(image.channels()));

  cv::Mat displayMat;
  // 确保输入是单通道的，因为我们要转为灰度 QImage
  cv::Mat singleChannelImage;
  if (image.channels() == 1) {
    singleChannelImage = image;
  } else if (image.channels() > 1) {
    logMessage(
        tr("Input image has %1 channels. Taking the first channel for display.")
            .arg(image.channels()));
    std::vector<cv::Mat> channels;
    cv::split(image, channels);
    singleChannelImage =
        channels[0]; // 或者显示幅度，如果输入是复数计算后的幅度
  } else {
    logMessage(tr("Error: Input image has 0 channels."));
    return;
  }

  // 将单通道图像转换为适合显示的 8 位灰度图
  if (singleChannelImage.depth() == CV_8U) {
    displayMat = singleChannelImage.clone();
  } else {
    // 归一化到 0-255
    double minVal, maxVal;
    cv::minMaxLoc(singleChannelImage, &minVal, &maxVal);
    logMessage(tr("Normalizing for display. Original min: %1, max: %2")
                   .arg(minVal)
                   .arg(maxVal));
    if (maxVal > minVal) {
      // 使用 CV_64F 进行中间计算以提高精度
      singleChannelImage.convertTo(displayMat, CV_64F);
      displayMat = (displayMat - minVal) * (255.0 / (maxVal - minVal));
      displayMat.convertTo(displayMat, CV_8U); // 最后转回 CV_8U
    } else {
      // 如果图像是恒定值
      singleChannelImage.convertTo(displayMat, CV_8U);
    }
  }

  // 转换为 QImage
  QImage qimg;
  if (displayMat.channels() == 1 &&
      displayMat.depth() == CV_8U) { // 确认是 8 位单通道
    qimg = QImage((const uchar *)displayMat.data, displayMat.cols,
                  displayMat.rows, displayMat.step, QImage::Format_Grayscale8);
  } else {
    logMessage(tr("Error: Could not prepare image for QImage conversion "
                  "(Expected CV_8UC1, got Type %1)")
                   .arg(cv::typeToString(displayMat.type())));
    return; // 无法转换
  }

  if (qimg.isNull()) {
    logMessage(tr("Error converting cv::Mat to QImage."));
    return;
  }

  // 在 QLabel 中显示 QPixmap
  QPixmap pixmap = QPixmap::fromImage(qimg);
  // 缩放图像以适应 QLabel 大小，保持纵横比
  ui->imageDisplayLabel->setPixmap(pixmap.scaled(ui->imageDisplayLabel->size(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation));
  logMessage(tr("Image displayed."));
}

void MainWindow::on_startAnalysisButton_clicked() {
  if (currentImage.empty() || poDataset == nullptr) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Please open an image first."));
    logMessage(tr("Analysis skipped: No image loaded."));
    return;
  }

  logMessage(tr("Starting analysis..."));
  ui->progressBar->setValue(0); // Reset progress bar

  // 清空之前的分析结果
  ui->overviewResultsTextEdit->clear();
  ui->method1ResultsTextEdit->clear();
  ui->method2ResultsTextEdit->clear();
  ui->method3ResultsTextEdit->clear();
  ui->method4ResultsTextEdit->clear();
  ui->method5ResultsTextEdit->clear();

  // 检查选择了哪些方法
  int totalSteps = 0;
  if (ui->checkBoxSNR->isChecked())
    totalSteps++;
  if (ui->checkBoxInfoContent->isChecked())
    totalSteps++;
  if (ui->checkBoxClarity->isChecked())
    totalSteps++;
  if (ui->checkBoxRadiometricAccuracy->isChecked())
    totalSteps++;
  if (ui->checkBoxGLCM->isChecked())
    totalSteps++;

  if (totalSteps == 0) {
    QMessageBox::information(this, tr("Info"),
                             tr("Please select at least one analysis method."));
    logMessage(tr("Analysis stopped: No methods selected."));
    return;
  }

  int currentStep = 0;
  ui->progressBar->setMaximum(totalSteps); // 设置进度条最大值

  // 运行选定的分析 (目前只是占位符)
  QString overviewResult = tr("Analysis Overview:\n");

  if (ui->checkBoxSNR->isChecked()) {
    logMessage(tr("Performing SNR/ENL analysis..."));
    performSNRAnalysis(); // 调用实际分析函数
    currentStep++;
    ui->progressBar->setValue(currentStep);
    // 假设 performSNRAnalysis 会更新 ui->method1ResultsTextEdit
    // 并返回结果字符串 overviewResult += "SNR/ENL: [Result]\n";
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod1); // 切换到对应标签页
    QCoreApplication::processEvents();                      // 更新 UI
  }
  if (ui->checkBoxInfoContent->isChecked()) {
    logMessage(tr("Performing Information Content analysis..."));
    performInfoContentAnalysis();
    currentStep++;
    ui->progressBar->setValue(currentStep);
    // overviewResult += "Information Content: [Result]\n";
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod2);
    QCoreApplication::processEvents();
  }
  if (ui->checkBoxClarity->isChecked()) {
    logMessage(tr("Performing Clarity analysis..."));
    performClarityAnalysis();
    currentStep++;
    ui->progressBar->setValue(currentStep);
    // overviewResult += "Clarity (Edge Response): [Result]\n";
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod3);
    QCoreApplication::processEvents();
  }
  if (ui->checkBoxRadiometricAccuracy->isChecked()) {
    logMessage(tr("Performing Radiometric Accuracy/Resolution analysis..."));
    performRadiometricAnalysis();
    currentStep++;
    ui->progressBar->setValue(currentStep);
    // overviewResult += "Radiometric Accuracy: [Result]\n";
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod4);
    QCoreApplication::processEvents();
  }
  if (ui->checkBoxGLCM->isChecked()) {
    logMessage(tr("Performing GLCM analysis..."));
    performGLCMAnalysis();
    currentStep++;
    ui->progressBar->setValue(currentStep);
    // overviewResult += "GLCM Features: [Result]\n";
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod5);
    QCoreApplication::processEvents();
  }

  logMessage(tr("Analysis finished."));
  ui->overviewResultsTextEdit->setText(
      overviewResult +
      tr("\n(Detailed results in respective tabs)"));      // 更新总览
  ui->resultsTabWidget->setCurrentWidget(ui->tabOverview); // 最后切回总览标签页
  QMessageBox::information(
      this, tr("Analysis Complete"),
      tr("Image analysis finished. Check the results tabs."));
}

void MainWindow::on_checkBoxSelectAll_toggled(bool checked) {
  ui->checkBoxSNR->setChecked(checked);
  ui->checkBoxInfoContent->setChecked(checked);
  ui->checkBoxClarity->setChecked(checked);
  ui->checkBoxRadiometricAccuracy->setChecked(checked);
  ui->checkBoxGLCM->setChecked(checked);
  logMessage(checked ? tr("Analysis methods selected all.")
                     : tr("Analysis methods deselected all."));
}

// --- 分析方法占位符实现 ---
// 这些函数需要你根据具体的算法使用 OpenCV 和 GDAL 来实现

void MainWindow::performSNRAnalysis() {
  QString resultLog = tr("SNR/ENL Analysis Results:\n");
  QString overviewResult = tr("SNR/ENL: ");

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded.");
    ui->method1ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error - No Image"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat; // 用于分析的矩阵 (通常是强度或幅度)

  // 1. 准备用于分析的单通道幅度/强度图像
  if (currentImage.channels() == 2) {
    // 处理复数数据：计算幅度
    logMessage(tr("SNR Analysis: Input is complex (2-channel), calculating magnitude."));
    std::vector<cv::Mat> channels;
    cv::split(currentImage, channels);
    // 幅度计算需要浮点型
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        logMessage(tr("Converting complex channels to CV_32F for magnitude calculation."));
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat); // analysisMat 现在是单通道浮点型
    resultLog += tr("Using magnitude image calculated from complex data.\n");
  } else if (currentImage.channels() == 1) {
    // 已经是单通道（通常是强度或已处理的幅度）
    logMessage(tr("SNR Analysis: Input is single-channel."));
    analysisMat = currentImage.clone(); // 克隆以防修改
    // 如果不是浮点型，转换为浮点型以进行精确计算
    if (analysisMat.depth() != CV_32F && analysisMat.depth() != CV_64F) {
        logMessage(tr("Converting single-channel image to CV_32F for analysis."));
        analysisMat.convertTo(analysisMat, CV_32F);
        resultLog += tr("Converted input to floating-point type (CV_32F).\n");
    } else {
         resultLog += tr("Using existing single-channel floating-point data.\n");
    }
  } else {
    // 不支持的通道数
     resultLog = tr("Error: Unsupported channel count (%1) for SNR analysis. Expected 1 (intensity/amplitude) or 2 (complex).")
                .arg(currentImage.channels());
     ui->method1ResultsTextEdit->setText(resultLog);
     ui->overviewResultsTextEdit->append(overviewResult + tr("Error - Unsupported Channels"));
     logMessage(resultLog);
     return;
  }

  // 2. 计算均值和标准差
  cv::Scalar meanValue, stdDevValue;
  try {
      // 计算整个图像的均值和标准差
      // 注意：对于大图像，这可能比较耗时
      cv::meanStdDev(analysisMat, meanValue, stdDevValue);

      double mean = meanValue[0]; // 取第一个通道的值
      double stddev = stdDevValue[0];

      resultLog += tr("\n--- Global Statistics ---\n");
      resultLog += tr("Mean (μ): %1\n").arg(mean);
      resultLog += tr("Standard Deviation (σ): %1\n").arg(stddev);

      // 3. 计算 SNR 和 ENL
      if (stddev > 1e-9) { // 避免除以零或非常小的值
          double snr = mean / stddev;
          double enl = snr * snr; // ENL = (μ/σ)^2

          resultLog += tr("\n--- Quality Metrics (Global) ---\n");
          resultLog += tr("Signal-to-Noise Ratio (SNR = μ/σ): %1\n").arg(snr);
          resultLog += tr("Equivalent Number of Looks (ENL = (μ/σ)²): %1\n").arg(enl);

          overviewResult += tr("SNR=%1, ENL=%2 (Global)")
                               .arg(QString::number(snr, 'f', 2))
                               .arg(QString::number(enl, 'f', 2));
          logMessage(tr("SNR/ENL calculated (Global): Mean=%1, StdDev=%2, SNR=%3, ENL=%4")
                         .arg(mean).arg(stddev).arg(snr).arg(enl));
      } else {
          resultLog += tr("\nWarning: Standard deviation is close to zero. Cannot calculate SNR/ENL reliably.");
          overviewResult += tr("N/A (StdDev near zero)");
          logMessage(tr("SNR/ENL calculation skipped: Standard deviation is near zero."));
      }

       resultLog += tr("\nNote: These metrics are calculated globally. For more accurate results, select a homogeneous region.");

  } catch (const cv::Exception& e) {
      resultLog += tr("\nError during mean/stddev calculation: %1")
                       .arg(QString::fromStdString(e.msg));
      overviewResult += tr("Error - Calculation Failed");
      logMessage(tr("OpenCV Error during SNR/ENL calculation: %1")
                     .arg(QString::fromStdString(e.msg)));
  }


  // 4. 更新 UI
  ui->method1ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}

void MainWindow::performInfoContentAnalysis() {
  QString resultLog; // 用于在文本框中显示详细信息
  QString overviewResult = tr("Information Content: "); // 用于在总览中显示结果

  if (currentImage.empty()) {
    resultLog = tr("Error: No image loaded.");
    ui->method2ResultsTextEdit->setText(resultLog);
    ui->overviewResultsTextEdit->append(overviewResult + tr("Error"));
    logMessage(resultLog);
    return;
  }

  cv::Mat analysisMat; // 用于计算熵的矩阵

  // 检查数据是否为复数 (这里假设 2 通道表示之前加载的复数数据)
  // 注意：更可靠的方法是在 openImageFile 中存储 isComplex 标志并在需要时访问它
  if (currentImage.channels() == 2) {
    logMessage(tr("Input is 2-channel, calculating magnitude for entropy "
                  "analysis."));
    std::vector<cv::Mat> channels;
    cv::split(currentImage, channels);
    // 确保通道是浮点类型，如果不是则转换 (幅度计算通常需要浮点)
    if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
        logMessage(tr("Converting complex channels to CV_32F for magnitude calculation."));
        channels[0].convertTo(channels[0], CV_32F);
        channels[1].convertTo(channels[1], CV_32F);
    }
    cv::magnitude(channels[0], channels[1], analysisMat); // analysisMat 现在是单通道浮点型
    resultLog = tr("Calculated magnitude from complex data.\n");
  } else if (currentImage.channels() > 1) {
    // 对于非复数的多通道数据，转换为灰度图
    // 注意：如果原始多通道图像不是常见的 BGR/RGB 顺序，转换可能需要调整
    logMessage(tr("Input is multi-channel (%1), converting to grayscale for "
                  "entropy analysis.")
                   .arg(currentImage.channels()));
    // 尝试使用 COLOR_BGR2GRAY，如果失败或类型不支持，则用第一通道
    try {
         // 确保输入类型与 cvtColor 兼容，通常是 CV_8U, CV_16U, CV_32F
         cv::Mat sourceForCvt;
         if (currentImage.depth() != CV_8U && currentImage.depth() != CV_16U && currentImage.depth() != CV_32F) {
             // 选择一个合适的转换类型，例如归一化到 CV_8U 或转换为 CV_32F
             logMessage(tr("Converting multi-channel image to CV_8UC3 before grayscale conversion."));
      double minVal, maxVal;
             // 需要对每个通道找到全局 min/max 进行归一化，这里简化处理
             currentImage.convertTo(sourceForCvt, CV_8U); // 简单截断/缩放，可能不理想
    } else {
             sourceForCvt = currentImage;
         }
         // 假设输入是 3 或 4 通道的 BGR(A) 图像，否则可能需要其他转换码
         if (sourceForCvt.channels() >= 3)
            cv::cvtColor(sourceForCvt, analysisMat, cv::COLOR_BGR2GRAY);
         else { // 如果不是标准的 3/4 通道，回退到第一通道
             logMessage(tr("Cannot determine standard color format, using first channel."));
             std::vector<cv::Mat> channels;
             cv::split(sourceForCvt, channels);
             analysisMat = channels[0];
         }

    } catch (const cv::Exception& e) {
        logMessage(tr("cvtColor failed (%1), falling back to first channel.")
                        .arg(QString::fromStdString(e.msg)));
         std::vector<cv::Mat> channels;
         cv::split(currentImage, channels);
         analysisMat = channels[0];
    }
     resultLog = tr("Converted multi-channel image to single channel.\n");

  } else {
    // 已经是单通道图像
    analysisMat = currentImage.clone(); // 克隆以防修改原始数据
    resultLog = tr("Input is single-channel.\n");
  }

  // --- 确保 analysisMat 是单通道 ---
  if (analysisMat.channels() != 1) {
      logMessage(tr("Error: Could not obtain single-channel image for entropy calculation."));
       resultLog += tr("\nError: Failed to get single-channel data.");
       ui->method2ResultsTextEdit->setText(resultLog);
       ui->overviewResultsTextEdit->append(overviewResult + tr("Error"));
       return;
  }


  // --- 将 analysisMat (单通道) 转换为 CV_8U 以计算直方图 ---
  cv::Mat histMat;
  if (analysisMat.depth() != CV_8U) {
    logMessage(tr("Normalizing single-channel image (type: %1) to 8-bit for histogram.")
                   .arg(cv::typeToString(analysisMat.type())));
    resultLog += tr("Normalized data to 8-bit range (0-255) for calculation.\n");
    // 归一化到 0-255
    double minVal, maxVal;
    cv::minMaxLoc(analysisMat, &minVal, &maxVal);
    if (maxVal > minVal) {
      // 使用 CV_64F 进行中间计算以提高精度
      cv::Mat tempMat;
      analysisMat.convertTo(tempMat, CV_64F);
      tempMat = (tempMat - minVal) * (255.0 / (maxVal - minVal));
      tempMat.convertTo(histMat, CV_8U); // 最后转回 CV_8U
    } else {
      // 处理图像值恒定的情况
      analysisMat.convertTo(histMat, CV_8U); // 结果将是单个值 (通常是 0)
      resultLog += tr("Image has constant value, entropy will be 0.\n");
    }
  } else {
    histMat = analysisMat; // 已经是 CV_8U
    resultLog += tr("Using existing 8-bit data directly for calculation.\n");
  }

  // --- 计算直方图 ---
  cv::Mat hist;
  int histSize = 256;       // 对于 CV_8U
  float range[] = {0, 256}; // CV_8U 的范围
  const float *histRange = {range};
  bool uniform = true;
  bool accumulate = false;
  float entropy = 0.0f; // 初始化熵

  try {
    cv::calcHist(&histMat, 1, // images
                 0,          // channels (use channel 0)
                 cv::Mat(),  // mask
                 hist,       // output histogram
                 1,          // histogram dimensions
                 &histSize,  // histogram size for each dimension
                 &histRange, // range for each dimension
                 uniform, accumulate);

    // 归一化直方图 (得到概率 p_i)
    hist /= histMat.total(); // histMat.total() 是像素总数

    // --- 计算熵 H = -sum(p_i * log2(p_i)) ---
    for (int i = 0; i < histSize; i++) {
      float p = hist.at<float>(i);
      if (p > 0) { // 避免 log2(0) 导致 -inf
        entropy -= p * log2f(p); // 使用 log2f 处理 float
      }
    }

    resultLog += tr("\nCalculated Entropy: %1 bits/pixel").arg(entropy);
    overviewResult +=
        tr("%1 bits/pixel").arg(QString::number(entropy, 'f', 4)); // 格式化概览结果
    logMessage(tr("Entropy calculation successful: %1 bits/pixel").arg(entropy));

  } catch (const cv::Exception &e) {
    resultLog += tr("\nError during histogram/entropy calculation: %1")
                     .arg(QString::fromStdString(e.msg));
    overviewResult += tr("Error");
    logMessage(tr("OpenCV Error during entropy calculation: %1")
                   .arg(QString::fromStdString(e.msg)));
     // 即使出错，也要显示日志
     ui->method2ResultsTextEdit->setText(resultLog);
     ui->overviewResultsTextEdit->append(overviewResult);
     return; // 提前返回
  }

  // --- 更新 UI ---
  ui->method2ResultsTextEdit->setText(resultLog);
  ui->overviewResultsTextEdit->append(overviewResult);
}

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
  analysisMat = prepareImageForGLCM(currentImage, prepareLog); // Reuse preparation
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
         cv::Mat sourceForCvt = inputImage;
         if (inputImage.depth() != CV_8U && inputImage.depth() != CV_16U && inputImage.depth() != CV_32F) {
             log += tr("Converting multi-channel image to CV_8U before grayscale.\n");
             // Simple conversion, might lose precision but needed for cvtColor often
              double minVal, maxVal;
              cv::minMaxLoc(inputImage, &minVal, &maxVal); // Estimate range loosely
              if (maxVal > minVal)
                inputImage.convertTo(sourceForCvt, CV_8U, 255.0/(maxVal-minVal), -minVal*255.0/(maxVal-minVal));
              else
                inputImage.convertTo(sourceForCvt, CV_8U);
         }
         // Assuming BGR(A) for color conversion, otherwise fallback
         if (sourceForCvt.channels() >= 3) {
              try {
                cv::cvtColor(sourceForCvt, analysisMat, cv::COLOR_BGR2GRAY);
              } catch (...) { // Catch potential errors if not BGR
                 log += tr("Grayscale conversion failed, using first channel.\n");
                 std::vector<cv::Mat> channels;
                 cv::split(sourceForCvt, channels);
                 analysisMat = channels[0];
              }
         } else {
            log += tr("Non-standard multi-channel, using first channel.\n");
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

    // 2. Convert to 8-bit unsigned integer (CV_8U)
    cv::Mat outputMat;
    if (analysisMat.depth() != CV_8U) {
        log += tr("Normalizing single-channel image (type: %1) to 8-bit for GLCM.\n")
                   .arg(cv::typeToString(analysisMat.type()));
        double minVal, maxVal;
        cv::minMaxLoc(analysisMat, &minVal, &maxVal);
        if (maxVal > minVal) {
            analysisMat.convertTo(outputMat, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
        } else {
            analysisMat.convertTo(outputMat, CV_8U); // Constant value image
            log += tr("Image has constant value.\n");
        }
    } else {
         log += tr("Image is already 8-bit single-channel.\n");
        outputMat = analysisMat; // Already CV_8U
    }

    return outputMat;
}

// Helper function to compute GLCM
// img: Input CV_8UC1 image
// glcm: Output GLCM matrix (levels x levels, CV_64F)
// dx, dy: Offset (e.g., dx=1, dy=0 for 0 degrees, dx=1, dy=1 for 45 degrees)
// levels: Number of gray levels (usually 256 for CV_8U)
// symmetric: If true, counts both (i, j) and (j, i) pairs. Recommended.
// normalize: If true, normalizes the GLCM so that sum is 1.0.
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

                glcm.at<double>(i, j)++;
                totalPairs++;

                if (symmetric) {
                    glcm.at<double>(j, i)++; // Count the symmetric pair
                     totalPairs++; // Count symmetric pair as well if normalization is based on symmetric count
                }
            }
        }
    }

    // Normalization is slightly different if symmetric is true vs false regarding the denominator
    // Here we normalize by the number of pairs counted.
    if (normalize && totalPairs > 0) {
        glcm /= totalPairs; // Normalize to get probabilities P_ij
    } else if (normalize && symmetric && totalPairs == 0) {
        // Handle case of empty image or no valid pairs, avoid division by zero
         glcm = cv::Mat::zeros(levels, levels, CV_64F);
    }
    // If not symmetric and normalize=true, the sum of glcm might not be 1.0
    // if pairs were only counted in one direction. Standard practice usually
    // implies symmetric=true or normalization handles the pair counting implicitly.
    // For simplicity, we use totalPairs as counted.
}

// Helper function to calculate GLCM features
// glcm: Input normalized GLCM (levels x levels, CV_64F, sum should be ~1.0)
// levels: Number of gray levels used to compute GLCM
// contrast, correlation, energy, homogeneity: Output features
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
    double glcmSum = cv::sum(glcm)[0]; // Check sum for normalization factor (should be ~1.0)


    // Calculate marginal probabilities (px, py) and means (mean_i, mean_j) first
    std::vector<double> px(levels, 0.0); // P_x(i) = Sum_j P(i,j)
    std::vector<double> py(levels, 0.0); // P_y(j) = Sum_i P(i,j)

    for (int i = 0; i < levels; ++i) {
        for (int j = 0; j < levels; ++j) {
            double p_ij = glcm.at<double>(i, j);
            px[i] += p_ij;
            py[j] += p_ij; // Note: if symmetric, px should equal py

            // Features that can be calculated directly
            contrast += (i - j) * (i - j) * p_ij;
            energy += p_ij * p_ij;
            homogeneity += p_ij / (1.0 + std::abs(i - j));
        }
        mean_i += i * px[i];
        mean_j += i * py[i]; // Use 'i' as index for py sum as well
    }


    // Calculate standard deviations (stddev_i, stddev_j)
    for (int i = 0; i < levels; ++i) {
        stddev_i += (i - mean_i) * (i - mean_i) * px[i];
        stddev_j += (i - mean_j) * (i - mean_j) * py[i];
    }
    // Convert sum of squares to standard deviation
    stddev_i = std::sqrt(stddev_i);
    stddev_j = std::sqrt(stddev_j);

    // Calculate correlation
    // Correlation = Sum_i Sum_j [ (i - mean_i)(j - mean_j) * P(i,j) ] / (stddev_i * stddev_j)
    if (stddev_i > 1e-6 && stddev_j > 1e-6) { // Avoid division by zero
         for (int i = 0; i < levels; ++i) {
            for (int j = 0; j < levels; ++j) {
                correlation += (i - mean_i) * (j - mean_j) * glcm.at<double>(i, j);
            }
        }
        correlation /= (stddev_i * stddev_j);
    } else {
        correlation = std::numeric_limits<double>::quiet_NaN(); // Indicate undefined
    }


    // Optional: Check for GLCM sum ~ 1.0 if normalization was expected
    // if (std::abs(glcmSum - 1.0) > 1e-5) {
    //     logMessage(QString("Warning: GLCM sum is %1, expected ~1.0. Features might be scaled.").arg(glcmSum));
    // }

}

// 添加缺失的槽函数实现
void MainWindow::on_actionOpenImage_triggered() {
  // 打开文件对话框让用户选择图像文件
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("Open Image"),
      "", // 初始目录 (空表示上次使用的目录或默认目录)
      tr("Image Files (*.tif *.tiff *.img *.hdr *.dat);;All Files "
         "(*)")); // 文件过滤器

  if (!filePath.isEmpty()) {
    logMessage(tr("Opening image via menu: %1").arg(filePath));
    openImageFile(filePath); // 调用现有函数打开文件
  } else {
    logMessage(tr("Image opening cancelled by user."));
  }
}
