#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <cmath>
#include <functional> // For std::bind or lambda used in constructor
#include <vector>

#include <QDateTime> // Needed for logMessage timestamp
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
// #include <QImage> // No longer needed directly here
#include <QCoreApplication> // For processEvents
#include <QFileInfo>        // For QFileInfo
#include <QMessageBox>
#include <QMimeData>
#include <QPixmap>
#include <QUrl>
#include <QtPrintSupport/QPrinter>      // 用于 PDF 导出
#include <QtPrintSupport/QPrintDialog>  // 用于打印对话框
#include <QTextDocument>    // 用于导出 TXT 文件内容处理
#include <QFile>            // 用于文件操作
#include <QTextStream>      // 用于文本流操作
#include <QStringConverter> // 用于 Qt6 中的文本编码转换
#include <QVBoxLayout>      // 用于垂直布局
#include <QHBoxLayout>      // 用于水平布局
#include <QGroupBox>        // 用于分组框
#include <QCheckBox>        // 用于复选框
#include <QPushButton>      // 用于按钮
#include <QProgressBar>     // 用于进度条
#include <QListWidget>      // 用于列表控件

// Keep OpenCV includes needed for analysis functions if they remain in this
// file
#include <opencv2/core.hpp> // Needed for cv::Mat type passed to analysis funcs

// Keep GDAL includes only for GDALAllRegister
#include "gdal_priv.h" // For GDALAllRegister, GDALClose(if needed), GDALDataTypeIsComplex etc. might be needed by analysis functions if not refactored.

// Include analysis utilities header
#include "analysis_utils.h"

// Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
      // Initialize ImageHandler with a lambda that captures 'this' and calls
      // logMessage
      ,
      m_imageHandler([this](const QString &msg) { this->logMessage(msg); }),
      m_currentImageIndex(-1) {
  ui->setupUi(this);
  setAcceptDrops(true); // Enable drag & drop
  GDALAllRegister();    // Initialize GDAL drivers
  logMessage(tr("应用程序已启动。GDAL 已初始化。允许拖放。"));

  // Initial UI State
  //ui->startAnalysisButton->setEnabled(false);
  //on_checkBoxSelectAll_toggled(ui->checkBoxSelectAll->isChecked());

  ui->imageDisplayLabel->setText(tr("图像显示区域"));
  ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
  ui->imageDisplayLabel->setStyleSheet("QLabel { color: grey; }");

  // 创建质量评估面板
  createQualityPanel();

  // Placeholder texts (no change)
  ui->overviewResultsTextEdit->setPlaceholderText(
      tr("所有选定分析结果的总结将显示在此处..."));
  ui->method1ResultsTextEdit->setPlaceholderText(
      tr("SNR/ENL 分析的详细结果..."));
  ui->method2ResultsTextEdit->setPlaceholderText(
      tr("信息内容（熵）的详细结果..."));
  ui->method3ResultsTextEdit->setPlaceholderText(
      tr("清晰度（梯度幅值）的详细结果..."));
  ui->method4ResultsTextEdit->setPlaceholderText(
      tr("辐射统计（最小值、最大值、平均值、标准差）的详细结果..."));
  ui->method5ResultsTextEdit->setPlaceholderText(
      tr("GLCM 纹理特征的详细结果..."));
  ui->logTextEdit->setPlaceholderText(
      tr("日志消息（加载、分析步骤、错误）将显示在此处..."));
      
  // 确保图像列表可见
  ui->dockWidget_imageList->setVisible(true);
  
  // 连接图像列表信号槽
  connect(ui->dockWidget_imageList->findChild<QListWidget*>("imageListWidget"), 
          SIGNAL(itemClicked(QListWidgetItem*)), 
          this, SLOT(on_imageListWidget_itemClicked(QListWidgetItem*)));
}

// Destructor
MainWindow::~MainWindow() {
  // No need to call closeCurrentImage() here.
  // m_imageHandler's destructor will be called automatically,
  // which in turn calls its own closeImage() method.
  delete ui;
  // Consider GDALCleanupAll(); if appropriate for application lifecycle
}

// Log Message (no change)
void MainWindow::logMessage(const QString &message) {
  QString timestamp =
      QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
  ui->logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
}

// Close Current Image (Handles UI reset)
void MainWindow::closeCurrentImage() {
  logMessage(tr("关闭当前图像并重置 UI。"));
  m_imageHandler.closeImage(); // Delegate resource closing to handler

  // 如果有当前图像索引
  if(m_currentImageIndex >= 0 && m_currentImageIndex < m_loadedImages.size()) {
    // 从列表中移除
    m_loadedImages.removeAt(m_currentImageIndex);
    m_currentImageIndex = -1;
    
    // 更新图像列表
    updateImageList();
    
    // 如果列表不为空，加载第一个图像
    if(!m_loadedImages.isEmpty()) {
        loadAndDisplayImage(0);
        return;
    }
  }

  // Reset UI elements to initial state
  ui->imageDisplayLabel->clear(); // Clear the pixmap
  ui->imageDisplayLabel->setText(
      tr("图像显示区域")); // Restore placeholder
  ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
  ui->imageDisplayLabel->setStyleSheet(
      "QLabel { color: grey; }"); // Restore style

  m_startAnalysisButton->setEnabled(false); // Disable analysis

  // Clear results tabs
  ui->overviewResultsTextEdit->clear();
  ui->method1ResultsTextEdit->clear();
  ui->method2ResultsTextEdit->clear();
  ui->method3ResultsTextEdit->clear();
  ui->method4ResultsTextEdit->clear();
  ui->method5ResultsTextEdit->clear();
}

