#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <vector>
#include <cmath>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setAcceptDrops(true); // 启用拖放接收
  GDALAllRegister();    // 初始化 GDAL 驱动，注册所有已知的驱动
  logMessage(tr("Application started. GDAL initialized. Drag & Drop enabled."));
  // connect 语句通常由 UI 设计器自动处理或遵循命名约定 (on_widgetName_signalName)

  // 初始状态下禁用分析按钮，直到图像被加载
  ui->startAnalysisButton->setEnabled(false);
  // 初始化复选框状态
  on_checkBoxSelectAll_toggled(ui->checkBoxSelectAll->isChecked());

  // 使用 tr() 以支持国际化
  ui->valueFilename->setText(tr("N/A"));
  ui->valueDimensions->setText(tr("N/A"));
  ui->valueDataType->setText(tr("N/A"));
  ui->imageDisplayLabel->setText(tr("Image Display Area")); // 初始提示文本
  ui->imageDisplayLabel->setAlignment(Qt::AlignCenter); // 居中显示提示文本
  ui->imageDisplayLabel->setStyleSheet("QLabel { color: grey; }"); // 灰色提示文本

  // 为结果和日志文本框设置占位符文本
  ui->overviewResultsTextEdit->setPlaceholderText(
      tr("Summary of all selected analysis results will appear here..."));
  ui->method1ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for SNR/ENL Analysis..."));
  ui->method2ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Information Content (Entropy)..."));
  ui->method3ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Clarity (Gradient Magnitude)..."));
  ui->method4ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for Radiometric Stats (Min, Max, Mean, StdDev)..."));
  ui->method5ResultsTextEdit->setPlaceholderText(
      tr("Detailed results for GLCM Texture Features..."));
  ui->logTextEdit->setPlaceholderText(
      tr("Log messages (loading, analysis steps, errors) will appear here..."));
}

MainWindow::~MainWindow() {
  closeCurrentImage(); // 清理 GDAL 数据集
  delete ui;
}

void MainWindow::logMessage(const QString &message) {
  QString timestamp =
      QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
  // 使用追加方式，避免覆盖旧日志
  ui->logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::closeCurrentImage() {
  if (poDataset != nullptr) {
    GDALClose(poDataset); // 关闭 GDAL 数据集句柄
    poDataset = nullptr;
    logMessage(tr("Closed current image dataset."));
  }
  currentImage.release(); // 释放 OpenCV Mat 内存
  currentFilename.clear();

  // 重置 UI 元素到初始状态
  ui->valueFilename->setText(tr("N/A"));
  ui->valueDimensions->setText(tr("N/A"));
  ui->valueDataType->setText(tr("N/A"));

  ui->imageDisplayLabel->clear(); // 清除图像
  ui->imageDisplayLabel->setText(tr("Image Display Area")); // 恢复提示文本
  ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
  ui->imageDisplayLabel->setStyleSheet("QLabel { color: grey; }");


  ui->startAnalysisButton->setEnabled(false); // 禁用分析按钮

  // 清空所有结果文本区域
  ui->overviewResultsTextEdit->clear();
  ui->method1ResultsTextEdit->clear();
  ui->method2ResultsTextEdit->clear();
  ui->method3ResultsTextEdit->clear();
  ui->method4ResultsTextEdit->clear();
  ui->method5ResultsTextEdit->clear();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
      QString filePath = urls.first().toLocalFile();
      QFileInfo fileInfo(filePath);
      QString suffix = fileInfo.suffix().toLower();
      // 定义支持的文件扩展名列表
      QStringList supportedSuffixes = {"tif", "tiff", "img", "hdr", "dat"};
      if (supportedSuffixes.contains(suffix)) {
        event->acceptProposedAction();
        logMessage(tr("Drag entered with supported file: %1").arg(fileInfo.fileName()));
        // 可选：添加视觉反馈，例如改变 QLabel 边框
        // ui->imageDisplayLabel->setStyleSheet("QLabel { border: 2px dashed blue; }");
        return;
      } else {
        logMessage(tr("Drag entered with unsupported file type: .%1").arg(suffix));
      }
    }
  }
  event->ignore(); // 忽略不支持的拖放内容
}

