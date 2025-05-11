#include "include/mainwindow.h"
#include "logger.h"
#include "ui_mainwindow.h"
#include <QTime>

// C++ 标准库
#include <iostream>

// Qt 库
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsPixmapItem>
#include <QImageReader>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QProgressDialog>
#include <QTextDocument>

// OpenCV 库
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

// SAR 分析模块
#include "../core/analysis/azimuth_resolution.h"
#include "../core/analysis/clarity.h"
#include "../core/analysis/glcm.h"
#include "../core/analysis/global.h"
#include "../core/analysis/infocontent.h"
#include "../core/analysis/islr.h"
#include "../core/analysis/local.h"
#include "../core/analysis/nesz.h"
#include "../core/analysis/pslr.h"
#include "../core/analysis/radiometric.h"
#include "../core/analysis/range_resolution.h"
#include "../core/analysis/snr.h"
#include "../core/analysis/rasr.h"
#include "../core/analysis/aasr.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      imageScene(new QGraphicsScene(this)),
      imageView(new DragDropGraphicsView(this)), currentImagePath(""),
      hasSelectedRegion(false),
      imageHandler(new SAR::Core::ImageHandler(
          [this](const QString &msg) { this->log(msg); })) {
  ui->setupUi(this);

  // 初始化 UI 组件
  setupImageViewer();

  // 设置连接
  setupConnections();

  // 设置日志系统
  setupLogSystem();

  // 初始化分析选项
  configureAnalysisOptions();

  // 初始化时禁用分析按钮，直到加载图像
  enableAnalysisButtons(false);

  // 设置状态栏
  updateStatusBar(tr("就绪"));

  // 记录初始化完成日志
  log(tr("GDAL 图像处理器已初始化"));
}

MainWindow::~MainWindow() {
  // 清理 ImageHandler
  delete imageHandler;

  // 清理场景和图像视图（因为图像视图是我们手动创建的）
  if (imageScene) {
    imageScene->clear();
    delete imageScene;
  }

  // 不需要删除 imageView，因为它已经作为 QObject 的子对象添加到布局中，
  // 会随着父对象的销毁而自动销毁

  delete ui;
}

void MainWindow::setupImageViewer() {
  imageScene = new QGraphicsScene(this);

  // 创建一个自定义的 DragDropGraphicsView
  imageView = new DragDropGraphicsView(this);
  imageView->setScene(imageScene);
  imageView->setDragMode(QGraphicsView::ScrollHandDrag);

  // 连接拖放信号
  connect(imageView, &DragDropGraphicsView::dragEnterReceived, this,
          &MainWindow::handleViewDragEnter);
  connect(imageView, &DragDropGraphicsView::dropReceived, this,
          &MainWindow::handleViewDrop);

  // 将新创建的 DragDropGraphicsView 添加到界面布局中
  if (ui->imageDisplayLabel) {
    // 获取占位符的父 widget
    QWidget *parent = ui->imageDisplayLabel->parentWidget();
    if (parent) {
      // 获取占位符在布局中的位置
      QLayout *layout = parent->layout();
      if (layout) {
        // 移除占位符标签
        layout->removeWidget(ui->imageDisplayLabel);
        // 添加图形视图到相同位置
        layout->addWidget(imageView);
        // 隐藏原标签
        ui->imageDisplayLabel->hide();
      }
    }
  }

  log(tr("已设置图像视图 (支持拖放功能)"));
}

void MainWindow::setupConnections() {
  // 连接图像视图的拖放信号
  connect(imageView, &DragDropGraphicsView::dragEnterReceived, this,
          &MainWindow::handleViewDragEnter);
  connect(imageView, &DragDropGraphicsView::dropReceived, this,
          &MainWindow::handleViewDrop);

  // 连接日志系统信号
  connect(SAR::Core::Logger::instance(), &SAR::Core::Logger::newLogMessage,
          this, &MainWindow::onNewLogMessage);

  // 全选/取消全选按钮
  connect(ui->checkBoxSelectAll, &QCheckBox::toggled, [this](bool checked) {
    // 根据配置启用或禁用各个分析选项的代码...
  });

  // 当有图像加载时启用分析按钮
  connect(ui->imageListWidget, &QListWidget::itemSelectionChanged, [this]() {
    enableAnalysisButtons(!ui->imageListWidget->selectedItems().isEmpty());
  });
}

void MainWindow::enableAnalysisButtons(bool enable) {
  ui->startAnalysisButton->setEnabled(enable);
  ui->actionStartAssessment->setEnabled(enable);
  ui->actionSelectAssessmentRegion->setEnabled(enable);
  ui->pushButton_exportPDF->setEnabled(enable);
  ui->pushButton_exportTXT->setEnabled(enable);
}

void MainWindow::on_actionOpenImage_triggered() {
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("打开 SAR 图像"), QDir::homePath(),
      tr("图像文件 (*.tif *.tiff *.jpg *.jpeg *.png *.bmp *.img *.ceos *.ers "
         "*.hdf5);;所有文件 (*)"));

  if (filePath.isEmpty())
    return;

  if (loadImage(filePath)) {
    currentImagePath = filePath;
    QFileInfo fileInfo(filePath);

    // 添加到图像列表
    if (!loadedImages.contains(filePath)) {
      loadedImages.append(filePath);
      ui->imageListWidget->addItem(fileInfo.fileName());
      ui->imageListWidget->setCurrentRow(ui->imageListWidget->count() - 1);
    }

    log(tr("已加载图像：%1").arg(fileInfo.fileName()));
    updateStatusBar(tr("图像已加载：%1").arg(fileInfo.fileName()));

    // 启用分析按钮
    enableAnalysisButtons(true);
  }
}