// Drag Enter Event (Minor logging change)
void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
      QString filePath = urls.first().toLocalFile();
      QFileInfo fileInfo(filePath);
      QString suffix = fileInfo.suffix().toLower();
      QStringList supportedSuffixes = {"tif", "tiff", "img", "hdr", "dat"};
      if (supportedSuffixes.contains(suffix)) {
        event->acceptProposedAction();
        // Log handled internally by openImageFile/ImageHandler now,
        // but we can keep this high-level log.
        logMessage(tr("拖入支持的文件：%1")
                       .arg(fileInfo.fileName()));
        return;
      } else {
        logMessage(
            tr("拖入不支持的文件类型：.%1").arg(suffix));
      }
    }
  }
  event->ignore();
}

// Drop Event (Calls modified openImageFile)
void MainWindow::dropEvent(QDropEvent *event) {
  const QMimeData *mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    QList<QUrl> urls = mimeData->urls();
    if (!urls.isEmpty()) {
      QString filePath = urls.first().toLocalFile();
      logMessage(tr("文件已放置：%1").arg(filePath));
      openImageFile(filePath); // Call the updated file opening function
      event->acceptProposedAction();
      return;
    }
  }
  event->ignore();
}

// Open Image File (Uses ImageHandler)
void MainWindow::openImageFile(const QString &filePath) {
  if (filePath.isEmpty()) {
    logMessage(tr("图像打开取消或提供了无效路径。"));
    return;
  }

  logMessage(tr("正在通过 MainWindow 尝试打开图像：%1").arg(filePath));
  QCoreApplication::processEvents(); // Update UI to show log message

  // 添加到图像列表
  QFileInfo fileInfo(filePath);
  
  // 检查是否已经加载过此图像
  for(int i = 0; i < m_loadedImages.size(); ++i) {
      if(m_loadedImages[i].absoluteFilePath() == fileInfo.absoluteFilePath()) {
          // 已经存在，直接加载显示
          loadAndDisplayImage(i);
          return;
      }
  }
  
  // 添加到列表并显示
  m_loadedImages.append(fileInfo);
  loadAndDisplayImage(m_loadedImages.size() - 1);
}