void MainWindow::dropEvent(QDropEvent *event) {
  // 可选：移除拖放进入时的视觉反馈
  // ui->imageDisplayLabel->setStyleSheet("QLabel { color: grey; }"); // 恢复默认样式

  const QMimeData *mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    QList<QUrl> urls = mimeData->urls();
    if (!urls.isEmpty()) {
      QString filePath = urls.first().toLocalFile();
      logMessage(tr("File dropped: %1").arg(filePath));
      openImageFile(filePath); // 调用文件打开处理函数
      event->acceptProposedAction();
      return;
    }
  }
  event->ignore();
}

void MainWindow::openImageFile(const QString &filePath) {
  if (filePath.isEmpty()) {
    logMessage(tr("Image opening cancelled or invalid path provided."));
    return;
  }

  logMessage(tr("Attempting to open image: %1").arg(filePath));
  QCoreApplication::processEvents(); // 允许 UI 更新，显示日志消息

  // 关闭任何已打开的图像
  closeCurrentImage();

  // 使用 GDAL 以只读方式打开文件
  poDataset = (GDALDataset *)GDALOpen(filePath.toUtf8().constData(), GA_ReadOnly); // 使用 UTF-8 编码路径

  if (poDataset == nullptr) {
    QString gdalErrorMsg = CPLGetLastErrorMsg(); // 获取 GDAL 详细错误信息
    logMessage(tr("Error: Could not open image file '%1'. GDAL Error: %2").arg(filePath, gdalErrorMsg));
    QMessageBox::critical(this, tr("GDAL Open Error"),
                          tr("Could not open the selected image file.\nPath: %1\nError: %2\nPlease check file integrity, permissions, and GDAL installation.")
                          .arg(filePath, gdalErrorMsg));
    return;
  }

  currentFilename = QFileInfo(filePath).fileName();
  logMessage(tr("Successfully opened dataset: %1").arg(currentFilename));

  // 更新图像基本信息显示
  updateImageInfo();

  int width = poDataset->GetRasterXSize();
  int height = poDataset->GetRasterYSize();
  int numBands = poDataset->GetRasterCount();

  if (width <= 0 || height <= 0 || numBands < 1) {
    logMessage(tr("Error: Image has invalid dimensions (%1x%2) or zero bands (%3).").arg(width).arg(height).arg(numBands));
    QMessageBox::critical(this, tr("Invalid Image Data"), tr("Image dimensions or band count are invalid."));
    closeCurrentImage();
    return;
  }

  // 仅读取第一个波段，SAR 数据通常是单波段（或复数形式的单波段）
  GDALRasterBand *poBand = poDataset->GetRasterBand(1);
  if (poBand == nullptr) {
    logMessage(tr("Error: Could not access raster band 1."));
    QMessageBox::critical(this, tr("Raster Band Error"), tr("Could not access the first raster band of the image."));
    closeCurrentImage();
    return;
  }

  GDALDataType dataType = poBand->GetRasterDataType();
  logMessage(tr("Image properties: %1x%2 pixels, %3 bands, Data type: %4")
                 .arg(width).arg(height).arg(numBands).arg(GDALGetDataTypeName(dataType)));

  // 将 GDAL 数据类型映射到 OpenCV 类型
  int cvType = -1;
  bool isComplex = GDALDataTypeIsComplex(dataType); // 检查是否为复数类型

  switch (dataType) {
    case GDT_Byte:    cvType = CV_8U; break;
    case GDT_UInt16:  cvType = CV_16U; break;
    case GDT_Int16:   cvType = CV_16S; break;
    case GDT_UInt32:  cvType = CV_32S; break; // 注意：OpenCV 对 CV_32U 支持有限，通常映射到 CV_32S
    case GDT_Int32:   cvType = CV_32S; break;
    case GDT_Float32: cvType = CV_32F; break;
    case GDT_Float64: cvType = CV_64F; break;
    // 复数类型的基础类型
    case GDT_CInt16:   cvType = CV_16S; break;
    case GDT_CInt32:   cvType = CV_32S; break;
    case GDT_CFloat32: cvType = CV_32F; break;
    case GDT_CFloat64: cvType = CV_64F; break;
    default:
      logMessage(tr("Error: Unsupported GDAL data type: %1").arg(GDALGetDataTypeName(dataType)));
      QMessageBox::critical(this, tr("Unsupported Data Type"), tr("The image data type (%1) is not supported.").arg(GDALGetDataTypeName(dataType)));
      closeCurrentImage();
      return;
  }

  // 分配 OpenCV Mat 内存
  // 对于复数，创建双通道 Mat；对于实数，创建单通道 Mat
  int cvChannels = isComplex ? 2 : 1;
  currentImage.create(height, width, CV_MAKETYPE(cvType, cvChannels));

  // 使用 RasterIO 读取数据
  CPLErr err = poBand->RasterIO(
      GF_Read,              // 读取模式
      0, 0,                 // 读取区域的左上角偏移 (x, y)
      width, height,        // 读取区域的尺寸 (宽，高)
      currentImage.ptr(),   // 指向目标缓冲区的指针 (OpenCV Mat 数据区)
      width, height,        // 目标缓冲区的尺寸 (与读取区域相同)
      dataType,             // 要读取的数据类型 (GDAL 类型)
      isComplex ? GDALGetDataTypeSizeBytes(dataType) : 0, // 像素间距 (字节)，0 表示默认
      isComplex ? GDALGetDataTypeSizeBytes(dataType) * width : 0 // 行间距 (字节)，0 表示默认
  );

  if (err != CE_None) {
    logMessage(tr("Error reading raster data using RasterIO. GDAL Error: %1").arg(CPLGetLastErrorMsg()));
    QMessageBox::critical(this, tr("Raster Read Error"), tr("Failed to read image data from the raster band."));
    closeCurrentImage();
    return;
  }

  logMessage(tr("Image data successfully read into cv::Mat. Type: %1, Channels: %2")
                 .arg(cv::typeToString(currentImage.type()))
                 .arg(currentImage.channels()));

  // 准备并显示图像
  if (!currentImage.empty()) {
    cv::Mat imageToDisplay;
    if (isComplex) {
      // 对于复数图像，计算幅度图像用于显示
      logMessage(tr("Calculating magnitude for display from complex data."));
      std::vector<cv::Mat> channels;
      cv::split(currentImage, channels);
      // 幅度计算通常需要浮点精度
      if (channels[0].depth() != CV_32F && channels[0].depth() != CV_64F) {
          logMessage(tr("Converting complex channels to CV_32F before magnitude calculation."));
          channels[0].convertTo(channels[0], CV_32F);
          channels[1].convertTo(channels[1], CV_32F);
      }
      cv::magnitude(channels[0], channels[1], imageToDisplay);
      logMessage(tr("Displaying magnitude image."));
    } else {
      // 对于非复数图像，直接使用
      imageToDisplay = currentImage;
      logMessage(tr("Displaying single-band image."));
    }
    displayImage(imageToDisplay); // 调用显示函数
    ui->startAnalysisButton->setEnabled(true); // 启用分析按钮
  } else {
    logMessage(tr("Error: cv::Mat is unexpectedly empty after reading data."));
    QMessageBox::critical(this, tr("Internal Error"), tr("Failed to prepare image data in memory after reading."));
    closeCurrentImage();
  }
}