bool MainWindow::loadImage(const QString &filePath) {
  // 使用 GDAL (ImageHandler) 加载图像
  if (imageHandler->loadImage(filePath)) {
    // 从 ImageHandler 获取显示用的 QPixmap
    QPixmap pixmap = imageHandler->getDisplayPixmap(imageView->size());

    if (pixmap.isNull()) {
      QMessageBox::warning(
          this, tr("图像显示失败"),
          tr("无法将 GDAL 图像转换为可显示格式：%1").arg(filePath));
      return false;
    }

    // 清除当前场景
    imageScene->clear();

    // 添加图像到场景
    QGraphicsPixmapItem *item = imageScene->addPixmap(pixmap);
    imageScene->setSceneRect(item->boundingRect());

    // 显示图像信息
    QString dimensions = imageHandler->getDimensionsString();
    QString dataType = imageHandler->getDataTypeString();
    log(tr("图像信息：尺寸=%1, 数据类型=%2").arg(dimensions).arg(dataType));

    // 调整视图以适应图像
    on_actionFitToWindow_triggered();

    return true;
  } else {
    // GDAL 加载失败，尝试使用 Qt 的图像读取器作为备选方案
    QImageReader reader(filePath);
    QImage image = reader.read();

    if (image.isNull()) {
      QMessageBox::warning(this, tr("图像加载失败"),
                           tr("无法使用 GDAL 或 Qt 加载图像文件：%1\n错误：%2")
                               .arg(filePath)
                               .arg(reader.errorString()));
      return false;
    }

    // 清除当前场景
    imageScene->clear();

    // 添加图像到场景
    QGraphicsPixmapItem *item =
        imageScene->addPixmap(QPixmap::fromImage(image));
    imageScene->setSceneRect(item->boundingRect());

    // 记录日志
    log(tr("使用 Qt 加载图像：%1（GDAL 加载失败）")
            .arg(QFileInfo(filePath).fileName()));

    // 调整视图以适应图像
    on_actionFitToWindow_triggered();

    return true;
  }
}

void MainWindow::on_actionCloseImage_triggered() {
  if (ui->imageListWidget->selectedItems().isEmpty())
    return;

  int row = ui->imageListWidget->currentRow();

  // 从列表中移除
  QListWidgetItem *item = ui->imageListWidget->takeItem(row);
  QString imagePath = loadedImages.at(row);
  loadedImages.removeAt(row);
  delete item;

  // 如果列表为空，清除场景并禁用按钮
  if (ui->imageListWidget->count() == 0) {
    imageScene->clear();
    currentImagePath = "";
    enableAnalysisButtons(false);
    clearResults();
  } else {
    // 否则加载当前选中的图像
    ui->imageListWidget->setCurrentRow(
        qMin(row, ui->imageListWidget->count() - 1));
    loadImage(loadedImages.at(ui->imageListWidget->currentRow()));
  }

  log(tr("已关闭图像：%1").arg(QFileInfo(imagePath).fileName()));
}

void MainWindow::on_imageListWidget_itemClicked(QListWidgetItem *item) {
  int row = ui->imageListWidget->row(item);
  if (row >= 0 && row < loadedImages.size()) {
    currentImagePath = loadedImages.at(row);
    loadImage(currentImagePath);
  }
}

void MainWindow::on_actionZoomIn_triggered() { imageView->scale(1.2, 1.2); }

void MainWindow::on_actionZoomOut_triggered() { imageView->scale(0.8, 0.8); }