// 创建质量评估面板
void MainWindow::createQualityPanel() {
    // 创建一个新的 QWidget 作为中央窗口
    QWidget *centralQualityWidget = new QWidget(this);
    QVBoxLayout *qualityLayout = new QVBoxLayout(centralQualityWidget);
    
    // 创建局部质量评价组
    QGroupBox *localQualityBox = new QGroupBox(tr("局部质量评价"), centralQualityWidget);
    QVBoxLayout *localLayout = new QVBoxLayout(localQualityBox);
    
    QCheckBox *checkBoxISLR = new QCheckBox(tr("积分旁瓣比 (ISLR)"), localQualityBox);
    QCheckBox *checkBoxPSLR = new QCheckBox(tr("峰值旁瓣比 (PSLR)"), localQualityBox);
    QCheckBox *checkBoxRangeResolution = new QCheckBox(tr("距离模糊度"), localQualityBox);
    QCheckBox *checkBoxAzimuthResolution = new QCheckBox(tr("方位模糊度"), localQualityBox);
    
    localLayout->addWidget(checkBoxISLR);
    localLayout->addWidget(checkBoxPSLR);
    localLayout->addWidget(checkBoxRangeResolution);
    localLayout->addWidget(checkBoxAzimuthResolution);
    
    // 创建全局质量评价组
    QGroupBox *globalQualityBox = new QGroupBox(tr("全局质量评价"), centralQualityWidget);
    QVBoxLayout *globalLayout = new QVBoxLayout(globalQualityBox);
    
    QCheckBox *checkBoxSNR = new QCheckBox(tr("信噪比分析 (SNR)"), globalQualityBox);
    QCheckBox *checkBoxInfoContent = new QCheckBox(tr("信息熵分析"), globalQualityBox);
    QCheckBox *checkBoxClarity = new QCheckBox(tr("清晰度分析"), globalQualityBox);
    QCheckBox *checkBoxRadiometricAccuracy = new QCheckBox(tr("辐射精度"), globalQualityBox);
    QCheckBox *checkBoxGLCM = new QCheckBox(tr("GLCM 纹理特征"), globalQualityBox);
    QCheckBox *checkBoxNESZ = new QCheckBox(tr("噪声等效后向散射系数 (NESZ)"), globalQualityBox);
    QCheckBox *checkBoxRadiometricResolution = new QCheckBox(tr("辐射分辨率"), globalQualityBox);
    QCheckBox *checkBoxENL = new QCheckBox(tr("等效视数 (ENL)"), globalQualityBox);
    
    globalLayout->addWidget(checkBoxSNR);
    globalLayout->addWidget(checkBoxInfoContent);
    globalLayout->addWidget(checkBoxClarity);
    globalLayout->addWidget(checkBoxRadiometricAccuracy);
    globalLayout->addWidget(checkBoxGLCM);
    globalLayout->addWidget(checkBoxNESZ);
    globalLayout->addWidget(checkBoxRadiometricResolution);
    globalLayout->addWidget(checkBoxENL);
    
    // 添加全选选项
    QCheckBox *checkBoxSelectAll = new QCheckBox(tr("全选"), centralQualityWidget);
    
    // 添加开始分析按钮
    QPushButton *startAnalysisButton = new QPushButton(tr("开始分析"), centralQualityWidget);
    
    // 添加进度条
    QProgressBar *progressBar = new QProgressBar(centralQualityWidget);
    progressBar->setValue(0);
    
    // 将所有组件添加到主布局
    qualityLayout->addWidget(localQualityBox);
    qualityLayout->addWidget(globalQualityBox);
    qualityLayout->addWidget(checkBoxSelectAll);
    qualityLayout->addWidget(startAnalysisButton);
    qualityLayout->addWidget(progressBar);
    
    // 设置布局
    centralQualityWidget->setLayout(qualityLayout);
    
    // 将小部件添加到中央窗口右侧
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(ui->splitter_main);
    mainLayout->addWidget(centralQualityWidget);
    
    // 设置主布局
    delete ui->centralwidget->layout();
    ui->centralwidget->setLayout(mainLayout);
    
    // 保存对新创建控件的引用，供其他函数使用
    m_checkBoxSelectAll = checkBoxSelectAll;
    m_startAnalysisButton = startAnalysisButton;
    m_progressBar = progressBar;
    m_checkBoxSNR = checkBoxSNR;
    m_checkBoxInfoContent = checkBoxInfoContent;
    m_checkBoxClarity = checkBoxClarity;
    m_checkBoxRadiometricAccuracy = checkBoxRadiometricAccuracy;
    m_checkBoxGLCM = checkBoxGLCM;
    m_checkBoxNESZ = checkBoxNESZ;
    m_checkBoxRadiometricResolution = checkBoxRadiometricResolution;
    m_checkBoxENL = checkBoxENL;
    m_checkBoxISLR = checkBoxISLR;
    m_checkBoxPSLR = checkBoxPSLR;
    m_checkBoxRangeResolution = checkBoxRangeResolution;
    m_checkBoxAzimuthResolution = checkBoxAzimuthResolution;
    
    // 连接信号与槽
    connect(checkBoxSelectAll, SIGNAL(toggled(bool)), this, SLOT(on_checkBoxSelectAll_toggled(bool)));
    connect(startAnalysisButton, SIGNAL(clicked()), this, SLOT(on_startAnalysisButton_clicked()));
    
    // 初始化 UI 状态
    startAnalysisButton->setEnabled(false);
    checkBoxSelectAll->setChecked(false);
}

// 更新图像列表显示
void MainWindow::updateImageList() {
    QListWidget *imageListWidget = ui->dockWidget_imageList->findChild<QListWidget*>("imageListWidget");
    if(!imageListWidget) return;
    
    imageListWidget->clear();
    
    for(int i = 0; i < m_loadedImages.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(m_loadedImages[i].fileName());
        item->setData(Qt::UserRole, i); // 存储图像索引
        
        // 如果是当前图像，设置为选中状态
        if(i == m_currentImageIndex) {
            item->setSelected(true);
        }
        
        imageListWidget->addItem(item);
    }
}

// 加载并显示指定索引的图像
void MainWindow::loadAndDisplayImage(int index) {
    if(index < 0 || index >= m_loadedImages.size()) {
        return;
    }
    
    QString filePath = m_loadedImages[index].absoluteFilePath();
    
    // 使用 ImageHandler 加载图像
    bool success = m_imageHandler.loadImage(filePath);
    
    if(success) {
        m_currentImageIndex = index;
        
        // 更新 UI
        statusBar()->showMessage(tr("已加载：%1 (%2, %3)")
                               .arg(m_imageHandler.getFilename())
                               .arg(m_imageHandler.getDimensionsString())
                               .arg(m_imageHandler.getDataTypeString()));
                               
        // 显示图像
        QPixmap pixmap = m_imageHandler.getDisplayPixmap(ui->imageDisplayLabel->size());
        if(!pixmap.isNull()) {
            ui->imageDisplayLabel->setPixmap(pixmap);
            ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
            ui->imageDisplayLabel->setStyleSheet("");
            logMessage(tr("图像成功显示。"));
            m_startAnalysisButton->setEnabled(true);
        } else {
            logMessage(tr("错误：ImageHandler 提供了空 QPixmap。"));
            ui->imageDisplayLabel->setText(tr("错误：显示失败"));
            ui->imageDisplayLabel->setAlignment(Qt::AlignCenter);
            ui->imageDisplayLabel->setStyleSheet("QLabel { color: red; }");
            closeCurrentImage();
            QMessageBox::critical(this, tr("显示错误"), tr("加载后准备图像显示失败。"));
        }
    } else {
        logMessage(tr("ImageHandler 无法加载图像。"));
        QMessageBox::critical(this, tr("图像加载错误"), 
            tr("无法打开或读取所选图像文件。\n路径：%1\n请检查文件完整性、权限，并查看日志以获取详情。")
                .arg(filePath));
        closeCurrentImage();
    }
    
    // 更新图像列表，高亮显示当前图像
    updateImageList();
}

