#include "include/mainwindow.h"
#include "../core/include/analysis_controller.h"
#include "include/drag_drop_graphics_view.h"
#include "include/report_generator.h"
#include "include/threshold_settings_dialog.h"
#include "include/filter_settings_dialog.h"
#include "../core/include/logger.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QLayout>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTabWidget>
#include <QThread>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , imageScene(new QGraphicsScene(this))
  , imageView(nullptr)
  , hasSelectedRegion(false)
  , imageHandler(new SAR::Core::ImageHandler())
  , analysisController(nullptr)
  , reportGenerator(nullptr)
{
  ui->setupUi(this);

  // 初始化成员变量
  imageScene = new QGraphicsScene(this);
  imageHandler = new SAR::Core::ImageHandler();
  
  // 设置日志记录器
  imageHandler->setLogger([this](const QString &msg) { this->log(msg); });
  
  // 创建分析控制器
  analysisController = new SAR::Core::AnalysisController(
    this, imageHandler, 
    [this](int progress, const QString &message) {
      this->onAnalysisProgress(progress, message);
    }
  );
  
  // 创建报告生成器
  reportGenerator = new SAR::UI::ReportGenerator(
    this, [this](const QString &msg) { this->log(msg); }
  );

  // 设置分析选项
  configureAnalysisOptions();
  configureResultTabs();
  
  // 设置窗口标题
  setWindowTitle(tr("SAR 图像质量分析工具"));

  // 设置图像查看器
  setupImageViewer();
  
  // 设置图像增强控件
  setupImageEnhancementControls();

  // 设置信号连接
  setupConnections();
  
  // 禁用分析按钮
  enableAnalysisButtons(false);
  
  // 日志初始化
  log(tr("应用程序已启动"));
}

MainWindow::~MainWindow() {
  delete ui;
  delete imageScene;
  delete imageHandler;
  // 析构前检查指针是否为空
  if (analysisController)
    delete analysisController;
  if (reportGenerator)
    delete reportGenerator;

  LOG_INFO("主窗口已销毁");
}