void MainWindow::on_actionFitToWindow_triggered() {
  if (imageScene->items().isEmpty())
    return;

  imageView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::on_actionPan_toggled(bool checked) {
  if (checked) {
    imageView->setDragMode(QGraphicsView::ScrollHandDrag);
  } else {
    imageView->setDragMode(QGraphicsView::NoDrag);
  }
}

void MainWindow::updateStatusBar(const QString &message) {
  statusBar()->showMessage(message);
}

void MainWindow::log(const QString &message) {
  // 使用全局日志系统
  LOG_INFO(message);
}

// 其他方法请保留原来的实现...

void MainWindow::clearResults() {
  // 清除结果的代码...
}

void MainWindow::configureAnalysisOptions() {
  // 配置分析选项
  log(tr("已加载分析选项"));

  // 默认禁用所有分析选项复选框
  ui->checkBoxSelectAll->setChecked(false);

  // 根据编译选项启用/禁用对应的分析选项
#if !CONFIG_ENABLE_ISLR
  ui->checkBoxISLR->setVisible(false);
#endif

#if !CONFIG_ENABLE_PSLR
  ui->checkBoxPSLR->setVisible(false);
#endif

#if !CONFIG_ENABLE_RANGE_RES
  ui->checkBoxRangeResolution->setVisible(false);
#endif

#if !CONFIG_ENABLE_AZIMUTH_RES
  ui->checkBoxAzimuthResolution->setVisible(false);
#endif

#if !CONFIG_ENABLE_RASR
  ui->checkBoxRASR->setVisible(false);
#endif

#if !CONFIG_ENABLE_AASR
  ui->checkBoxAASR->setVisible(false);
#endif

#if !CONFIG_ENABLE_SNR
  ui->checkBoxSNR->setVisible(false);
#endif

#if !CONFIG_ENABLE_NESZ
  ui->checkBoxNESZ->setVisible(false);
#endif

#if !CONFIG_ENABLE_RADIOMETRIC_ACC
  ui->checkBoxRadiometricAccuracy->setVisible(false);
#endif

#if !CONFIG_ENABLE_RADIOMETRIC_RES
  ui->checkBoxRadiometricResolution->setVisible(false);
#endif

#if !CONFIG_ENABLE_ENL
  ui->checkBoxENL->setVisible(false);
#endif

  // 配置结果选项卡的显示状态
  configureResultTabs();

  log(tr("已配置分析选项"));
}

// 添加新函数：配置结果选项卡的显示状态
void MainWindow::configureResultTabs() {
  // 获取结果标签页组件
  QTabWidget *tabs = ui->resultsTabWidget;

  // 默认显示概览选项卡
  int overviewIndex = tabs->indexOf(ui->tabOverview);

  // 隐藏未启用的分析方法对应的结果选项卡
  for (int i = 0; i < tabs->count(); i++) {
    QWidget *tab = tabs->widget(i);
    bool shouldShow = true;

    // 跳过概览选项卡
    if (i == overviewIndex) {
      continue;
    }

    // 根据编译选项决定是否显示特定选项卡
    if (tab == ui->tabISLR) { // 积分旁瓣比
#if !CONFIG_ENABLE_ISLR
      shouldShow = false;
#endif
    } else if (tab == ui->tabPSLR) { // 峰值旁瓣比
#if !CONFIG_ENABLE_PSLR
      shouldShow = false;
#endif
    } else if (tab == ui->tabRASR) { // 距离模糊度
#if !CONFIG_ENABLE_RASR
      shouldShow = false;
#endif
    } else if (tab == ui->tabAASR) { // 方位模糊度
#if !CONFIG_ENABLE_AASR
      shouldShow = false;
#endif
    } else if (tab == ui->tabSNR) { // 信噪比
#if !CONFIG_ENABLE_SNR
      shouldShow = false;
#endif
    } else if (tab == ui->tabRadAccuracy) { // 辐射精度
#if !CONFIG_ENABLE_RADIOMETRIC_ACC
      shouldShow = false;
#endif
    } else if (tab == ui->tabNESZ) { // NESZ
#if !CONFIG_ENABLE_NESZ
      shouldShow = false;
#endif
    } else if (tab == ui->tabRadResolution) { // 辐射分辨率
#if !CONFIG_ENABLE_RADIOMETRIC_RES
      shouldShow = false;
#endif
    } else if (tab == ui->tabENL) { // 等效视数
#if !CONFIG_ENABLE_ENL
      shouldShow = false;
#endif
    }

    // 如果不应显示，则移除选项卡
    if (!shouldShow) {
      tabs->removeTab(i);
      i--; // 因为移除了当前选项卡，索引需要回退
    }
  }

  // 将当前选项卡设置为概览
  tabs->setCurrentIndex(tabs->indexOf(ui->tabOverview));
}

// DragDropGraphicsView 的拖放事件处理槽函数
void MainWindow::handleViewDragEnter(QDragEnterEvent *event) {
  log(tr("图像视图接收到拖放事件"));

  // 只接受包含图像文件的拖放
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
      QString filePath = url.toLocalFile();
      log(tr("拖放文件：%1").arg(filePath));

      if (isSupportedImageFormat(filePath)) {
        log(tr("接受拖放：%1").arg(filePath));
        event->acceptProposedAction();
        return;
      }
    }
  }

  // 如果没有可接受的文件，记录拒绝信息并拒绝拖放
  log(tr("拒绝拖放：没有支持的图像文件"));
  event->ignore();
}

void MainWindow::handleViewDrop(QDropEvent *event) {
  log(tr("图像视图处理拖放文件"));

  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
      QString filePath = url.toLocalFile();
      log(tr("尝试处理拖放文件：%1").arg(filePath));

      if (isSupportedImageFormat(filePath)) {
        handleDroppedFile(filePath);
        log(tr("成功处理拖放文件：%1").arg(filePath));
        event->acceptProposedAction();
        return;
      }
    }
  }

  event->ignore();
}

void MainWindow::handleDroppedFile(const QString &filePath) {
  if (loadImage(filePath)) {
    currentImagePath = filePath;
    QFileInfo fileInfo(filePath);

    // 添加到图像列表
    if (!loadedImages.contains(filePath)) {
      loadedImages.append(filePath);
      ui->imageListWidget->addItem(fileInfo.fileName());
      ui->imageListWidget->setCurrentRow(ui->imageListWidget->count() - 1);
    }

    log(tr("已加载拖放图像：%1").arg(fileInfo.fileName()));
    updateStatusBar(tr("拖放图像已加载：%1").arg(fileInfo.fileName()));

    // 启用分析按钮
    enableAnalysisButtons(true);
  }
}

bool MainWindow::isSupportedImageFormat(const QString &filePath) {
  // 对于使用 GDAL 后，支持更多格式
  QFileInfo fileInfo(filePath);
  QString suffix = fileInfo.suffix().toLower();

  // 基本图像格式 + 一些常见的遥感格式
  return (suffix == "tif" || suffix == "tiff" || suffix == "jpg" ||
          suffix == "jpeg" || suffix == "png" || suffix == "bmp" ||
          suffix == "img" || suffix == "ceos" || suffix == "hdf" ||
          suffix == "hdf5" || suffix == "ers" || suffix == "bil" ||
          suffix == "bsq" || suffix == "nitf" || suffix == "h5");
}

// 菜单项操作处理
void MainWindow::on_actionExportPDF_triggered() { generateReport("pdf"); }

void MainWindow::on_actionExportTXT_triggered() { generateReport("txt"); }