// Start Analysis Button Clicked (MODIFIED)
void MainWindow::on_startAnalysisButton_clicked() {
  if (!m_imageHandler.isValid()) {
    QMessageBox::warning(
        this, tr("分析未启动"),
        tr("请在开始分析前打开有效的图像文件。"));
    logMessage(tr("用户点击了分析按钮，但未加载有效图像。"));
    return;
  }

  logMessage(tr("用户启动分析过程。"));
  m_progressBar->setValue(0);

  ui->overviewResultsTextEdit->clear();
  ui->method1ResultsTextEdit->clear();
  ui->method2ResultsTextEdit->clear();
  ui->method3ResultsTextEdit->clear();
  ui->method4ResultsTextEdit->clear();
  ui->method5ResultsTextEdit->clear();

  // --- Get image data from ImageHandler ---
  const cv::Mat &imageData = m_imageHandler.getImageData();

  // --- Define analysis tasks using standalone functions ---
  int totalSteps = 0;
  // Store pairs of { Task Function, Result UI Update Function }
  std::vector<std::pair<std::function<AnalysisResult()>,
                        std::function<void(const AnalysisResult &)>>> tasks;

  // Add SNR Analysis if selected
  if (m_checkBoxSNR->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performSNRAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->method1ResultsTextEdit->setText(result.detailedLog);
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }

  // Add Information Content Analysis if selected
  if (m_checkBoxInfoContent->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performInfoContentAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->method2ResultsTextEdit->setText(result.detailedLog);
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }

  // Add Clarity Analysis if selected
  if (m_checkBoxClarity->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performClarityAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->method3ResultsTextEdit->setText(result.detailedLog);
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }

  // Add Radiometric Analysis if selected
  if (m_checkBoxRadiometricAccuracy->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performRadiometricAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->method4ResultsTextEdit->setText(result.detailedLog);
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }

  // Add GLCM Analysis if selected
  if (m_checkBoxGLCM->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performGLCMAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->method5ResultsTextEdit->setText(result.detailedLog);
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }
  
  // 添加积分旁瓣比 (ISLR) 分析
  if (m_checkBoxISLR->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performISLRAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->method6ResultsTextEdit->setText(result.detailedLog);
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }
  
  // 添加峰值旁瓣比 (PSLR) 分析
  if (m_checkBoxPSLR->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performPSLRAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->method7ResultsTextEdit->setText(result.detailedLog);
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }
  
  // 添加距离模糊度分析
  if (m_checkBoxRangeResolution->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performRangeResolutionAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->method8ResultsTextEdit->setText(result.detailedLog);
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }
  
  // 添加方位模糊度分析
  if (m_checkBoxAzimuthResolution->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performAzimuthResolutionAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->method9ResultsTextEdit->setText(result.detailedLog);
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }
  
  // 添加噪声等效后向散射系数 (NESZ) 分析
  if (m_checkBoxNESZ->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performNESZAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }
  
  // 添加辐射分辨率分析
  if (m_checkBoxRadiometricResolution->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performRadiometricResolutionAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }
  
  // 添加等效视数 (ENL) 分析
  if (m_checkBoxENL->isChecked()) {
    tasks.push_back(
        {[&imageData]() { return performENLAnalysis(imageData); },
         [this](const AnalysisResult &result) {
           ui->overviewResultsTextEdit->append(result.overviewSummary);
         }});
    totalSteps++;
  }

  // Make sure at least one analysis is selected
  if (tasks.empty()) {
    QMessageBox::warning(this, tr("未选择分析方法"),
                        tr("请至少选择一种分析方法。"));
    logMessage(tr("未选择任何分析方法。"));
    return;
  }

  // Reset progress bar
  m_progressBar->setMaximum(totalSteps);
  m_progressBar->setValue(0);

  // Run analysis tasks
  logMessage(tr("开始执行 %1 个分析任务...").arg(tasks.size()));
  
  int completedSteps = 0;
  for (const auto &task : tasks) {
    // Step 1: Run analysis function
    AnalysisResult result = task.first();
    
    // Step 2: Update UI with results
    if (result.success) {
      task.second(result);
      logMessage(tr("分析完成：%1").arg(result.analysisName));
    } else {
      // Skip updating UI for failed analysis
      logMessage(tr("分析失败：%1 - 详情请参见日志").arg(result.analysisName));
    }
    
    // Update progress bar
    completedSteps++;
    m_progressBar->setValue(completedSteps);
    QCoreApplication::processEvents(); // Keep UI responsive
  }
  
  logMessage(tr("所有分析任务执行完毕。"));
}