void MainWindow::setupImageViewer() {
  // 创建一个自定义的 DragDropGraphicsView
  imageView = new DragDropGraphicsView(this);
  imageView->setScene(imageScene);
  imageView->setDragMode(QGraphicsView::ScrollHandDrag);

  // 将新创建的 DragDropGraphicsView 添加到界面布局中
  if (ui->imageDisplayLabel) {
    // 获取占位符的父 widget
    QWidget *parentWidget = ui->imageDisplayLabel->parentWidget();
    if (parentWidget) {
      // 获取占位符在布局中的位置
      QLayout *layout = parentWidget->layout();
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

  LOG_INFO("已设置图像视图 (支持拖放功能)");
}

void MainWindow::setupConnections() {
  // 连接图像视图的拖放信号
  connect(imageView, SIGNAL(dragEnterReceived(QDragEnterEvent *)), this,
          SLOT(handleViewDragEnter(QDragEnterEvent *)));
  connect(imageView, SIGNAL(dropReceived(QDropEvent *)), this,
          SLOT(handleViewDrop(QDropEvent *)));

  // 连接日志系统信号
  connect(SAR::Core::Logger::instance(), SIGNAL(newLogMessage(const QString &)),
          this, SLOT(onNewLogMessage(const QString &)));

  // 全选/取消全选按钮
  connect(ui->checkBoxSelectAll, &QCheckBox::toggled, this,
          &MainWindow::on_checkBoxSelectAll_toggled);

  // 当有图像加载时启用分析按钮
  connect(ui->imageListWidget, &QListWidget::itemSelectionChanged, [this]() {
    bool hasSelectedItems = !ui->imageListWidget->selectedItems().isEmpty();
    enableAnalysisButtons(hasSelectedItems);
  });

  // 连接分析控制器信号
  connect(analysisController, &SAR::Core::AnalysisController::analysisProgress,
          this, &MainWindow::onAnalysisProgress);
  connect(analysisController, &SAR::Core::AnalysisController::analysisComplete,
          this, &MainWindow::onAnalysisComplete);

  LOG_INFO("日志系统已设置");
}

void MainWindow::configureAnalysisOptions() {
  // 配置分析选项
  LOG_INFO("已加载分析选项");

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
  ui->actionRangeResolution->setVisible(false);
#endif

#if !CONFIG_ENABLE_AZIMUTH_RES
  ui->actionAzimuthResolution->setVisible(false);
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

  LOG_INFO("已配置分析选项");
}

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
    if (tab == ui->tabISLR) {
      // 积分旁瓣比
#if !CONFIG_ENABLE_ISLR
      shouldShow = false;
#endif
    } else if (tab == ui->tabPSLR) {
      // 峰值旁瓣比
#if !CONFIG_ENABLE_PSLR
      shouldShow = false;
#endif
    } else if (tab == ui->tabRASR) {
      // 距离模糊度
#if !CONFIG_ENABLE_RASR
      shouldShow = false;
#endif
    } else if (tab == ui->tabAASR) {
      // 方位模糊度
#if !CONFIG_ENABLE_AASR
      shouldShow = false;
#endif
    } else if (tab == ui->tabSNR) {
      // 信噪比
#if !CONFIG_ENABLE_SNR
      shouldShow = false;
#endif
    } else if (tab == ui->tabRadAccuracy) {
      // 辐射精度
#if !CONFIG_ENABLE_RADIOMETRIC_ACC
      shouldShow = false;
#endif
    } else if (tab == ui->tabNESZ) {
      // NESZ
#if !CONFIG_ENABLE_NESZ
      shouldShow = false;
#endif
    } else if (tab == ui->tabRadResolution) {
      // 辐射分辨率
#if !CONFIG_ENABLE_RADIOMETRIC_RES
      shouldShow = false;
#endif
    } else if (tab == ui->tabENL) {
      // 等效视数
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

void MainWindow::enableAnalysisButtons(bool enable) {
  ui->startAnalysisButton->setEnabled(enable);
  ui->actionStartAssessment->setEnabled(enable);
  ui->actionSelectAssessmentRegion->setEnabled(enable);
  ui->pushButton_exportPDF->setEnabled(enable);
  ui->pushButton_exportTXT->setEnabled(enable);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void MainWindow::dropEvent(QDropEvent *event) {
  if (event->mimeData()->hasUrls()) {
    QString filePath = event->mimeData()->urls().first().toLocalFile();
    handleDroppedFile(filePath);
  }
}

void MainWindow::handleViewDragEnter(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void MainWindow::handleViewDrop(QDropEvent *event) {
  if (event->mimeData()->hasUrls()) {
    QString filePath = event->mimeData()->urls().first().toLocalFile();
    handleDroppedFile(filePath);
  }
}

void MainWindow::handleDroppedFile(const QString &filePath) {
  if (isSupportedImageFormat(filePath)) {
    loadImage(filePath);
  } else {
    QMessageBox::warning(this, tr("不支持的文件格式"),
                         tr("文件 %1 不是支持的图像格式。").arg(filePath));
  }
}

bool MainWindow::isSupportedImageFormat(const QString &filePath) {
  // 简单检查文件扩展名
  QFileInfo fileInfo(filePath);
  QString extension = fileInfo.suffix().toLower();

  // 支持的格式列表
  QStringList supportedFormats = {
    "tif", "tiff", "jpg", "jpeg",
    "png", "bmp", "gif"
  };

  return supportedFormats.contains(extension);
}

bool MainWindow::loadImage(const QString &filePath) {
  // 使用 GDAL 加载图像
  if (imageHandler->loadImage(filePath)) {
    // 更新 UI
    QPixmap pixmap = imageHandler->getDisplayPixmap(imageView->size());
    if (pixmap.isNull()) {
      QMessageBox::warning(this, tr("图像显示失败"),
                         tr("无法将图像转换为可显示格式：%1").arg(filePath));
      LOG_ERROR(QString("无法将图像转换为可显示格式：%1").arg(filePath));
      return false;
    }
    
    imageScene->clear();
    QGraphicsPixmapItem *item = imageScene->addPixmap(pixmap);
    imageScene->setSceneRect(item->boundingRect());
    imageView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);

    // 更新当前图像路径
    currentImagePath = filePath;

    // 添加到图像列表
    if (!loadedImages.contains(filePath)) {
      loadedImages.append(filePath);
      QListWidgetItem *item =
          new QListWidgetItem(QFileInfo(filePath).fileName());
      item->setData(Qt::UserRole, filePath);
      ui->imageListWidget->addItem(item);
    }

    // 选择图像列表中的当前项
    for (int i = 0; i < ui->imageListWidget->count(); i++) {
      if (ui->imageListWidget->item(i)->data(Qt::UserRole).toString() ==
          filePath) {
        ui->imageListWidget->setCurrentRow(i);
        break;
      }
    }

    // 启用分析按钮
    enableAnalysisButtons(true);
    
    // 启用图像增强控件
    QList<QGroupBox*> enhancementGroups = findChildren<QGroupBox*>();
    for (QGroupBox* group : enhancementGroups) {
      if (group->property("enhancementGroup").toBool()) {
        group->setEnabled(true);
      }
    }
    
    // 自动增强图像显示
    onAutoEnhanceClicked();

    // 更新状态栏
    updateStatusBar(tr("已加载图像：%1").arg(QFileInfo(filePath).fileName()));

    LOG_INFO(QString("已加载图像：%1").arg(filePath));

    return true;
  } else {
    QMessageBox::warning(this, tr("加载失败"),
                         tr("无法加载图像文件：%1").arg(filePath));
    LOG_ERROR(QString("无法加载图像文件：%1").arg(filePath));
    return false;
  }
}

void MainWindow::updateStatusBar(const QString &message) {
  ui->statusbar->showMessage(message);
}

void MainWindow::onNewLogMessage(const QString &message) {
  ui->logTextEdit->append(message);
}

void MainWindow::clearLog() {
  ui->logTextEdit->clear();
  LOG_INFO("日志已清除");
}

void MainWindow::saveLog() {
  QString fileName = QFileDialog::getSaveFileName(
    this, tr("保存日志"), QString(), tr("日志文件 (*.log);;所有文件 (*)"));
  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream stream(&file);
      stream << ui->logTextEdit->toPlainText();
      file.close();

      LOG_INFO(QString("日志已保存到：%1").arg(fileName));
    } else {
      QMessageBox::warning(this, tr("保存失败"),
                           tr("无法保存日志文件：%1").arg(fileName));
      LOG_ERROR(QString("无法保存日志文件：%1").arg(fileName));
    }
  }
}

void MainWindow::on_actionOpenImage_triggered() {
  QString fileName = QFileDialog::getOpenFileName(
    this, tr("打开 SAR 图像"), QString(),
    tr("图像文件 (*.tif *.tiff *.jpg *.jpeg *.png *.bmp);;所有文件 (*)"));
  if (!fileName.isEmpty()) {
    loadImage(fileName);
  }
}

void MainWindow::on_actionCloseImage_triggered() {
  // 清除当前显示的图像
  imageScene->clear();
  currentImagePath.clear();

  // 禁用分析按钮
  enableAnalysisButtons(false);

  // 更新状态栏
  updateStatusBar(tr("已关闭图像"));

  LOG_INFO("已关闭图像");
}

void MainWindow::on_checkBoxSelectAll_toggled(bool checked) {
  // 全选或取消全选所有分析选项
  ui->checkBoxISLR->setChecked(checked);
  ui->checkBoxPSLR->setChecked(checked);
  ui->checkBoxRASR->setChecked(checked);
  ui->checkBoxAASR->setChecked(checked);
  ui->checkBoxSNR->setChecked(checked);
  ui->checkBoxNESZ->setChecked(checked);
  ui->checkBoxRadiometricAccuracy->setChecked(checked);
  ui->checkBoxRadiometricResolution->setChecked(checked);
  ui->checkBoxENL->setChecked(checked);

  LOG_INFO(checked ? "已全选所有分析选项" : "已取消全选分析选项");
}

// 分析功能的实现
void MainWindow::on_startAnalysisButton_clicked() {
  // 检查是否有选中的图像
  if (currentImagePath.isEmpty()) {
    QMessageBox::warning(this, tr("无图像"), tr("请先加载 SAR 图像"));
    return;
  }

  // 收集选中的分析方法
  QStringList selectedMethods;
  if (ui->checkBoxISLR->isChecked())
    selectedMethods << "ISLR";
  if (ui->checkBoxPSLR->isChecked())
    selectedMethods << "PSLR";
  if (ui->checkBoxRASR->isChecked())
    selectedMethods << "RASR";
  if (ui->checkBoxAASR->isChecked())
    selectedMethods << "AASR";
  if (ui->checkBoxSNR->isChecked())
    selectedMethods << "SNR";
  if (ui->checkBoxNESZ->isChecked())
    selectedMethods << "NESZ";
  if (ui->checkBoxRadiometricAccuracy->isChecked())
    selectedMethods << "RadiometricAccuracy";
  if (ui->checkBoxRadiometricResolution->isChecked())
    selectedMethods << "RadiometricResolution";
  if (ui->checkBoxENL->isChecked())
    selectedMethods << "ENL";

  // 检查是否有选中的方法
  if (selectedMethods.isEmpty()) {
    QMessageBox::warning(this, tr("未选择方法"), tr("请选择至少一种分析方法"));
    return;
  }

  // 执行分析
  performAnalysis(selectedMethods);
}

void MainWindow::performAnalysis(const QStringList &selectedMethods) {
  LOG_INFO(QString("开始分析，选中的方法：%1").arg(selectedMethods.join(", ")));

  // 清除之前的结果
  clearResults();

  // 使用 AnalysisController 执行分析
  // 这将通过信号返回结果
  currentResults =
      analysisController->performAnalysis(selectedMethods, currentImagePath);

  showAnalysisResult(currentResults);

  LOG_INFO("分析完成");
}

void MainWindow::clearResults() {
  imageResults.clear();

  // 清除所有结果显示区域
  ui->overviewResultsTextEdit->clear();
  ui->ISLRResultsTextEdit->clear();
  ui->PSLRResultsTextEdit->clear();
  ui->RASRResultsTextEdit->clear();
  ui->AASRResultsTextEdit->clear();
  ui->SNRResultsTextEdit->clear();
  ui->NESZResultsTextEdit->clear();
  ui->RadAccuracyResultsTextEdit->clear();
  ui->RadResolutionResultsTextEdit->clear();

  LOG_INFO("已清除之前的分析结果");
}

void MainWindow::updateResults(const QString &method, const QString &result) {
  imageResults[method] = result;
  LOG_INFO(QString("更新了 %1 的分析结果").arg(method));
}

void MainWindow::showAnalysisResult(const SAR::Core::AnalysisResult &result) {
  // 保存当前结果
  currentResults = result;

  // 生成概览 HTML
  QString overviewHtml = result.toHtml();
  ui->overviewResultsTextEdit->setHtml(overviewHtml);

  // 为每个分析方法分别显示结果
  auto results = result.getAllResults();
  QMapIterator<QString, SAR::Core::AnalysisResultItem> i(results);
  while (i.hasNext()) {
    i.next();
    const QString &methodName = i.key();
    const SAR::Core::AnalysisResultItem &item = i.value();

    // 构建此方法的 HTML 结果
    QString html = "<h3>" + item.methodName + "</h3>";

    if (item.isSuccess) {
      html += "<p><b>结果值：</b>" + QString::number(item.numericValue);
      if (!item.unit.isEmpty()) {
        html += " " + item.unit;
      }
      html += "</p>";

      if (!item.description.isEmpty()) {
        html += "<p>" + item.description + "</p>";
      }

      // 附加数值结果
      if (!item.additionalValues.isEmpty()) {
        html += "<p><b>详细数值：</b></p><ul>";
        QMapIterator<QString, double> j(item.additionalValues);
        while (j.hasNext()) {
          j.next();
          html +=
              "<li>" + j.key() + ": " + QString::number(j.value()) + "</li>";
        }
        html += "</ul>";
      }

      // 附加信息
      if (!item.additionalInfo.isEmpty()) {
        html += "<p><b>附加信息：</b></p><ul>";
        QMapIterator<QString, QString> k(item.additionalInfo);
        while (k.hasNext()) {
          k.next();
          html += "<li>" + k.key() + ": " + k.value() + "</li>";
        }
        html += "</ul>";
      }
    } else {
      html += "<p><b>分析失败</b></p>";
      html += "<p><b>错误信息：</b>" + item.errorMessage + "</p>";
    }

    // 在相应的选项卡中显示
    if (methodName == "ISLR") {
      ui->ISLRResultsTextEdit->setHtml(html);
    } else if (methodName == "PSLR") {
      ui->PSLRResultsTextEdit->setHtml(html);
    } else if (methodName == "RASR") {
      ui->RASRResultsTextEdit->setHtml(html);
    } else if (methodName == "AASR") {
      ui->AASRResultsTextEdit->setHtml(html);
    } else if (methodName == "SNR") {
      ui->SNRResultsTextEdit->setHtml(html);
    } else if (methodName == "NESZ") {
      ui->NESZResultsTextEdit->setHtml(html);
    } else if (methodName == "RadiometricAccuracy") {
      ui->RadAccuracyResultsTextEdit->setHtml(html);
    } else if (methodName == "RadiometricResolution") {
      ui->RadResolutionResultsTextEdit->setHtml(html);
    } else if (methodName == "ENL") {
      ui->ENLResultsTextEdit->setHtml(html);
    }

    // 同时更新旧的结果存储方式，保持向后兼容
    updateResults(methodName, html);
  }

  // 切换到结果选项卡
  ui->resultsTabWidget->setCurrentIndex(0); // 概览选项卡

  LOG_INFO("已显示分析结果");
}

// 导出结果功能
void MainWindow::on_pushButton_exportPDF_clicked() { generateReport("PDF"); }

void MainWindow::on_pushButton_exportTXT_clicked() { generateReport("TXT"); }

void MainWindow::generateReport(const QString &format) {
  // 优先使用新的分析结果
  if (!currentResults.getAllResults().isEmpty()) {
    // 使用新的接口生成报告
    bool success = reportGenerator->generateReport(format, currentResults);

    if (success) {
      QMessageBox::information(this, tr("导出成功"), tr("报告已成功导出"));
      LOG_INFO(QString("报告已导出"));
    } else {
      QMessageBox::warning(this, tr("导出失败"), tr("无法导出报告"));
      LOG_ERROR(QString("导出报告失败"));
    }
  }
  // 如果没有新结果，使用旧的结果
  else if (!imageResults.isEmpty()) {
    // 使用旧的接口生成报告
    bool success =
        reportGenerator->generateReport(format, imageResults, currentImagePath);

    if (success) {
      QMessageBox::information(this, tr("导出成功"), tr("报告已成功导出"));
      LOG_INFO(QString("报告已导出"));
    } else {
      QMessageBox::warning(this, tr("导出失败"), tr("无法导出报告"));
      LOG_ERROR(QString("导出报告失败"));
    }
  } else {
    QMessageBox::warning(this, tr("无结果"), tr("没有分析结果可导出"));
  }
}

// 其他必要函数
void MainWindow::on_actionStartAssessment_triggered() {
  on_startAnalysisButton_clicked();
}

void MainWindow::on_actionSelectAssessmentRegion_triggered() {
  // 实现区域选择逻辑
  QMessageBox::information(this, tr("区域选择"),
                           tr("请在图像上选择感兴趣区域"));
}

void MainWindow::on_actionExportPDF_triggered() {
  on_pushButton_exportPDF_clicked();
}

void MainWindow::on_actionExportTXT_triggered() {
  on_pushButton_exportTXT_clicked();
}

void MainWindow::on_actionZoomIn_triggered() { imageView->scale(1.2, 1.2); }

void MainWindow::on_actionZoomOut_triggered() { imageView->scale(0.8, 0.8); }

void MainWindow::on_actionFitToWindow_triggered() {
  imageView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::on_actionPan_toggled(bool checked) {
  if (checked) {
    imageView->setDragMode(QGraphicsView::ScrollHandDrag);
  } else {
    imageView->setDragMode(QGraphicsView::NoDrag);
  }
}

void MainWindow::on_actionFullScreen_triggered() {
  if (isFullScreen()) {
    showNormal();
  } else {
    showFullScreen();
  }
}

void MainWindow::on_imageListWidget_itemClicked(QListWidgetItem *item) {
  QString filePath = item->data(Qt::UserRole).toString();
  if (!filePath.isEmpty() && filePath != currentImagePath) {
    loadImage(filePath);
  }
}

void MainWindow::on_actionAbout_triggered() {
  QMessageBox::about(this, tr("关于"),
                     tr("SAR 图像质量分析工具\n版本 "
                       "1.0\n\n用于合成孔径雷达图像质量评估的工具"));
}

QString MainWindow::getCurrentDateTime() {
  return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void MainWindow::log(const QString &message) { LOG_INFO(message); }

void MainWindow::onAnalysisProgress(int progress, const QString &message) {
  // 更新状态栏
  updateStatusBar(message);

  // 可以在这里更新进度条等 UI 元素

  LOG_INFO(QString("分析进度：%1% - %2").arg(progress).arg(message));
}

void MainWindow::onAnalysisComplete(const SAR::Core::AnalysisResult &results) {
  // 分析完成后的处理
  updateStatusBar(tr("分析已完成"));

  // 显示分析结果
  showAnalysisResult(results);

  LOG_INFO("分析已完成");
}

// 实现阈值设置对话框的槽函数
void MainWindow::on_actionThresholdSettings_triggered()
{
    // 创建并显示阈值设置对话框
    SAR::UI::ThresholdSettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 更新状态栏
        updateStatusBar(tr("阈值设置已更新"));
        LOG_INFO("用户更新了分析算法阈值设置");
    }
}

// 添加滤波相关槽的实现
void MainWindow::on_actionFilterSettings_triggered() {
    showFilterSettingsDialog();
}

void MainWindow::on_actionLowPassFilter_triggered() {
    applyFilter(SAR::Core::FilterType::LowPass);
}

void MainWindow::on_actionHighPassFilter_triggered() {
    applyFilter(SAR::Core::FilterType::HighPass);
}

void MainWindow::on_actionBandPassFilter_triggered() {
    applyFilter(SAR::Core::FilterType::BandPass);
}

void MainWindow::on_actionMedianFilter_triggered() {
    applyFilter(SAR::Core::FilterType::Median);
}

void MainWindow::on_actionGaussianFilter_triggered() {
    applyFilter(SAR::Core::FilterType::Gaussian);
}

void MainWindow::on_actionBilateralFilter_triggered() {
    applyFilter(SAR::Core::FilterType::Bilateral);
}

void MainWindow::on_actionLeeFilter_triggered() {
    applyFilter(SAR::Core::FilterType::Lee);
}

void MainWindow::on_actionFrostFilter_triggered() {
    applyFilter(SAR::Core::FilterType::Frost);
}

void MainWindow::on_actionKuanFilter_triggered() {
    applyFilter(SAR::Core::FilterType::Kuan);
}

void MainWindow::applyFilter(SAR::Core::FilterType filterType) {
    // 检查是否有图像加载
    if (!imageHandler || !imageHandler->isValid()) {
        QMessageBox::warning(this, tr("警告"), tr("请先加载图像"));
        return;
    }
    
    // 根据滤波器类型显示设置对话框或直接应用默认参数
    showFilterSettingsDialog(filterType);
}

void MainWindow::showFilterSettingsDialog(SAR::Core::FilterType filterType) {
    // 创建滤波器设置对话框
    FilterSettingsDialog dialog(this, filterType, currentFilterParams);
    
    // 显示对话框
    if (dialog.exec() == QDialog::Accepted) {
        // 获取用户设置的参数
        currentFilterParams = dialog.getFilterParameters();
        
        // 创建处理日志
        QString filterLog;
        
        // 更新状态栏
        updateStatusBar(tr("正在应用 %1...").arg(
            SAR::Core::ImageFilters::getFilterTypeDescription(currentFilterParams.type)));
        
        // 应用滤波器
        QApplication::setOverrideCursor(Qt::WaitCursor);
        bool success = imageHandler->applyFilterInplace(currentFilterParams, &filterLog);
        QApplication::restoreOverrideCursor();
        
        // 记录日志
        log(filterLog);
        
        if (success) {
            // 更新图像显示
            if (imageScene) {
                imageScene->clear();
                QPixmap pixmap = imageHandler->getDisplayPixmap(imageView->size());
                imageScene->addPixmap(pixmap);
                imageView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);
            }
            
            updateStatusBar(tr("成功应用 %1").arg(
                SAR::Core::ImageFilters::getFilterTypeDescription(currentFilterParams.type)));
        } else {
            QMessageBox::critical(this, tr("错误"), tr("无法应用滤波器，请查看日志了解详情"));
            updateStatusBar(tr("无法应用滤波器"));
        }
    }
}

void MainWindow::setupImageEnhancementControls() {
    // 创建增强控件容器
    QGroupBox* enhancementGroup = new QGroupBox(tr("SAR图像增强"), this);
    QVBoxLayout* enhancementLayout = new QVBoxLayout(enhancementGroup);
    
    // 创建显示模式选择下拉框
    QHBoxLayout* displayModeLayout = new QHBoxLayout();
    QLabel* displayModeLabel = new QLabel(tr("显示模式:"), this);
    QComboBox* displayModeCombo = new QComboBox(this);
    displayModeCombo->setObjectName("displayModeCombo");
    displayModeCombo->addItem(tr("线性缩放"), static_cast<int>(SAR::Core::ImageDisplayMode::Linear));
    displayModeCombo->addItem(tr("对数缩放"), static_cast<int>(SAR::Core::ImageDisplayMode::Logarithmic));
    displayModeCombo->addItem(tr("平方根缩放"), static_cast<int>(SAR::Core::ImageDisplayMode::Sqrt));
    displayModeCombo->addItem(tr("百分比裁剪"), static_cast<int>(SAR::Core::ImageDisplayMode::ClipPercent));
    connect(displayModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onDisplayModeChanged(int)));
    displayModeLayout->addWidget(displayModeLabel);
    displayModeLayout->addWidget(displayModeCombo);
    
    // 创建裁剪百分比设置
    QGroupBox* clipGroup = new QGroupBox(tr("裁剪百分比设置"), this);
    clipGroup->setObjectName("clipGroup");
    QGridLayout* clipLayout = new QGridLayout(clipGroup);
    
    QLabel* lowerLabel = new QLabel(tr("下限 (%):"), this);
    QDoubleSpinBox* lowerSpinBox = new QDoubleSpinBox(this);
    lowerSpinBox->setObjectName("lowerClipSpinBox");
    lowerSpinBox->setRange(0.0, 50.0);
    lowerSpinBox->setValue(1.0);
    lowerSpinBox->setSingleStep(0.5);
    
    QLabel* upperLabel = new QLabel(tr("上限 (%):"), this);
    QDoubleSpinBox* upperSpinBox = new QDoubleSpinBox(this);
    upperSpinBox->setObjectName("upperClipSpinBox");
    upperSpinBox->setRange(50.0, 100.0);
    upperSpinBox->setValue(99.0);
    upperSpinBox->setSingleStep(0.5);
    
    connect(lowerSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onClipPercentileChanged()));
    connect(upperSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onClipPercentileChanged()));
    
    clipLayout->addWidget(lowerLabel, 0, 0);
    clipLayout->addWidget(lowerSpinBox, 0, 1);
    clipLayout->addWidget(upperLabel, 1, 0);
    clipLayout->addWidget(upperSpinBox, 1, 1);
    
    // 创建自动增强按钮
    QPushButton* autoEnhanceButton = new QPushButton(tr("自动增强"), this);
    autoEnhanceButton->setObjectName("autoEnhanceButton");
    connect(autoEnhanceButton, SIGNAL(clicked()), this, SLOT(onAutoEnhanceClicked()));
    
    // 将控件添加到布局
    enhancementLayout->addLayout(displayModeLayout);
    enhancementLayout->addWidget(clipGroup);
    enhancementLayout->addWidget(autoEnhanceButton);
    enhancementLayout->addStretch();
    
    // 找到适合的位置添加增强控件
    if (ui->dockWidgetContents_analysis && ui->verticalLayout_analysis) {
        // 添加到分析面板中
        ui->verticalLayout_analysis->insertWidget(0, enhancementGroup);
    } else if (ui->dockWidgetContents_log && ui->verticalLayout_log) {
        // 添加到日志面板上方
        ui->verticalLayout_log->insertWidget(0, enhancementGroup);
    } else {
        // 如果没找到合适的布局，直接添加到主窗口
        QVBoxLayout* centralLayout = new QVBoxLayout();
        centralLayout->addWidget(enhancementGroup);
        this->setCentralWidget(new QWidget(this));
        this->centralWidget()->setLayout(centralLayout);
    }
    
    // 初始禁用控件，等图像加载后启用
    enhancementGroup->setEnabled(false);
    // 保存控件指针以便后续访问
    enhancementGroup->setProperty("enhancementGroup", true);
    
    // 设置裁剪控件初始状态
    clipGroup->setVisible(false); // 默认不显示裁剪控件
}

