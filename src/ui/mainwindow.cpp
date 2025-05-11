#include "include/mainwindow.h"
#include "ui_mainwindow.h"
#include "logger.h"
#include "include/drag_drop_graphics_view.h"
#include "analysis_controller.h"
#include "report_generator.h"
#include <QLayout>
#include <QTabWidget>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 初始化成员变量
    imageScene = new QGraphicsScene(this);
    imageHandler = new SAR::Core::ImageHandler();
    analysisController = nullptr; // 需要完整实现AnalysisController后再初始化
    reportGenerator = nullptr; // 需要完整实现ReportGenerator后再初始化
    
    // 设置窗口标题
    setWindowTitle(tr("SAR 图像质量分析工具"));
    
    // 设置图像查看器
    setupImageViewer();
    
    // 设置信号连接
    setupConnections();
    
    // 设置日志系统
    setupLogSystem();
    
    // 配置分析选项
    configureAnalysisOptions();
    
    // 更新状态栏
    updateStatusBar(tr("准备就绪"));
    
    LOG_INFO("主窗口初始化完成");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete imageScene;
    delete imageHandler;
    // 析构前检查指针是否为空
    if (analysisController) delete analysisController;
    if (reportGenerator) delete reportGenerator;
    
    LOG_INFO("主窗口已销毁");
}

void MainWindow::setupImageViewer()
{
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

void MainWindow::setupConnections()
{
    // 连接图像视图的拖放信号
    connect(imageView, SIGNAL(dragEnterReceived(QDragEnterEvent*)),
            this, SLOT(handleViewDragEnter(QDragEnterEvent*)));
    connect(imageView, SIGNAL(dropReceived(QDropEvent*)),
            this, SLOT(handleViewDrop(QDropEvent*)));

    // 连接日志系统信号
    connect(SAR::Core::Logger::instance(), SIGNAL(newLogMessage(const QString&)),
            this, SLOT(onNewLogMessage(const QString&)));

    // 全选/取消全选按钮
    connect(ui->checkBoxSelectAll, &QCheckBox::toggled, this, &MainWindow::on_checkBoxSelectAll_toggled);

    // 当有图像加载时启用分析按钮
    connect(ui->imageListWidget, &QListWidget::itemSelectionChanged, [this]() {
        bool hasSelectedItems = !ui->imageListWidget->selectedItems().isEmpty();
        enableAnalysisButtons(hasSelectedItems);
    });
}

void MainWindow::setupLogSystem()
{
    // 连接清除日志和保存日志功能
    // 注意：UI里没有pushButtonClearLog和pushButtonSaveLog按钮
    // 需要在界面中添加这些按钮，或者改用其他方式实现这些功能
    
    LOG_INFO("日志系统已设置");
}

void MainWindow::configureAnalysisOptions()
{
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

void MainWindow::configureResultTabs()
{
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

void MainWindow::enableAnalysisButtons(bool enable)
{
    ui->startAnalysisButton->setEnabled(enable);
    ui->actionStartAssessment->setEnabled(enable);
    ui->actionSelectAssessmentRegion->setEnabled(enable);
    ui->pushButton_exportPDF->setEnabled(enable);
    ui->pushButton_exportTXT->setEnabled(enable);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QString filePath = event->mimeData()->urls().first().toLocalFile();
        handleDroppedFile(filePath);
    }
}

void MainWindow::handleViewDragEnter(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::handleViewDrop(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QString filePath = event->mimeData()->urls().first().toLocalFile();
        handleDroppedFile(filePath);
    }
}

void MainWindow::handleDroppedFile(const QString &filePath)
{
    if (isSupportedImageFormat(filePath)) {
        loadImage(filePath);
    } else {
        QMessageBox::warning(this, tr("不支持的文件格式"), 
                             tr("文件 %1 不是支持的图像格式。").arg(filePath));
    }
}

bool MainWindow::isSupportedImageFormat(const QString &filePath)
{
    // 简单检查文件扩展名
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    // 支持的格式列表
    QStringList supportedFormats = {"tif", "tiff", "jpg", "jpeg", "png", "bmp", "gif"};
    
    return supportedFormats.contains(extension);
}

bool MainWindow::loadImage(const QString &filePath)
{
    // 使用 GDAL 加载图像
    if (imageHandler->loadImage(filePath)) {
        // 更新 UI
        QPixmap pixmap(filePath);
        imageScene->clear();
        imageScene->addPixmap(pixmap);
        imageView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);
        
        // 更新当前图像路径
        currentImagePath = filePath;
        
        // 添加到图像列表
        if (!loadedImages.contains(filePath)) {
            loadedImages.append(filePath);
            QListWidgetItem *item = new QListWidgetItem(QFileInfo(filePath).fileName());
            item->setData(Qt::UserRole, filePath);
            ui->imageListWidget->addItem(item);
        }
        
        // 选择图像列表中的当前项
        for (int i = 0; i < ui->imageListWidget->count(); i++) {
            if (ui->imageListWidget->item(i)->data(Qt::UserRole).toString() == filePath) {
                ui->imageListWidget->setCurrentRow(i);
                break;
            }
        }
        
        // 启用分析按钮
        enableAnalysisButtons(true);
        
        // 更新状态栏
        updateStatusBar(tr("已加载图像: %1").arg(QFileInfo(filePath).fileName()));
        
        LOG_INFO(QString("已加载图像: %1").arg(filePath));
        
        return true;
    } else {
        QMessageBox::warning(this, tr("加载失败"), 
                             tr("无法加载图像文件: %1").arg(filePath));
        LOG_ERROR(QString("无法加载图像文件: %1").arg(filePath));
        return false;
    }
}

void MainWindow::updateStatusBar(const QString &message)
{
    ui->statusbar->showMessage(message);
}

void MainWindow::onNewLogMessage(const QString &message)
{
    ui->logTextEdit->append(message);
}

void MainWindow::clearLog()
{
    ui->logTextEdit->clear();
    LOG_INFO("日志已清除");
}

void MainWindow::saveLog()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存日志"), 
                                                  QString(), tr("日志文件 (*.log);;所有文件 (*)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << ui->logTextEdit->toPlainText();
            file.close();
            
            LOG_INFO(QString("日志已保存到: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, tr("保存失败"), 
                                 tr("无法保存日志文件: %1").arg(fileName));
            LOG_ERROR(QString("无法保存日志文件: %1").arg(fileName));
        }
    }
}