// 全选/取消全选
void MainWindow::on_checkBoxSelectAll_toggled(bool checked) {
  QWidget *centralQualityWidget = ui->centralwidget->findChild<QWidget*>();
  if(!centralQualityWidget) return;
  
  // 查找局部质量评价组和全局质量评价组
  QGroupBox *localQualityBox = centralQualityWidget->findChild<QGroupBox*>(QString(), Qt::FindDirectChildrenOnly);
  QGroupBox *globalQualityBox = NULL;
  
  if(localQualityBox) {
    // 找到第二个组框（全局质量）
    QList<QGroupBox*> groupBoxes = centralQualityWidget->findChildren<QGroupBox*>(QString(), Qt::FindDirectChildrenOnly);
    if(groupBoxes.size() > 1) {
      globalQualityBox = groupBoxes[1];
    }
  }
  
  // 设置局部质量选项
  if(localQualityBox) {
    QList<QCheckBox*> localCheckBoxes = localQualityBox->findChildren<QCheckBox*>();
    for(QCheckBox *checkBox : localCheckBoxes) {
      checkBox->setChecked(checked);
    }
  }
  
  // 设置全局质量选项
  if(globalQualityBox) {
    QList<QCheckBox*> globalCheckBoxes = globalQualityBox->findChildren<QCheckBox*>();
    for(QCheckBox *checkBox : globalCheckBoxes) {
      checkBox->setChecked(checked);
    }
  }
  
  logMessage(checked ? tr("已选择所有分析方法。") : tr("已取消选择所有分析方法。"));
}

// Menu Action: Open Image (Calls modified openImageFile)
void MainWindow::on_actionOpenImage_triggered() {
  QString filePath =
      QFileDialog::getOpenFileName(this, tr("打开 SAR 图像文件"), QString(),
                                   tr("支持的图像格式 (*.tif *.tiff "
                                      "*.img *.hdr *.dat);;所有文件 (*.*)"));

  if (!filePath.isEmpty()) {
    logMessage(tr("通过文件菜单打开图像：%1").arg(filePath));
    openImageFile(filePath); // Call the updated file opening function
  } else {
    logMessage(tr("用户取消通过文件菜单打开图像。"));
  }
}