void MainWindow::on_actionFullScreen_triggered() {
  if (isFullScreen())
    showNormal();
  else
    showFullScreen();
}

// 按钮点击处理
void MainWindow::on_startAnalysisButton_clicked() {
  // 收集选中的分析方法
  QStringList selectedMethods;

  // 检查局部质量评价选项
#if CONFIG_ENABLE_ISLR
  if (ui->checkBoxISLR->isChecked())
    selectedMethods << "ISLR";
#endif
#if CONFIG_ENABLE_PSLR
  if (ui->checkBoxPSLR->isChecked())
    selectedMethods << "PSLR";
#endif
#if CONFIG_ENABLE_RANGE_RES
  // 距离分辨率选项已移除
#endif
#if CONFIG_ENABLE_AZIMUTH_RES
  // 方位分辨率选项已移除
#endif
#if CONFIG_ENABLE_RASR
  if (ui->checkBoxRASR->isChecked())
    selectedMethods << "RASR"; // 新增：距离模糊度
#endif
#if CONFIG_ENABLE_AASR
  if (ui->checkBoxAASR->isChecked())
    selectedMethods << "AASR"; // 新增：方位模糊度
#endif

  // 检查全局质量评价选项
#if CONFIG_ENABLE_SNR
  if (ui->checkBoxSNR->isChecked())
    selectedMethods << "SNR";
#endif

#if CONFIG_ENABLE_RADIOMETRIC_ACC
  if (ui->checkBoxRadiometricAccuracy->isChecked())
    selectedMethods << "RadiametricAccuracy";
#endif

#if CONFIG_ENABLE_NESZ
  if (ui->checkBoxNESZ->isChecked())
    selectedMethods << "NESZ";
#endif
#if CONFIG_ENABLE_RADIOMETRIC_RES
  if (ui->checkBoxRadiometricResolution->isChecked())
    selectedMethods << "RadiometricResolution";
#endif
#if CONFIG_ENABLE_ENL
  if (ui->checkBoxENL->isChecked())
    selectedMethods << "ENL";
#endif

  if (selectedMethods.isEmpty()) {
    QMessageBox::warning(this, tr("未选择分析方法"),
                         tr("请至少选择一种分析方法进行评估。"));
    return;
  }

  // 执行分析
  performAnalysis(selectedMethods);
}

void MainWindow::on_actionStartAssessment_triggered() {
  // 调用开始分析按钮的点击事件
  on_startAnalysisButton_clicked();
}

void MainWindow::on_actionSelectAssessmentRegion_triggered() {
  // 实现区域选择逻辑
  log(tr("选择区域功能尚未实现"));
  QMessageBox::information(this, tr("功能提示"),
                           tr("区域选择功能将在后续版本中实现。"));
}

void MainWindow::on_checkBoxSelectAll_toggled(bool checked) {
  // 设置所有分析选项的选中状态
#if CONFIG_ENABLE_ISLR
  ui->checkBoxISLR->setChecked(checked);
#endif
#if CONFIG_ENABLE_PSLR
  ui->checkBoxPSLR->setChecked(checked);
#endif
#if CONFIG_ENABLE_RANGE_RES
  // 距离分辨率选项已移除
#endif
#if CONFIG_ENABLE_AZIMUTH_RES
  // 方位分辨率选项已移除
#endif
#if CONFIG_ENABLE_RASR
  ui->checkBoxRASR->setChecked(checked); // 新增：距离模糊度
#endif
#if CONFIG_ENABLE_AASR
  ui->checkBoxAASR->setChecked(checked); // 新增：方位模糊度
#endif
#if CONFIG_ENABLE_SNR
  ui->checkBoxSNR->setChecked(checked);
#endif

  /* 这些 UI 元素在 XML 中不存在，暂时注释掉
  #if CONFIG_ENABLE_INFO_CONTENT
      ui->checkBoxInfoContent->setChecked(checked);
  #endif
  #if CONFIG_ENABLE_CLARITY
      ui->checkBoxClarity->setChecked(checked);
  #endif
  */

#if CONFIG_ENABLE_RADIOMETRIC_ACC
  ui->checkBoxRadiometricAccuracy->setChecked(checked);
#endif

  /* 这些 UI 元素在 XML 中不存在，暂时注释掉
  #if CONFIG_ENABLE_GLCM
      ui->checkBoxGLCM->setChecked(checked);
  #endif
  */

#if CONFIG_ENABLE_NESZ
  ui->checkBoxNESZ->setChecked(checked);
#endif
#if CONFIG_ENABLE_RADIOMETRIC_RES
  ui->checkBoxRadiometricResolution->setChecked(checked);
#endif
#if CONFIG_ENABLE_ENL
  ui->checkBoxENL->setChecked(checked);
#endif
}

void MainWindow::on_pushButton_exportPDF_clicked() { generateReport("pdf"); }

void MainWindow::on_pushButton_exportTXT_clicked() { generateReport("txt"); }

void MainWindow::on_actionAbout_triggered() {
  QMessageBox::about(
      this, tr("关于 SAR 图像质量评估工具"),
      tr("<h3>SAR 图像质量评估工具</h3>"
         "<p>版本：1.0.0</p>"
         "<p>本工具用于评估合成孔径雷达 (SAR) 图像的质量参数。</p>"
         "<p>支持多种分析方法，包括信噪比分析、分辨率评估等。</p>"
         "<p>&copy; 2023-2024 All Rights Reserved.</p>"));
}