void MainWindow::refreshImageDisplay() {
    if (!imageHandler->isValid()) {
        return;
    }
    
    QPixmap pixmap = imageHandler->getDisplayPixmap(imageView->size());
    if (pixmap.isNull()) {
        QMessageBox::warning(this, tr("图像显示失败"),
                         tr("无法将图像转换为可显示格式"));
        return;
    }
    
    // 保存当前的变换状态
    QTransform currentTransform;
    if (imageScene->items().size() > 0) {
        QGraphicsItem* item = imageScene->items().first();
        currentTransform = item->transform();
    }
    
    // 清除场景并添加新的图像
    imageScene->clear();
    QGraphicsPixmapItem *item = imageScene->addPixmap(pixmap);
    
    // 应用之前的变换
    item->setTransform(currentTransform);
    
    // 更新场景边界
    imageScene->setSceneRect(item->boundingRect());
    
    // 更新状态栏
    updateStatusBar(tr("已刷新图像显示: %1").arg(QFileInfo(currentImagePath).fileName()));
}

void MainWindow::applyImageEnhancement() {
    if (!imageHandler->isValid()) {
        return;
    }
    
    // 刷新图像显示
    refreshImageDisplay();
}

void MainWindow::onDisplayModeChanged(int index) {
    if (!imageHandler->isValid()) {
        return;
    }
    
    QComboBox* comboBox = qobject_cast<QComboBox*>(sender());
    if (!comboBox) {
        return;
    }
    
    // 获取选中的显示模式
    SAR::Core::ImageDisplayMode mode = static_cast<SAR::Core::ImageDisplayMode>(
        comboBox->itemData(index).toInt());
    
    // 设置显示模式
    imageHandler->setDisplayMode(mode);
    
    // 更新裁剪控件可见性
    QGroupBox* clipGroup = findChild<QGroupBox*>("clipGroup");
    if (clipGroup) {
        clipGroup->setVisible(mode == SAR::Core::ImageDisplayMode::ClipPercent);
    }
    
    // 应用图像增强
    applyImageEnhancement();
}