// 导出 PDF 报告
void MainWindow::exportReportToPDF() {
  if (!m_imageHandler.isValid()) {
    QMessageBox::warning(this, tr("无法导出"),
                        tr("请先打开图像并执行分析才能导出报告。"));
    return;
  }
  
  QString filePath = QFileDialog::getSaveFileName(this, 
                                                 tr("保存 PDF 报告"), 
                                                 QString("%1_分析报告.pdf").arg(m_imageHandler.getFilename()),
                                                 tr("PDF 文件 (*.pdf)"));
  if (filePath.isEmpty()) {
    logMessage(tr("导出 PDF 操作被用户取消"));
    return;
  }
  
  // 创建打印机对象用于 PDF 生成
  QPrinter printer(QPrinter::HighResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setOutputFileName(filePath);
  
  // 创建文档对象
  QTextDocument document;
  QString html = "<html><body>";
  html += QString("<h1>SAR 图像质量评估报告</h1>");
  html += QString("<p>图像文件：%1</p>").arg(m_imageHandler.getFilename());
  html += QString("<p>图像尺寸：%1</p>").arg(m_imageHandler.getDimensionsString());
  html += QString("<p>数据类型：%1</p>").arg(m_imageHandler.getDataTypeString());
  html += "<h2>分析结果概述</h2>";
  html += QString("<pre>%1</pre>").arg(ui->overviewResultsTextEdit->toPlainText());
  
  // 添加各项详细分析
  html += "<h2>详细分析结果</h2>";
  if (!ui->method1ResultsTextEdit->toPlainText().isEmpty()) {
    html += "<h3>信噪比分析</h3>";
    html += QString("<pre>%1</pre>").arg(ui->method1ResultsTextEdit->toPlainText());
  }
  if (!ui->method2ResultsTextEdit->toPlainText().isEmpty()) {
    html += "<h3>信息熵分析</h3>";
    html += QString("<pre>%1</pre>").arg(ui->method2ResultsTextEdit->toPlainText());
  }
  if (!ui->method3ResultsTextEdit->toPlainText().isEmpty()) {
    html += "<h3>清晰度分析</h3>";
    html += QString("<pre>%1</pre>").arg(ui->method3ResultsTextEdit->toPlainText());
  }
  if (!ui->method4ResultsTextEdit->toPlainText().isEmpty()) {
    html += "<h3>辐射精度</h3>";
    html += QString("<pre>%1</pre>").arg(ui->method4ResultsTextEdit->toPlainText());
  }
  if (!ui->method5ResultsTextEdit->toPlainText().isEmpty()) {
    html += "<h3>GLCM 纹理特征</h3>";
    html += QString("<pre>%1</pre>").arg(ui->method5ResultsTextEdit->toPlainText());
  }
  
  html += "<p>报告生成时间：" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</p>";
  html += "</body></html>";
  
  document.setHtml(html);
  document.print(&printer);
  
  logMessage(tr("PDF 报告已导出到：%1").arg(filePath));
  QMessageBox::information(this, tr("导出成功"), tr("PDF 报告已成功导出"));
}

// 导出 TXT 报告
void MainWindow::exportReportToTXT() {
  if (!m_imageHandler.isValid()) {
    QMessageBox::warning(this, tr("无法导出"),
                        tr("请先打开图像并执行分析才能导出报告。"));
    return;
  }
  
  QString filePath = QFileDialog::getSaveFileName(this, 
                                                 tr("保存文本报告"), 
                                                 QString("%1_分析报告.txt").arg(m_imageHandler.getFilename()),
                                                 tr("文本文件 (*.txt)"));
  if (filePath.isEmpty()) {
    logMessage(tr("导出 TXT 操作被用户取消"));
    return;
  }
  
  // 创建并打开文件
  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::critical(this, tr("导出错误"), tr("无法创建文件：%1").arg(filePath));
    logMessage(tr("导出 TXT 报告失败：无法创建文件"));
    return;
  }
  
  QTextStream out(&file);
  #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  out.setCodec("UTF-8");  // Qt5 及更早版本
  #else
  out.setEncoding(QStringConverter::Utf8);  // Qt6 版本
  #endif
  
  // 写入报告内容
  out << "SAR 图像质量评估报告\n";
  out << "====================\n\n";
  out << "图像文件：" << m_imageHandler.getFilename() << "\n";
  out << "图像尺寸：" << m_imageHandler.getDimensionsString() << "\n";
  out << "数据类型：" << m_imageHandler.getDataTypeString() << "\n\n";
  
  out << "分析结果概述\n";
  out << "------------\n";
  out << ui->overviewResultsTextEdit->toPlainText() << "\n\n";
  
  // 添加各项详细分析
  out << "详细分析结果\n";
  out << "------------\n";
  if (!ui->method1ResultsTextEdit->toPlainText().isEmpty()) {
    out << "信噪比分析:\n";
    out << ui->method1ResultsTextEdit->toPlainText() << "\n\n";
  }
  if (!ui->method2ResultsTextEdit->toPlainText().isEmpty()) {
    out << "信息熵分析:\n";
    out << ui->method2ResultsTextEdit->toPlainText() << "\n\n";
  }
  if (!ui->method3ResultsTextEdit->toPlainText().isEmpty()) {
    out << "清晰度分析:\n";
    out << ui->method3ResultsTextEdit->toPlainText() << "\n\n";
  }
  if (!ui->method4ResultsTextEdit->toPlainText().isEmpty()) {
    out << "辐射精度:\n";
    out << ui->method4ResultsTextEdit->toPlainText() << "\n\n";
  }
  if (!ui->method5ResultsTextEdit->toPlainText().isEmpty()) {
    out << "GLCM 纹理特征:\n";
    out << ui->method5ResultsTextEdit->toPlainText() << "\n\n";
  }
  
  out << "报告生成时间：" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
  
  file.close();
  logMessage(tr("TXT 报告已导出到：%1").arg(filePath));
  QMessageBox::information(this, tr("导出成功"), tr("文本报告已成功导出"));
}

// 选择评估区域
void MainWindow::selectAssessmentRegion() {
  if (!m_imageHandler.isValid()) {
    QMessageBox::warning(this, tr("无法选择区域"),
                        tr("请先打开有效的图像文件。"));
    return;
  }
  
  logMessage(tr("开始选择图像评估区域..."));
  // 这里需要实现区域选择功能，可能需要使用 QRubberBand 或其他绘图方式
  // 目前先显示一个消息，表示功能正在开发中
  QMessageBox::information(this, tr("功能开发中"),
                          tr("区域选择功能正在开发中，请稍后再试。"));
}

// 缩放图像
void MainWindow::zoomImage(double factor) {
  // 这里需要实现图像缩放功能
  // 可能需要保存当前缩放级别，然后根据 factor 调整
  logMessage(tr("图像缩放功能正在开发中 (factor: %1)").arg(factor));
  QMessageBox::information(this, tr("功能开发中"),
                          tr("图像缩放功能正在开发中，请稍后再试。"));
}

// 实现所有槽函数

// PDF 导出按钮
void MainWindow::on_pushButton_exportPDF_clicked() {
  logMessage(tr("用户点击了导出 PDF 按钮"));
  exportReportToPDF();
}