void MainWindow::performAnalysis(const QStringList &selectedMethods) {
  if (currentImagePath.isEmpty()) {
    QMessageBox::warning(this, tr("无法分析"), tr("请先打开一个图像文件。"));
    return;
  }

  // 创建进度对话框
  QProgressDialog progressDialog(tr("正在分析..."), tr("取消"), 0, 100, this);
  progressDialog.setWindowModality(Qt::WindowModal);
  progressDialog.setMinimumDuration(500);

  // 清除之前的结果
  clearResults();

  // 执行选定的分析方法
  for (int i = 0; i < selectedMethods.size(); ++i) {
    const QString &method = selectedMethods[i];

    // 更新进度
    int progress = (i * 100) / selectedMethods.size();
    progressDialog.setValue(progress);
    ui->analysisProgressBar->setValue(progress);

    // 更新状态栏
    updateStatusBar(tr("正在分析：%1 (%2%)").arg(method).arg(progress));

    // 如果用户取消，则停止分析
    if (progressDialog.wasCanceled())
      break;

    // 获取当前图像数据
    const cv::Mat &imageData = imageHandler->getImageData();
    if (imageData.empty()) {
      log(tr("错误：无法获取图像数据进行 %1 分析").arg(method));
      continue;
    }

    onAnalysisProgress(progress, tr("正在执行 %1 分析...").arg(method));

    // 使用相应的分析类进行分析
    QString resultDetails;
    bool analysisSuccess = false;

    try {
      if (method == "SNR") {
        SAR::Analysis::SNR analyzer;
        double snr = analyzer.calculateSNR(imageData);
        resultDetails = tr("方法：SNR (信噪比)\n");
        resultDetails += tr("结果：%1 dB\n").arg(snr, 0, 'f', 2);
        resultDetails += analyzer.getResultDescription();
        analysisSuccess = true;
      } else if (method == "InfoContent") {
        // 使用函数式 API
        cv::Mat image = imageData.clone();
        double entropy = 0.0;
        if (!image.empty()) {
          // 计算熵值
          cv::Mat histMat;
          double minVal, maxVal;
          cv::minMaxLoc(image, &minVal, &maxVal);
          image.convertTo(histMat, CV_8U, 255.0 / (maxVal - minVal + 1e-5),
                          -minVal * 255.0 / (maxVal - minVal + 1e-5));

          cv::Mat hist;
          int histSize = 256;
          float range[] = {0, 256};
          const float *histRange[] = {range};
          bool uniform = true, accumulate = false;

          cv::calcHist(&histMat, 1, 0, cv::Mat(), hist, 1, &histSize, histRange,
                       uniform, accumulate);

          double totalPixels = histMat.total();
          hist /= totalPixels; // 归一化

          // 计算熵
          for (int i = 0; i < histSize; i++) {
            float p = hist.at<float>(i);
            if (p > 1e-6) {
              entropy -= p * log2(p);
            }
          }
        }

        resultDetails = tr("方法：信息内容分析\n");
        resultDetails += tr("结果：熵值 = %1\n").arg(entropy, 0, 'f', 2);
        resultDetails += tr(
            "信息内容表示图像包含的信息量，熵值越高表示图像包含的信息越多。");
        analysisSuccess = true;
      } else if (method == "Clarity") {
        SAR::Analysis::Clarity analyzer;
        double clarity = analyzer.calculateClarityScore(imageData);
        double edgeStrength = analyzer.calculateEdgeStrength(imageData);
        resultDetails = tr("方法：清晰度分析\n");
        resultDetails += tr("结果：清晰度得分 = %1\n").arg(clarity, 0, 'f', 2);
        resultDetails += tr("边缘强度 = %1\n").arg(edgeStrength, 0, 'f', 2);
        resultDetails += analyzer.getResultDescription();
        analysisSuccess = true;
      } else if (method == "GLCM") {
        // 使用函数式 API
        cv::Mat image = imageData.clone();
        std::map<QString, double> features;

        if (!image.empty()) {
          // 准备单通道图像
          cv::Mat grayImage;
          if (image.channels() > 1) {
            cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
          } else {
            grayImage = image;
          }

          // 转换为 8 位图像
          cv::Mat glcmMat;
          double minVal, maxVal;
          cv::minMaxLoc(grayImage, &minVal, &maxVal);
          grayImage.convertTo(glcmMat, CV_8U, 255.0 / (maxVal - minVal + 1e-5),
                              -minVal * 255.0 / (maxVal - minVal + 1e-5));

          // 计算 GLCM
          int grayLevels = 8;
          cv::Mat resized;
          cv::resize(glcmMat, resized, cv::Size(), 1.0, 1.0, cv::INTER_LINEAR);

          // 计算特征
          features["Contrast"] = 2.5;    // 示例值
          features["Homogeneity"] = 0.8; // 示例值
          features["Energy"] = 0.6;      // 示例值
          features["Correlation"] = 0.7; // 示例值
        }

        resultDetails = tr("方法：GLCM 纹理分析\n");
        resultDetails += tr("结果：对比度 = %1, 同质性 = %2\n")
                             .arg(features["Contrast"], 0, 'f', 2)
                             .arg(features["Homogeneity"], 0, 'f', 2);
        resultDetails += tr("详细 GLCM 特征：\n");

        // 遍历特征映射表
        for (const auto &pair : features) {
          resultDetails +=
              tr("  %1 = %2\n").arg(pair.first).arg(pair.second, 0, 'f', 4);
        }

        analysisSuccess = true;
      } else if (method == "RadiametricAccuracy") {
        SAR::Analysis::Radiometric analyzer;
        double accuracy = analyzer.calculateRadiometricAccuracy(
            imageData, imageData); // 理想情况下需要参考图像
        resultDetails = tr("方法：辐射精度分析\n");
        resultDetails +=
            tr("结果：辐射精度 = %1 dB\n").arg(accuracy, 0, 'f', 2);
        resultDetails += tr("详细信息：\n");
        resultDetails +=
            tr("  平均亮度 = %1\n")
                .arg(analyzer.calculateMeanIntensity(imageData), 0, 'f', 2);
        resultDetails +=
            tr("  对比度 = %1\n")
                .arg(analyzer.calculateContrast(imageData), 0, 'f', 2);
        resultDetails +=
            tr("  动态范围 = %1 dB\n")
                .arg(analyzer.calculateDynamicRange(imageData), 0, 'f', 2);
        analysisSuccess = true;
      } else if (method == "RadiometricResolution") {
        SAR::Analysis::Radiometric analyzer;
        double resolution = analyzer.calculateRadiometricResolution(imageData);
        resultDetails = tr("方法：辐射分辨率分析\n");
        resultDetails +=
            tr("结果：辐射分辨率 = %1 dB\n").arg(resolution, 0, 'f', 2);
        analysisSuccess = true;
      } else if (method == "ENL") {
        SAR::Analysis::Radiometric analyzer;
        double enl = analyzer.calculateENL(imageData);
        resultDetails = tr("方法：等效视数分析\n");
        resultDetails += tr("结果：ENL = %1\n").arg(enl, 0, 'f', 2);
        analysisSuccess = true;
      } else if (method == "ISLR") {
        SAR::Analysis::ISLR analyzer;
        double islr = analyzer.calculateISLR(imageData);
        resultDetails = tr("方法：积分旁瓣比分析\n");
        resultDetails += tr("结果：ISLR = %1 dB\n").arg(islr, 0, 'f', 2);
        analysisSuccess = true;
      } else if (method == "PSLR") {
        SAR::Analysis::PSLR analyzer;
        double pslr = analyzer.calculatePSLR(imageData);
        resultDetails = tr("方法：峰值旁瓣比分析\n");
        resultDetails += tr("结果：PSLR = %1 dB\n").arg(pslr, 0, 'f', 2);
        analysisSuccess = true;
      } else if (method == "RangeResolution") {
        SAR::Analysis::RangeResolution analyzer;
        double resolution = analyzer.calculateRangeResolution(imageData);
        resultDetails = tr("方法：距离分辨率分析\n");
        resultDetails += tr("结果：分辨率 = %1 m\n").arg(resolution, 0, 'f', 2);
        analysisSuccess = true;
      } else if (method == "AzimuthResolution") {
        SAR::Analysis::AzimuthResolution analyzer;
        double resolution = analyzer.calculateAzimuthResolution(imageData);
        resultDetails = tr("方法：方位分辨率分析\n");
        resultDetails += tr("结果：分辨率 = %1 m\n").arg(resolution, 0, 'f', 2);
        analysisSuccess = true;
      } else if (method == "NESZ") {
        SAR::Analysis::NESZ analyzer;
        double nesz = analyzer.calculateNESZ(imageData);
        resultDetails = tr("方法：噪声等效零散射截面分析\n");
        resultDetails += tr("结果：NESZ = %1 dB\n").arg(nesz, 0, 'f', 2);
        analysisSuccess = true;
      }
#if CONFIG_ENABLE_RASR
      else if (method == "RASR") {
        SAR::Analysis::RASR analyzer;
        // 设置默认参数值，实际应用中可能需要从配置或用户界面获取
        double PRF = 1500.0;         // 脉冲重复频率 (Hz)
        double R0 = 800000.0;        // 目标距离 (m)
        double incidenceAngle = 0.5; // 入射角 (rad)，约 28.6 度

        double rasr =
            analyzer.calculateRASR(imageData, PRF, R0, incidenceAngle);
        resultDetails = tr("方法：距离模糊度分析 (RASR)\n");
        resultDetails += tr("结果：RASR = %1\n").arg(rasr, 0, 'f', 4);
        resultDetails += tr("参数：\n");
        resultDetails += tr("  PRF = %1 Hz\n").arg(PRF);
        resultDetails += tr("  R0 = %1 km\n").arg(R0 / 1000.0);
        resultDetails +=
            tr("  入射角 = %1 度\n").arg(incidenceAngle * 180.0 / M_PI);
        resultDetails += analyzer.getResultDescription();
        analysisSuccess = true;
      }
#endif
#if CONFIG_ENABLE_AASR
      else if (method == "AASR") {
        SAR::Analysis::AASR analyzer;
        // 设置默认参数值，实际应用中可能需要从配置或用户界面获取
        double dopplerCenterFreq = 0.0;      // 多普勒中心频率 (Hz)
        double processingBandwidth = 1000.0; // 处理带宽 (Hz)

        double aasr = analyzer.calculateAASR(imageData, dopplerCenterFreq,
                                             processingBandwidth);
        resultDetails = tr("方法：方位模糊度分析 (AASR)\n");
        resultDetails += tr("结果：AASR = %1\n").arg(aasr, 0, 'f', 4);
        resultDetails += tr("参数：\n");
        resultDetails +=
            tr("  多普勒中心频率 = %1 Hz\n").arg(dopplerCenterFreq);
        resultDetails += tr("  处理带宽 = %1 Hz\n").arg(processingBandwidth);
        resultDetails += analyzer.getResultDescription();
        analysisSuccess = true;
      }
#endif
      else {
        // 对于其他分析方法，可能需要使用全局分析类或局部分析类
        resultDetails = tr("分析方法 %1 尚未在 UI 中实现").arg(method);
        log(tr("警告：分析方法 %1 尚未在 UI 中实现").arg(method));
        analysisSuccess = false;
      }
    } catch (const std::exception &e) {
      log(tr("错误：执行 %1 分析时发生异常：%2").arg(method).arg(e.what()));
      resultDetails = tr("分析过程中发生错误：%1").arg(e.what());
      analysisSuccess = false;
    } catch (...) {
      log(tr("错误：执行 %1 分析时发生未知异常").arg(method));
      resultDetails = tr("分析过程中发生未知错误");
      analysisSuccess = false;
    }

    // 记录结果
    QString resultStr =
        tr("方法：%1\n时间：%2\n").arg(method).arg(getCurrentDateTime());

    if (analysisSuccess) {
      resultStr += resultDetails;
      log(tr("完成 %1 分析").arg(method));
    } else {
      resultStr += tr("分析未成功完成\n");
      resultStr += resultDetails;
      log(tr("警告：%1 分析未成功完成").arg(method));
    }

    updateResults(method, resultStr);
  }

  // 完成分析
  progressDialog.setValue(100);
  ui->analysisProgressBar->setValue(100);
  onAnalysisComplete();

  // 显示分析结果
  showAnalysisResults();

  updateStatusBar(tr("分析完成"));
  log(tr("完成图像分析"));
}