void MainWindow::updateImageInfo() {
  if (!poDataset) {
      logMessage(tr("UpdateImageInfo called with no active dataset."));
      return;
  }

  ui->valueFilename->setText(currentFilename);
  int width = poDataset->GetRasterXSize();
  int height = poDataset->GetRasterYSize();
  ui->valueDimensions->setText(QString("%1 x %2").arg(width).arg(height));

  GDALRasterBand *poBand = poDataset->GetRasterBand(1);
  if (poBand) {
    GDALDataType dataType = poBand->GetRasterDataType();
    ui->valueDataType->setText(GDALGetDataTypeName(dataType));
  } else {
    logMessage(tr("Could not get band 1 to update data type info."));
    ui->valueDataType->setText(tr("Error"));
  }
}

void MainWindow::displayImage(const cv::Mat &image) {
  if (image.empty()) {
    logMessage(tr("Attempted to display an empty image."));
    ui->imageDisplayLabel->setText(tr("Error: Image is empty"));
    ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
    ui->imageDisplayLabel->setStyleSheet("QLabel { color: red; }");
    return;
  }

  logMessage(tr("Preparing image for display. Input type: %1, Channels: %2")
                 .arg(cv::typeToString(image.type()))
                 .arg(image.channels()));

  cv::Mat displayMat; // 用于最终显示的 CV_8UC1 图像

  // 确保我们有一个单通道图像用于灰度显示
  cv::Mat singleChannelImage;
  if (image.channels() == 1) {
    singleChannelImage = image;
  } else if (image.channels() > 1) {
    // 如果输入是多通道 (例如复数计算后的幅度图，虽然通常是单通道)，取第一个通道
    // 或者如果未来支持彩色，这里需要不同的逻辑
    logMessage(tr("Input image has %1 channels. Using the first channel for grayscale display.")
                   .arg(image.channels()));
    std::vector<cv::Mat> channels;
    cv::split(image, channels);
    singleChannelImage = channels[0];
  } else {
    logMessage(tr("Error: Input image for display has 0 channels."));
     ui->imageDisplayLabel->setText(tr("Error: Invalid image channels"));
     ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
     ui->imageDisplayLabel->setStyleSheet("QLabel { color: red; }");
    return;
  }

  // 归一化到 8 位灰度图 (0-255) 以便显示
  if (singleChannelImage.depth() == CV_8U) {
    displayMat = singleChannelImage.clone(); // 如果已经是 8U，直接使用
    logMessage(tr("Image is already CV_8U. No normalization needed for display."));
  } else {
    double minVal, maxVal;
    cv::minMaxLoc(singleChannelImage, &minVal, &maxVal);
    logMessage(tr("Normalizing image for display. Original range: [%1, %2]").arg(minVal).arg(maxVal));
    if (maxVal > minVal) {
      // 归一化公式：output = (input - minVal) * (255.0 / (maxVal - minVal))
      // 使用 convertTo 进行类型转换和线性变换
      singleChannelImage.convertTo(displayMat, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    } else {
      // 如果图像值恒定，将其转换为 0 或某个中间灰度值
      singleChannelImage.convertTo(displayMat, CV_8U); // 结果通常是全黑或全白，取决于原值
      logMessage(tr("Image has constant value. Displaying as uniform gray."));
    }
  }

  // 将 CV_8UC1 Mat 转换为 QImage (Format_Grayscale8)
  QImage qimg;
  if (displayMat.channels() == 1 && displayMat.depth() == CV_8U) {
    // 注意 QImage 构造函数需要 data, width, height, bytesPerLine
    qimg = QImage(displayMat.data, displayMat.cols, displayMat.rows, static_cast<int>(displayMat.step), QImage::Format_Grayscale8);
    // 创建一个深拷贝，以防 displayMat 在 QImage 仍在使用时被销毁
    qimg = qimg.copy();
  } else {
    logMessage(tr("Error: Failed to prepare image as CV_8UC1 for QImage conversion. Actual type: %1")
                   .arg(cv::typeToString(displayMat.type())));
     ui->imageDisplayLabel->setText(tr("Error: Display conversion failed"));
     ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
     ui->imageDisplayLabel->setStyleSheet("QLabel { color: red; }");
    return;
  }

  if (qimg.isNull()) {
    logMessage(tr("Error: QImage conversion resulted in a null image."));
    ui->imageDisplayLabel->setText(tr("Error: QImage is null"));
    ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
    ui->imageDisplayLabel->setStyleSheet("QLabel { color: red; }");
    return;
  }

  // 在 QLabel 中显示 QPixmap，并进行缩放以适应 QLabel 尺寸
  QPixmap pixmap = QPixmap::fromImage(qimg);
  ui->imageDisplayLabel->setPixmap(pixmap.scaled(ui->imageDisplayLabel->size(), // 缩放到 QLabel 的当前尺寸
                                                 Qt::KeepAspectRatio,      // 保持宽高比
                                                 Qt::SmoothTransformation)); // 使用平滑滤波进行缩放
  ui->imageDisplayLabel->setAlignment(Qt::AlignCenter); // 确保图像居中
  ui->imageDisplayLabel->setStyleSheet(""); // 清除之前的样式 (如边框或错误颜色)
  logMessage(tr("Image successfully displayed."));
}

void MainWindow::on_startAnalysisButton_clicked() {
  if (currentImage.empty() || poDataset == nullptr) {
    QMessageBox::warning(this, tr("Analysis Not Started"), tr("Please open an image file before starting the analysis."));
    logMessage(tr("Analysis button clicked, but no image is loaded."));
    return;
  }

  logMessage(tr("Analysis process started by user."));
  ui->progressBar->setValue(0); // 重置进度条

  // 清空之前的分析结果显示区域
  ui->overviewResultsTextEdit->clear();
  ui->method1ResultsTextEdit->clear();
  ui->method2ResultsTextEdit->clear();
  ui->method3ResultsTextEdit->clear();
  ui->method4ResultsTextEdit->clear();
  ui->method5ResultsTextEdit->clear();

  // 统计选中的分析任务数量
  int totalSteps = 0;
  std::vector<std::function<void()>> analysisTasks; // 存储要执行的任务
  std::vector<QWidget*> resultTabs; // 存储对应的结果标签页

  if (ui->checkBoxSNR->isChecked()) {
      totalSteps++;
      analysisTasks.push_back([this](){ this->performSNRAnalysis(); });
      resultTabs.push_back(ui->tabMethod1);
  }
  if (ui->checkBoxInfoContent->isChecked()) {
      totalSteps++;
      analysisTasks.push_back([this](){ this->performInfoContentAnalysis(); });
      resultTabs.push_back(ui->tabMethod2);
  }
  if (ui->checkBoxClarity->isChecked()) {
      totalSteps++;
      analysisTasks.push_back([this](){ this->performClarityAnalysis(); });
      resultTabs.push_back(ui->tabMethod3);
  }
  if (ui->checkBoxRadiometricAccuracy->isChecked()) {
      totalSteps++;
      analysisTasks.push_back([this](){ this->performRadiometricAnalysis(); });
      resultTabs.push_back(ui->tabMethod4);
  }
  if (ui->checkBoxGLCM->isChecked()) {
      totalSteps++;
      analysisTasks.push_back([this](){ this->performGLCMAnalysis(); });
      resultTabs.push_back(ui->tabMethod5);
  }

  if (totalSteps == 0) {
    QMessageBox::information(this, tr("No Analysis Selected"), tr("Please select at least one analysis method using the checkboxes."));
    logMessage(tr("Analysis stopped: No methods were selected."));
    return;
  }

  ui->progressBar->setMaximum(totalSteps); // 设置进度条的最大值
  ui->overviewResultsTextEdit->setText(tr("Starting selected analyses...\n")); // 初始总览文本

  // 依次执行选定的分析任务
  int currentStep = 0;
  for (size_t i = 0; i < analysisTasks.size(); ++i) {
      logMessage(tr("Performing analysis step %1 of %2...").arg(currentStep + 1).arg(totalSteps));
      analysisTasks[i](); // 执行分析函数
      currentStep++;
      ui->progressBar->setValue(currentStep); // 更新进度条
      if (resultTabs[i]) {
          ui->resultsTabWidget->setCurrentWidget(resultTabs[i]); // 切换到对应的结果标签页
      }
      QCoreApplication::processEvents(); // 强制处理事件循环，确保 UI 更新和响应
  }

  logMessage(tr("All selected analyses finished."));
  // 在总览文本末尾添加完成提示
  ui->overviewResultsTextEdit->append(tr("\nAnalysis complete. Check individual tabs for detailed results."));
  ui->resultsTabWidget->setCurrentWidget(ui->tabOverview); // 最后切换回总览标签页

  QMessageBox::information(this, tr("Analysis Complete"),
      tr("Selected image analyses have finished. You can view the detailed results in the corresponding tabs."));
}

void MainWindow::on_checkBoxSelectAll_toggled(bool checked) {
  // 根据 "Select All" 复选框的状态，统一设置所有分析方法的复选框
  ui->checkBoxSNR->setChecked(checked);
  ui->checkBoxInfoContent->setChecked(checked);
  ui->checkBoxClarity->setChecked(checked);
  ui->checkBoxRadiometricAccuracy->setChecked(checked);
  ui->checkBoxGLCM->setChecked(checked);
  logMessage(checked ? tr("Selected all analysis methods.") : tr("Deselected all analysis methods."));
}


// --- 分析方法实现已移至 analysis_*.cpp 文件 ---
// 无需在此处保留这些函数的空实现或注释


// --- 菜单项 "Open Image" 的槽函数实现 ---
void MainWindow::on_actionOpenImage_triggered() {
  // 打开标准文件对话框让用户选择图像
  QString filePath = QFileDialog::getOpenFileName(
      this,
      tr("Open SAR Image File"), // 对话框标题
      QString(), // 初始目录 (空表示上次使用的或默认)
      tr("Supported Image Formats (*.tif *.tiff *.img *.hdr *.dat);;All Files (*.*)") // 文件类型过滤器
  );

  if (!filePath.isEmpty()) {
    logMessage(tr("Opening image via File menu: %1").arg(filePath));
    openImageFile(filePath); // 调用通用的文件打开函数
  } else {
    logMessage(tr("Image opening via File menu cancelled by user."));
  }
}
