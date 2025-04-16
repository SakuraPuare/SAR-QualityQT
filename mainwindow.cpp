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
      // 确保通道是浮点类型以进行幅度计算
       if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
           logMessage(tr("Converting complex channels to CV_32F for magnitude display."));
           channels[0].convertTo(channels[0], CV_32F);
           channels[1].convertTo(channels[1], CV_32F);
       }
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

  // 运行选定的分析
  QString overviewResult = tr("Analysis Overview:\n");
   // Clear existing overview text before appending new results
  ui->overviewResultsTextEdit->setText(overviewResult);


  if (ui->checkBoxSNR->isChecked()) {
    logMessage(tr("Performing SNR/ENL analysis..."));
    performSNRAnalysis(); // 调用在 analysis_snr.cpp 中实现的函数
    currentStep++;
    ui->progressBar->setValue(currentStep);
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod1); // 切换到对应标签页
    QCoreApplication::processEvents();                      // 更新 UI
  }
  if (ui->checkBoxInfoContent->isChecked()) {
    logMessage(tr("Performing Information Content analysis..."));
    performInfoContentAnalysis(); // 调用在 analysis_infocontent.cpp 中实现的函数
    currentStep++;
    ui->progressBar->setValue(currentStep);
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod2);
    QCoreApplication::processEvents();
  }
  if (ui->checkBoxClarity->isChecked()) {
    logMessage(tr("Performing Clarity analysis..."));
    performClarityAnalysis(); // 调用在 analysis_clarity.cpp 中实现的函数
    currentStep++;
    ui->progressBar->setValue(currentStep);
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod3);
    QCoreApplication::processEvents();
  }
  if (ui->checkBoxRadiometricAccuracy->isChecked()) {
    logMessage(tr("Performing Radiometric Accuracy/Resolution analysis..."));
    performRadiometricAnalysis(); // 调用在 analysis_radiometric.cpp 中实现的函数
    currentStep++;
    ui->progressBar->setValue(currentStep);
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod4);
    QCoreApplication::processEvents();
  }
  if (ui->checkBoxGLCM->isChecked()) {
    logMessage(tr("Performing GLCM analysis..."));
    performGLCMAnalysis(); // 调用在 analysis_glcm.cpp 中实现的函数
    currentStep++;
    ui->progressBar->setValue(currentStep);
    ui->resultsTabWidget->setCurrentWidget(ui->tabMethod5);
    QCoreApplication::processEvents();
  }

  logMessage(tr("Analysis finished."));
  // Append final message to overview
  QString currentOverview = ui->overviewResultsTextEdit->toPlainText();
  ui->overviewResultsTextEdit->setText(
      currentOverview +
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


// --- 分析方法实现已移至 analysis_*.cpp 文件 ---
// void MainWindow::performSNRAnalysis() { ... }
// void MainWindow::performInfoContentAnalysis() { ... }
// void MainWindow::performClarityAnalysis() { ... }
// void MainWindow::performRadiometricAnalysis() { ... }
// void MainWindow::performGLCMAnalysis() { ... }
// cv::Mat MainWindow::prepareImageForGLCM(...) { ... }
// void MainWindow::computeGLCM(...) { ... }
// void MainWindow::calculateGLCMFeatures(...) { ... }


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