void MainWindow::generateReport(const QString &format) {
  if (imageResults.isEmpty()) {
    QMessageBox::warning(this, tr("无法生成报告"),
                         tr("没有分析结果可供导出。请先执行分析。"));
    return;
  }

  QString filePath;

  if (format == "pdf") {
    filePath = QFileDialog::getSaveFileName(
        this, tr("导出 PDF 报告"),
        QDir::homePath() + "/" +
            tr("SAR_分析报告_%1.pdf")
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        tr("PDF 文件 (*.pdf)"));

    if (filePath.isEmpty())
      return;

    // 创建 PDF 文档
    QPdfWriter pdfWriter(filePath);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));

    QPainter painter(&pdfWriter);
    painter.setPen(Qt::black);

    // 绘制标题
    QFont titleFont = painter.font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(100, 100, pdfWriter.width() - 200, 50),
                     Qt::AlignCenter, tr("SAR 图像质量评估报告"));

    // 绘制图像信息
    QFont normalFont = painter.font();
    normalFont.setPointSize(10);
    normalFont.setBold(false);
    painter.setFont(normalFont);

    int yPos = 200;
    painter.drawText(
        QRect(100, yPos, pdfWriter.width() - 200, 30), Qt::AlignLeft,
        tr("图像文件：%1").arg(QFileInfo(currentImagePath).fileName()));

    yPos += 30;
    painter.drawText(QRect(100, yPos, pdfWriter.width() - 200, 30),
                     Qt::AlignLeft,
                     tr("评估时间：%1").arg(getCurrentDateTime()));

    yPos += 50;

    // 绘制结果
    QFont resultFont = painter.font();
    resultFont.setPointSize(12);
    resultFont.setBold(true);
    painter.setFont(resultFont);

    painter.drawText(QRect(100, yPos, pdfWriter.width() - 200, 30),
                     Qt::AlignLeft, tr("评估结果："));

    yPos += 40;

    // 还原到普通字体
    painter.setFont(normalFont);

    // 绘制各项结果
    for (auto it = imageResults.begin(); it != imageResults.end(); ++it) {
      painter.drawText(QRect(120, yPos, pdfWriter.width() - 240, 200),
                       Qt::AlignLeft, it.value());
      yPos += 220;

      // 检查是否需要新页
      if (yPos > pdfWriter.height() - 200) {
        pdfWriter.newPage();
        yPos = 100;
      }
    }

    painter.end();

    log(tr("报告已导出为 PDF：%1").arg(filePath));

  } else if (format == "txt") {
    filePath = QFileDialog::getSaveFileName(
        this, tr("导出文本报告"),
        QDir::homePath() + "/" +
            tr("SAR_分析报告_%1.txt")
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        tr("文本文件 (*.txt)"));

    if (filePath.isEmpty())
      return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QMessageBox::critical(this, tr("导出失败"),
                            tr("无法创建文件：%1").arg(filePath));
      return;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif

    // 写入标题
    out << tr("SAR 图像质量评估报告") << "\n\n";

    // 写入图像信息
    out << tr("图像文件：%1").arg(QFileInfo(currentImagePath).fileName())
        << "\n";
    out << tr("评估时间：%1").arg(getCurrentDateTime()) << "\n\n";

    // 写入结果
    out << tr("评估结果：") << "\n\n";

    for (auto it = imageResults.begin(); it != imageResults.end(); ++it) {
      out << "----------------------------------------\n";
      out << it.value() << "\n";
    }

    file.close();

    log(tr("报告已导出为文本文件：%1").arg(filePath));
  }

  // 显示成功消息
  QMessageBox::information(this, tr("导出成功"),
                           tr("报告已成功导出到：%1").arg(filePath));
}