// TXT 导出按钮
void MainWindow::on_pushButton_exportTXT_clicked() {
  logMessage(tr("用户点击了导出 TXT 按钮"));
  exportReportToTXT();
}

// 菜单导出 PDF
void MainWindow::on_actionExportPDF_triggered() {
  logMessage(tr("用户通过菜单选择了导出 PDF"));
  exportReportToPDF();
}

// 菜单导出 TXT
void MainWindow::on_actionExportTXT_triggered() {
  logMessage(tr("用户通过菜单选择了导出 TXT"));
  exportReportToTXT();
}

// 关闭图像
void MainWindow::on_actionCloseImage_triggered() {
  logMessage(tr("关闭当前图像"));
  closeCurrentImage();
}

// Start Assessment from toolbar/menu (should call the same code as startAnalysisButton)
void MainWindow::on_actionStartAssessment_triggered() {
  logMessage(tr("用户从工具栏/菜单启动分析。"));
  // Simply call the existing button click handler
  on_startAnalysisButton_clicked();
}

// 选择评估区域
void MainWindow::on_actionSelectAssessmentRegion_triggered() {
  logMessage(tr("通过菜单触发区域选择"));
  selectAssessmentRegion();
}

// 放大
void MainWindow::on_actionZoomIn_triggered() {
  logMessage(tr("图像放大"));
  zoomImage(1.25); // 放大 25%
}

// 缩小
void MainWindow::on_actionZoomOut_triggered() {
  logMessage(tr("图像缩小"));
  zoomImage(0.8); // 缩小 20%
}

// 适合窗口
void MainWindow::on_actionFitToWindow_triggered() {
  logMessage(tr("图像适合窗口显示"));
  // 实现图像适合窗口大小显示
  QMessageBox::information(this, tr("功能开发中"),
                          tr("适合窗口功能正在开发中，请稍后再试。"));
}

// 漫游模式
void MainWindow::on_actionPan_toggled(bool checked) {
  logMessage(tr("漫游模式：%1").arg(checked ? "开启" : "关闭"));
  // 实现图像漫游模式
  QMessageBox::information(this, tr("功能开发中"),
                          tr("漫游模式功能正在开发中，请稍后再试。"));
}

// 工具栏保存报告
void MainWindow::on_actionSaveReport_toolbar_triggered() {
  logMessage(tr("通过工具栏保存报告"));
  
  // 显示选择导出格式对话框
  QMessageBox msgBox;
  msgBox.setWindowTitle(tr("选择报告格式"));
  msgBox.setText(tr("请选择导出报告的格式："));
  QPushButton *pdfButton = msgBox.addButton(tr("PDF"), QMessageBox::ActionRole);
  QPushButton *txtButton = msgBox.addButton(tr("文本"), QMessageBox::ActionRole);
  msgBox.addButton(QMessageBox::Cancel);
  
  msgBox.exec();
  
  if (msgBox.clickedButton() == pdfButton) {
    exportReportToPDF();
  } else if (msgBox.clickedButton() == txtButton) {
    exportReportToTXT();
  }
}

// 全屏显示
void MainWindow::on_actionFullScreen_triggered() {
  if (isFullScreen()) {
    showNormal();
    logMessage(tr("退出全屏显示"));
  } else {
    showFullScreen();
    logMessage(tr("进入全屏显示"));
  }
}

// 图像列表项点击
void MainWindow::on_imageListWidget_itemClicked(QListWidgetItem *item) {
    if(!item) return;
    
    int index = item->data(Qt::UserRole).toInt();
    if(index != m_currentImageIndex) {
        loadAndDisplayImage(index);
    }
}

// 图像列表显示切换
void MainWindow::on_actionToggleImageList_toggled(bool checked) {
    ui->dockWidget_imageList->setVisible(checked);
}

// 新增分析算法触发函数
void MainWindow::on_actionISLR_triggered() {
    if(!m_imageHandler.isValid()) {
        QMessageBox::warning(this, tr("分析未启动"), tr("请在开始分析前打开有效的图像文件。"));
        return;
    }
    
    logMessage(tr("启动积分旁瓣比 (ISLR) 分析..."));
    
    const cv::Mat &imageData = m_imageHandler.getImageData();
    AnalysisResult result = performISLRAnalysis(imageData);
    
    if(result.success) {
        ui->method6ResultsTextEdit->setText(result.detailedLog);
        ui->overviewResultsTextEdit->append(result.overviewSummary);
        logMessage(tr("积分旁瓣比分析完成。"));
    } else {
        logMessage(tr("积分旁瓣比分析失败。"));
        QMessageBox::warning(this, tr("分析失败"), tr("积分旁瓣比分析失败，请查看日志了解详情。"));
    }
}