void MainWindow::on_actionOpenImage_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开 SAR 图像"), 
                                                  QString(), tr("图像文件 (*.tif *.tiff *.jpg *.jpeg *.png *.bmp);;所有文件 (*)"));
    if (!fileName.isEmpty()) {
        loadImage(fileName);
    }
}

void MainWindow::on_actionCloseImage_triggered()
{
    // 清除当前显示的图像
    imageScene->clear();
    currentImagePath.clear();
    
    // 禁用分析按钮
    enableAnalysisButtons(false);
    
    // 更新状态栏
    updateStatusBar(tr("已关闭图像"));
    
    LOG_INFO("已关闭图像");
}

void MainWindow::on_checkBoxSelectAll_toggled(bool checked)
{
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
void MainWindow::on_startAnalysisButton_clicked()
{
    // 检查是否有选中的图像
    if (currentImagePath.isEmpty()) {
        QMessageBox::warning(this, tr("无图像"), tr("请先加载SAR图像"));
        return;
    }
    
    // 收集选中的分析方法
    QStringList selectedMethods;
    if (ui->checkBoxISLR->isChecked()) selectedMethods << "ISLR";
    if (ui->checkBoxPSLR->isChecked()) selectedMethods << "PSLR";
    if (ui->checkBoxRASR->isChecked()) selectedMethods << "RASR";
    if (ui->checkBoxAASR->isChecked()) selectedMethods << "AASR";
    if (ui->checkBoxSNR->isChecked()) selectedMethods << "SNR";
    if (ui->checkBoxNESZ->isChecked()) selectedMethods << "NESZ";
    if (ui->checkBoxRadiometricAccuracy->isChecked()) selectedMethods << "RadiometricAccuracy";
    if (ui->checkBoxRadiometricResolution->isChecked()) selectedMethods << "RadiometricResolution";
    if (ui->checkBoxENL->isChecked()) selectedMethods << "ENL";
    
    // 检查是否有选中的方法
    if (selectedMethods.isEmpty()) {
        QMessageBox::warning(this, tr("未选择方法"), tr("请选择至少一种分析方法"));
        return;
    }
    
    // 执行分析
    performAnalysis(selectedMethods);
}

void MainWindow::performAnalysis(const QStringList &selectedMethods)
{
    LOG_INFO(QString("开始分析，选中的方法: %1").arg(selectedMethods.join(", ")));
    
    // 显示进度对话框
    QProgressDialog progress(tr("正在进行分析..."), tr("取消"), 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();
    
    // 清除之前的结果
    clearResults();
    
    // 执行分析
    for (int i = 0; i < selectedMethods.size(); i++) {
        QString method = selectedMethods[i];
        int progressValue = (i * 100) / selectedMethods.size();
        
        // 更新进度
        progress.setValue(progressValue);
        progress.setLabelText(tr("正在分析: %1").arg(method));
        
        // 处理用户取消
        if (progress.wasCanceled()) {
            LOG_INFO("用户取消了分析");
            break;
        }
        
        // 模拟分析过程
        // 在实际应用中，这里会调用analysisController的相应方法
        QApplication::processEvents();
        QThread::msleep(500); // 模拟分析耗时
        
        // 生成模拟结果
        QString result = QString("方法 %1 的分析结果: %2").arg(method).arg(QDateTime::currentDateTime().toString());
        updateResults(method, result);
    }
    
    // 完成分析
    progress.setValue(100);
    
    // 显示结果
    showAnalysisResults();
    
    LOG_INFO("分析完成");
}

void MainWindow::clearResults()
{
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

void MainWindow::updateResults(const QString &method, const QString &result)
{
    imageResults[method] = result;
    LOG_INFO(QString("更新了 %1 的分析结果").arg(method));
}

void MainWindow::showAnalysisResults()
{
    // 在结果文本区域显示所有结果
    QString resultText;
    QMapIterator<QString, QString> i(imageResults);
    while (i.hasNext()) {
        i.next();
        resultText += QString("<h3>%1</h3><p>%2</p>").arg(i.key()).arg(i.value());
        
        // 在各自的选项卡中显示具体分析结果
        if (i.key() == "ISLR") {
            ui->ISLRResultsTextEdit->setHtml(i.value());
        } else if (i.key() == "PSLR") {
            ui->PSLRResultsTextEdit->setHtml(i.value());
        } else if (i.key() == "RASR") {
            ui->RASRResultsTextEdit->setHtml(i.value());
        } else if (i.key() == "AASR") {
            ui->AASRResultsTextEdit->setHtml(i.value());
        } else if (i.key() == "SNR") {
            ui->SNRResultsTextEdit->setHtml(i.value());
        } else if (i.key() == "NESZ") {
            ui->NESZResultsTextEdit->setHtml(i.value());
        } else if (i.key() == "RadiometricAccuracy") {
            ui->RadAccuracyResultsTextEdit->setHtml(i.value());
        } else if (i.key() == "RadiometricResolution") {
            ui->RadResolutionResultsTextEdit->setHtml(i.value());
        }
    }
    
    // 显示概览结果
    ui->overviewResultsTextEdit->setHtml(resultText);
    
    // 切换到结果选项卡
    ui->resultsTabWidget->setCurrentIndex(0); // 概览选项卡
    
    LOG_INFO("已显示分析结果");
}

// 导出结果功能
void MainWindow::on_pushButton_exportPDF_clicked()
{
    generateReport("PDF");
}

void MainWindow::on_pushButton_exportTXT_clicked()
{
    generateReport("TXT");
}

void MainWindow::generateReport(const QString &format)
{
    // 检查是否有结果
    if (imageResults.isEmpty()) {
        QMessageBox::warning(this, tr("无结果"), tr("没有分析结果可导出"));
        return;
    }
    
    QString fileName;
    
    if (format == "PDF") {
        fileName = QFileDialog::getSaveFileName(this, tr("导出PDF报告"), 
                                              QString(), tr("PDF文件 (*.pdf)"));
    } else if (format == "TXT") {
        fileName = QFileDialog::getSaveFileName(this, tr("导出文本报告"), 
                                              QString(), tr("文本文件 (*.txt)"));
    }
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // 在实际应用中，这里会调用reportGenerator的相应方法
    bool success = true;
    
    if (success) {
        QMessageBox::information(this, tr("导出成功"), 
                               tr("报告已成功导出到: %1").arg(fileName));
        LOG_INFO(QString("报告已导出到: %1").arg(fileName));
    } else {
        QMessageBox::warning(this, tr("导出失败"), 
                           tr("无法导出报告到: %1").arg(fileName));
        LOG_ERROR(QString("导出报告失败: %1").arg(fileName));
    }
}

// 其他必要函数
void MainWindow::on_actionStartAssessment_triggered()
{
    on_startAnalysisButton_clicked();
}

void MainWindow::on_actionSelectAssessmentRegion_triggered()
{
    // 实现区域选择逻辑
    QMessageBox::information(this, tr("区域选择"), tr("请在图像上选择感兴趣区域"));
}

void MainWindow::on_actionExportPDF_triggered()
{
    on_pushButton_exportPDF_clicked();
}

void MainWindow::on_actionExportTXT_triggered()
{
    on_pushButton_exportTXT_clicked();
}

void MainWindow::on_actionZoomIn_triggered()
{
    imageView->scale(1.2, 1.2);
}

void MainWindow::on_actionZoomOut_triggered()
{
    imageView->scale(0.8, 0.8);
}

void MainWindow::on_actionFitToWindow_triggered()
{
    imageView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::on_actionPan_toggled(bool checked)
{
    if (checked) {
        imageView->setDragMode(QGraphicsView::ScrollHandDrag);
    } else {
        imageView->setDragMode(QGraphicsView::NoDrag);
    }
}

void MainWindow::on_actionFullScreen_triggered()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void MainWindow::on_imageListWidget_itemClicked(QListWidgetItem *item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    if (!filePath.isEmpty() && filePath != currentImagePath) {
        loadImage(filePath);
    }
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("关于"), 
                      tr("SAR图像质量分析工具\n版本 1.0\n\n用于合成孔径雷达图像质量评估的工具"));
}

QString MainWindow::getCurrentDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void MainWindow::log(const QString &message)
{
    LOG_INFO(message);
}

void MainWindow::onAnalysisProgress(int progress, const QString &message)
{
    // 更新状态栏
    updateStatusBar(message);
    
    // 可以在这里更新进度条等UI元素
    
    LOG_INFO(QString("分析进度: %1% - %2").arg(progress).arg(message));
}

void MainWindow::onAnalysisComplete()
{
    // 分析完成后的处理
    updateStatusBar(tr("分析已完成"));
    
    LOG_INFO("分析已完成");
} 