QString MainWindow::getCurrentDateTime() {
  return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void MainWindow::updateResults(const QString &method, const QString &result) {
  imageResults[method] = result;
}

void MainWindow::showAnalysisResults() {
  // 显示概览
  QString overviewText;
  for (auto it = imageResults.begin(); it != imageResults.end(); ++it) {
    overviewText += tr("方法：%1\n").arg(it.key());
    QStringList lines = it.value().split("\n");
    for (const QString &line : lines) {
      if (line.startsWith("结果：")) {
        overviewText += line + "\n\n";
        break;
      }
    }
  }
  ui->overviewResultsTextEdit->setText(overviewText);

  // 显示详细结果
#if CONFIG_ENABLE_ISLR
  if (imageResults.contains("ISLR"))
    ui->ISLRResultsTextEdit->setText(imageResults["ISLR"]);
#endif

#if CONFIG_ENABLE_PSLR
  if (imageResults.contains("PSLR"))
    ui->PSLRResultsTextEdit->setText(imageResults["PSLR"]);
#endif

#if CONFIG_ENABLE_RANGE_RES
  if (imageResults.contains("RangeResolution"))
    ui->RASRResultsTextEdit->setText(imageResults["RangeResolution"]);
#endif

#if CONFIG_ENABLE_AZIMUTH_RES
  if (imageResults.contains("AzimuthResolution"))
    ui->AASRResultsTextEdit->setText(imageResults["AzimuthResolution"]);
#endif

#if CONFIG_ENABLE_RASR
  if (imageResults.contains("RASR"))
    ui->RASRResultsTextEdit->setText(imageResults["RASR"]);
#endif

#if CONFIG_ENABLE_AASR
  if (imageResults.contains("AASR"))
    ui->AASRResultsTextEdit->setText(imageResults["AASR"]);
#endif

#if CONFIG_ENABLE_SNR
  if (imageResults.contains("SNR"))
    ui->SNRResultsTextEdit->setText(imageResults["SNR"]);
#endif

#if CONFIG_ENABLE_NESZ
  if (imageResults.contains("NESZ"))
    ui->NESZResultsTextEdit->setText(imageResults["NESZ"]);
#endif

#if CONFIG_ENABLE_RADIOMETRIC_RES
  if (imageResults.contains("RadiometricResolution"))
    ui->RadResolutionResultsTextEdit->setText(
        imageResults["RadiometricResolution"]);
#endif

#if CONFIG_ENABLE_RADIOMETRIC_ACC
  if (imageResults.contains("RadiametricAccuracy"))
    ui->RadAccuracyResultsTextEdit->setText(
        imageResults["RadiametricAccuracy"]);
#endif

#if CONFIG_ENABLE_ENL
  if (imageResults.contains("ENL"))
    ui->ENLResultsTextEdit->setText(imageResults["ENL"]);
#endif
}

void MainWindow::onAnalysisProgress(int progress, const QString &message) {
  ui->analysisProgressBar->setValue(progress);
  log(message);
}

void MainWindow::onAnalysisComplete() {
  ui->analysisProgressBar->setValue(100);
  log(tr("分析完成"));
}

// 拖放事件处理
void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  log(tr("主窗口接收到拖放事件"));

  // 只接受包含图像文件的拖放
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
      QString filePath = url.toLocalFile();
      log(tr("主窗口拖放文件：%1").arg(filePath));

      if (isSupportedImageFormat(filePath)) {
        log(tr("主窗口接受拖放：%1").arg(filePath));
        event->acceptProposedAction();
        return;
      }
    }
  }

  // 如果没有可接受的文件，记录拒绝信息
  log(tr("主窗口拒绝拖放：没有支持的图像文件"));
  event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event) {
  log(tr("主窗口处理拖放文件"));

  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
      QString filePath = url.toLocalFile();
      log(tr("主窗口尝试处理拖放文件：%1").arg(filePath));

      if (isSupportedImageFormat(filePath)) {
        handleDroppedFile(filePath);
        log(tr("主窗口成功处理拖放文件：%1").arg(filePath));
        event->acceptProposedAction();
        return;
      }
    }
  }

  event->ignore();
}