void MainWindow::on_actionPSLR_triggered() {
    if(!m_imageHandler.isValid()) {
        QMessageBox::warning(this, tr("分析未启动"), tr("请在开始分析前打开有效的图像文件。"));
        return;
    }
    
    logMessage(tr("启动峰值旁瓣比 (PSLR) 分析..."));
    
    const cv::Mat &imageData = m_imageHandler.getImageData();
    AnalysisResult result = performPSLRAnalysis(imageData);
    
    if(result.success) {
        ui->method7ResultsTextEdit->setText(result.detailedLog);
        ui->overviewResultsTextEdit->append(result.overviewSummary);
        logMessage(tr("峰值旁瓣比分析完成。"));
    } else {
        logMessage(tr("峰值旁瓣比分析失败。"));
        QMessageBox::warning(this, tr("分析失败"), tr("峰值旁瓣比分析失败，请查看日志了解详情。"));
    }
}

void MainWindow::on_actionRangeResolution_triggered() {
    if(!m_imageHandler.isValid()) {
        QMessageBox::warning(this, tr("分析未启动"), tr("请在开始分析前打开有效的图像文件。"));
        return;
    }
    
    logMessage(tr("启动距离模糊度分析..."));
    
    const cv::Mat &imageData = m_imageHandler.getImageData();
    AnalysisResult result = performRangeResolutionAnalysis(imageData);
    
    if(result.success) {
        ui->method8ResultsTextEdit->setText(result.detailedLog);
        ui->overviewResultsTextEdit->append(result.overviewSummary);
        logMessage(tr("距离模糊度分析完成。"));
    } else {
        logMessage(tr("距离模糊度分析失败。"));
        QMessageBox::warning(this, tr("分析失败"), tr("距离模糊度分析失败，请查看日志了解详情。"));
    }
}

void MainWindow::on_actionAzimuthResolution_triggered() {
    if(!m_imageHandler.isValid()) {
        QMessageBox::warning(this, tr("分析未启动"), tr("请在开始分析前打开有效的图像文件。"));
        return;
    }
    
    logMessage(tr("启动方位模糊度分析..."));
    
    const cv::Mat &imageData = m_imageHandler.getImageData();
    AnalysisResult result = performAzimuthResolutionAnalysis(imageData);
    
    if(result.success) {
        ui->method9ResultsTextEdit->setText(result.detailedLog);
        ui->overviewResultsTextEdit->append(result.overviewSummary);
        logMessage(tr("方位模糊度分析完成。"));
    } else {
        logMessage(tr("方位模糊度分析失败。"));
        QMessageBox::warning(this, tr("分析失败"), tr("方位模糊度分析失败，请查看日志了解详情。"));
    }
}

void MainWindow::on_actionNESZ_triggered() {
    if(!m_imageHandler.isValid()) {
        QMessageBox::warning(this, tr("分析未启动"), tr("请在开始分析前打开有效的图像文件。"));
        return;
    }
    
    logMessage(tr("启动噪声等效后向散射系数 (NESZ) 分析..."));
    
    const cv::Mat &imageData = m_imageHandler.getImageData();
    AnalysisResult result = performNESZAnalysis(imageData);
    
    if(result.success) {
        ui->overviewResultsTextEdit->append(result.overviewSummary);
        logMessage(tr("噪声等效后向散射系数分析完成。"));
    } else {
        logMessage(tr("噪声等效后向散射系数分析失败。"));
        QMessageBox::warning(this, tr("分析失败"), tr("噪声等效后向散射系数分析失败，请查看日志了解详情。"));
    }
}

void MainWindow::on_actionRadiometricResolution_triggered() {
    if(!m_imageHandler.isValid()) {
        QMessageBox::warning(this, tr("分析未启动"), tr("请在开始分析前打开有效的图像文件。"));
        return;
    }
    
    logMessage(tr("启动辐射分辨率分析..."));
    
    const cv::Mat &imageData = m_imageHandler.getImageData();
    AnalysisResult result = performRadiometricResolutionAnalysis(imageData);
    
    if(result.success) {
        ui->overviewResultsTextEdit->append(result.overviewSummary);
        logMessage(tr("辐射分辨率分析完成。"));
    } else {
        logMessage(tr("辐射分辨率分析失败。"));
        QMessageBox::warning(this, tr("分析失败"), tr("辐射分辨率分析失败，请查看日志了解详情。"));
    }
}

void MainWindow::on_actionENL_triggered() {
    if(!m_imageHandler.isValid()) {
        QMessageBox::warning(this, tr("分析未启动"), tr("请在开始分析前打开有效的图像文件。"));
        return;
    }
    
    logMessage(tr("启动等效视数 (ENL) 分析..."));
    
    const cv::Mat &imageData = m_imageHandler.getImageData();
    AnalysisResult result = performENLAnalysis(imageData);
    
    if(result.success) {
        ui->overviewResultsTextEdit->append(result.overviewSummary);
        logMessage(tr("等效视数分析完成。"));
    } else {
        logMessage(tr("等效视数分析失败。"));
        QMessageBox::warning(this, tr("分析失败"), tr("等效视数分析失败，请查看日志了解详情。"));
    }
}

// --- Member function definitions for analysis are removed from here ---
// --- They now exist as standalone functions in analysis_*.cpp files ---