void MainWindow::onClipPercentileChanged() {
    if (!imageHandler->isValid()) {
        return;
    }
    
    QDoubleSpinBox* lowerSpinBox = findChild<QDoubleSpinBox*>("lowerClipSpinBox");
    QDoubleSpinBox* upperSpinBox = findChild<QDoubleSpinBox*>("upperClipSpinBox");
    
    if (!lowerSpinBox || !upperSpinBox) {
        return;
    }
    
    // 获取设置的裁剪百分比
    double lowerPercent = lowerSpinBox->value();
    double upperPercent = upperSpinBox->value();
    
    // 设置裁剪百分比
    imageHandler->setClipPercentile(lowerPercent, upperPercent);
    
    // 应用图像增强
    applyImageEnhancement();
}

void MainWindow::onAutoEnhanceClicked() {
    if (!imageHandler->isValid()) {
        return;
    }
    
    // 执行自动增强
    if (imageHandler->autoEnhance()) {
        // 更新UI控件状态
        QComboBox* displayModeCombo = findChild<QComboBox*>("displayModeCombo");
        if (displayModeCombo) {
            // 获取当前显示模式
            SAR::Core::ImageDisplayMode mode = imageHandler->getDisplayMode();
            // 更新下拉框选择
            for (int i = 0; i < displayModeCombo->count(); i++) {
                if (displayModeCombo->itemData(i).toInt() == static_cast<int>(mode)) {
                    displayModeCombo->setCurrentIndex(i);
                    break;
                }
            }
        }
        
        // 获取裁剪百分比
        QPair<double, double> clipValues = imageHandler->getClipPercentile();
        
        // 更新裁剪百分比控件
        QDoubleSpinBox* lowerSpinBox = findChild<QDoubleSpinBox*>("lowerClipSpinBox");
        QDoubleSpinBox* upperSpinBox = findChild<QDoubleSpinBox*>("upperClipSpinBox");
        
        if (lowerSpinBox && upperSpinBox) {
            lowerSpinBox->setValue(clipValues.first);
            upperSpinBox->setValue(clipValues.second);
        }
        
        // 应用图像增强
        applyImageEnhancement();
        
        // 更新状态栏
        updateStatusBar(tr("已自动增强图像显示"));
    }
}