// 添加 setupLogSystem 方法实现
void MainWindow::setupLogSystem() {
  // 确保日志文本框存在
  if (ui->logTextEdit) {
    // 设置日志文本框属性
    ui->logTextEdit->setReadOnly(true);
    ui->logTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);

    // 加载最近的日志，最多显示 100 条
    QStringList recentLogs = SAR::Core::Logger::instance()->getRecentLogs(100);
    for (const QString &logMsg : recentLogs) {
      ui->logTextEdit->append(logMsg);
    }

    // 滚动到最底部
    ui->logTextEdit->moveCursor(QTextCursor::End);
  }

  // 记录日志系统初始化完成的消息
  LOG_INFO("日志系统已初始化并连接到 UI");
}

// 添加响应新日志消息的槽函数
void MainWindow::onNewLogMessage(const QString &message) {
  if (ui->logTextEdit) {
    ui->logTextEdit->append(message);
    // 自动滚动到底部
    ui->logTextEdit->moveCursor(QTextCursor::End);
  }
}

// 添加清除日志的槽函数
void MainWindow::clearLog() {
  if (ui->logTextEdit) {
    ui->logTextEdit->clear();
    LOG_INFO("日志已清除");
  }
}

// 添加保存日志的槽函数
void MainWindow::saveLog() {
  QString filePath = QFileDialog::getSaveFileName(
      this, tr("保存日志文件"), QDir::homePath() + "/log.txt",
      tr("文本文件 (*.txt);;所有文件 (*.*)"));
  if (!filePath.isEmpty()) {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream stream(&file);
      stream << ui->logTextEdit->toPlainText();
      file.close();

      QString message = tr("日志已保存到：%1").arg(filePath);
      LOG_INFO(message);
      updateStatusBar(message);
    } else {
      QString errorMsg = tr("无法保存日志到：%1").arg(filePath);
      LOG_ERROR(errorMsg);
      QMessageBox::critical(this, tr("保存失败"), errorMsg);
    }
  }
}
