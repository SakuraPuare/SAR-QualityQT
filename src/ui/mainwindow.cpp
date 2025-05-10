#include "include/mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QDir>
#include <QImageReader>
#include <QTime>
#include <QFileInfo>
#include <QGraphicsPixmapItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    hasSelectedRegion(false),
    imageHandler(new SAR::Core::ImageHandler([this](const QString &msg) { this->log(msg); }))
{
    ui->setupUi(this);
    
    // 设置接受拖放
    setAcceptDrops(true);
    
    // 初始化图像查看器
    setupImageViewer();
    
    // 配置信号和槽
    setupConnections();
    
    // 配置可用的分析选项
    configureAnalysisOptions();
    
    // 默认禁用分析按钮
    enableAnalysisButtons(false);
    
    // 设置状态栏初始消息
    updateStatusBar(tr("就绪"));
    
    // GDAL读取器初始化日志
    log(tr("GDAL图像处理器已初始化"));
}

MainWindow::~MainWindow()
{
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

void MainWindow::setupImageViewer()
{
    imageScene = new QGraphicsScene(this);
    
    // 创建一个自定义的DragDropGraphicsView
    imageView = new DragDropGraphicsView(this);
    imageView->setScene(imageScene);
    imageView->setDragMode(QGraphicsView::ScrollHandDrag);
    
    // 连接拖放信号
    connect(imageView, &DragDropGraphicsView::dragEnterReceived, this, &MainWindow::handleViewDragEnter);
    connect(imageView, &DragDropGraphicsView::dropReceived, this, &MainWindow::handleViewDrop);
    
    // 将新创建的DragDropGraphicsView添加到界面布局中
    if (ui->imageDisplayLabel) {
        // 获取占位符的父widget
        QWidget* parent = ui->imageDisplayLabel->parentWidget();
        if (parent) {
            // 获取占位符在布局中的位置
            QLayout* layout = parent->layout();
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
    
    log(tr("已设置图像视图(支持拖放功能)"));
}

void MainWindow::setupConnections()
{
    // 全选/取消全选按钮
    connect(ui->checkBoxSelectAll, &QCheckBox::toggled, [this](bool checked) {
        // 根据配置启用或禁用各个分析选项的代码...
    });
    
    // 当有图像加载时启用分析按钮
    connect(ui->imageListWidget, &QListWidget::itemSelectionChanged, [this]() {
        enableAnalysisButtons(!ui->imageListWidget->selectedItems().isEmpty());
    });
}

void MainWindow::enableAnalysisButtons(bool enable)
{
    ui->startAnalysisButton->setEnabled(enable);
    ui->actionStartAssessment->setEnabled(enable);
    ui->actionSelectAssessmentRegion->setEnabled(enable);
    ui->pushButton_exportPDF->setEnabled(enable);
    ui->pushButton_exportTXT->setEnabled(enable);
}

void MainWindow::on_actionOpenImage_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("打开 SAR 图像"), QDir::homePath(),
        tr("图像文件 (*.tif *.tiff *.jpg *.jpeg *.png *.bmp *.img *.ceos *.ers *.hdf5);;所有文件 (*)"));
        
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

bool MainWindow::loadImage(const QString &filePath)
{
    // 使用 GDAL (ImageHandler) 加载图像
    if (imageHandler->loadImage(filePath)) {
        // 从 ImageHandler 获取显示用的 QPixmap
        QPixmap pixmap = imageHandler->getDisplayPixmap(imageView->size());
        
        if (pixmap.isNull()) {
            QMessageBox::warning(this, tr("图像显示失败"),
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
        QGraphicsPixmapItem *item = imageScene->addPixmap(QPixmap::fromImage(image));
        imageScene->setSceneRect(item->boundingRect());
        
        // 记录日志
        log(tr("使用 Qt 加载图像：%1（GDAL 加载失败）").arg(QFileInfo(filePath).fileName()));
        
        // 调整视图以适应图像
        on_actionFitToWindow_triggered();
        
        return true;
    }
}

void MainWindow::on_actionCloseImage_triggered()
{
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
        ui->imageListWidget->setCurrentRow(qMin(row, ui->imageListWidget->count() - 1));
        loadImage(loadedImages.at(ui->imageListWidget->currentRow()));
    }
    
    log(tr("已关闭图像：%1").arg(QFileInfo(imagePath).fileName()));
}

void MainWindow::on_imageListWidget_itemClicked(QListWidgetItem *item)
{
    int row = ui->imageListWidget->row(item);
    if (row >= 0 && row < loadedImages.size()) {
        currentImagePath = loadedImages.at(row);
        loadImage(currentImagePath);
    }
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
    if (imageScene->items().isEmpty())
        return;
        
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

void MainWindow::updateStatusBar(const QString &message)
{
    statusBar()->showMessage(message);
}

void MainWindow::log(const QString &message)
{
    QString logMessage = QString("[%1] %2").arg(QTime::currentTime().toString("hh:mm:ss"), message);
    ui->logTextEdit->append(logMessage);
}

// 其他方法请保留原来的实现...

void MainWindow::clearResults()
{
    // 清除结果的代码...
}

void MainWindow::configureAnalysisOptions()
{
    // 配置分析选项
    log(tr("已加载分析选项"));
    
    // 默认禁用所有分析选项复选框
    ui->checkBoxSelectAll->setChecked(false);
    
    // 更多配置代码...
}

// DragDropGraphicsView 的拖放事件处理槽函数
void MainWindow::handleViewDragEnter(QDragEnterEvent *event)
{
    log(tr("图像视图接收到拖放事件"));
    
    // 只接受包含图像文件的拖放
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();
            log(tr("拖放文件: %1").arg(filePath));
            
            if (isSupportedImageFormat(filePath)) {
                log(tr("接受拖放: %1").arg(filePath));
                event->acceptProposedAction();
                return;
            }
        }
    }
    
    // 如果没有可接受的文件，记录拒绝信息并拒绝拖放
    log(tr("拒绝拖放: 没有支持的图像文件"));
    event->ignore();
}

void MainWindow::handleViewDrop(QDropEvent *event)
{
    log(tr("图像视图处理拖放文件"));
    
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();
            log(tr("尝试处理拖放文件: %1").arg(filePath));
            
            if (isSupportedImageFormat(filePath)) {
                handleDroppedFile(filePath);
                log(tr("成功处理拖放文件: %1").arg(filePath));
                event->acceptProposedAction();
                return;
            }
        }
    }
    
    event->ignore();
}

void MainWindow::handleDroppedFile(const QString &filePath)
{
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

bool MainWindow::isSupportedImageFormat(const QString &filePath)
{
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
void MainWindow::on_actionExportPDF_triggered()
{
    generateReport("pdf");
}

void MainWindow::on_actionExportTXT_triggered()
{
    generateReport("txt");
}

void MainWindow::on_actionFullScreen_triggered()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

// 按钮点击处理
void MainWindow::on_startAnalysisButton_clicked()
{
    // 收集选中的分析方法
    QStringList selectedMethods;
    
    // 检查局部质量评价选项
    if (ui->checkBoxISLR->isChecked())
        selectedMethods << "ISLR";
    if (ui->checkBoxPSLR->isChecked())
        selectedMethods << "PSLR";
    if (ui->checkBoxRangeResolution->isChecked())
        selectedMethods << "RangeResolution";
    if (ui->checkBoxAzimuthResolution->isChecked())
        selectedMethods << "AzimuthResolution";
    
    // 检查全局质量评价选项
    if (ui->checkBoxSNR->isChecked())
        selectedMethods << "SNR";
    if (ui->checkBoxInfoContent->isChecked())
        selectedMethods << "InfoContent";
    if (ui->checkBoxClarity->isChecked())
        selectedMethods << "Clarity";
    if (ui->checkBoxRadiometricAccuracy->isChecked())
        selectedMethods << "RadiametricAccuracy";
    if (ui->checkBoxGLCM->isChecked())
        selectedMethods << "GLCM";
    if (ui->checkBoxNESZ->isChecked())
        selectedMethods << "NESZ";
    if (ui->checkBoxRadiometricResolution->isChecked())
        selectedMethods << "RadiometricResolution";
    if (ui->checkBoxENL->isChecked())
        selectedMethods << "ENL";
    
    if (selectedMethods.isEmpty()) {
        QMessageBox::warning(this, tr("未选择分析方法"), 
            tr("请至少选择一种分析方法进行评估。"));
        return;
    }
    
    // 执行分析
    performAnalysis(selectedMethods);
}

void MainWindow::on_actionStartAssessment_triggered()
{
    // 调用开始分析按钮的点击事件
    on_startAnalysisButton_clicked();
}

void MainWindow::on_actionSelectAssessmentRegion_triggered()
{
    // 实现区域选择逻辑
    log(tr("选择区域功能尚未实现"));
    QMessageBox::information(this, tr("功能提示"), 
        tr("区域选择功能将在后续版本中实现。"));
}

void MainWindow::on_checkBoxSelectAll_toggled(bool checked)
{
    // 设置所有分析选项的选中状态
    ui->checkBoxISLR->setChecked(checked);
    ui->checkBoxPSLR->setChecked(checked);
    ui->checkBoxRangeResolution->setChecked(checked);
    ui->checkBoxAzimuthResolution->setChecked(checked);
    ui->checkBoxSNR->setChecked(checked);
    ui->checkBoxInfoContent->setChecked(checked);
    ui->checkBoxClarity->setChecked(checked);
    ui->checkBoxRadiometricAccuracy->setChecked(checked);
    ui->checkBoxGLCM->setChecked(checked);
    ui->checkBoxNESZ->setChecked(checked);
    ui->checkBoxRadiometricResolution->setChecked(checked);
    ui->checkBoxENL->setChecked(checked);
}

void MainWindow::on_pushButton_exportPDF_clicked()
{
    generateReport("pdf");
}

void MainWindow::on_pushButton_exportTXT_clicked()
{
    generateReport("txt");
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("关于 SAR 图像质量评估工具"),
        tr("<h3>SAR 图像质量评估工具</h3>"
           "<p>版本：1.0.0</p>"
           "<p>本工具用于评估合成孔径雷达 (SAR) 图像的质量参数。</p>"
           "<p>支持多种分析方法，包括信噪比分析、分辨率评估等。</p>"
           "<p>&copy; 2023-2024 All Rights Reserved.</p>"));
}

void MainWindow::performAnalysis(const QStringList &selectedMethods)
{
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
        
        // 模拟分析过程
        // 在实际应用中，这里应该调用 imageHandler 的相应分析方法
        onAnalysisProgress(progress, tr("正在执行 %1 分析...").arg(method));
        
        // 生成模拟结果
        QString result = tr("方法：%1\n时间：%2\n结果：模拟分析结果\n\n")
                        .arg(method)
                        .arg(getCurrentDateTime());
        
        // 记录结果
        updateResults(method, result);
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

void MainWindow::generateReport(const QString &format)
{
    if (imageResults.isEmpty()) {
        QMessageBox::warning(this, tr("无法生成报告"), 
            tr("没有分析结果可供导出。请先执行分析。"));
        return;
    }
    
    QString filePath;
    
    if (format == "pdf") {
        filePath = QFileDialog::getSaveFileName(this, tr("导出 PDF 报告"),
            QDir::homePath() + "/" + tr("SAR_分析报告_%1.pdf").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
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
        painter.drawText(QRect(100, yPos, pdfWriter.width() - 200, 30), 
                       Qt::AlignLeft, tr("图像文件：%1").arg(QFileInfo(currentImagePath).fileName()));
        
        yPos += 30;
        painter.drawText(QRect(100, yPos, pdfWriter.width() - 200, 30), 
                       Qt::AlignLeft, tr("评估时间：%1").arg(getCurrentDateTime()));
        
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
        filePath = QFileDialog::getSaveFileName(this, tr("导出文本报告"),
            QDir::homePath() + "/" + tr("SAR_分析报告_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
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
        out << tr("图像文件：%1").arg(QFileInfo(currentImagePath).fileName()) << "\n";
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

QString MainWindow::getCurrentDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void MainWindow::updateResults(const QString &method, const QString &result)
{
    imageResults[method] = result;
}

void MainWindow::showAnalysisResults()
{
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
    if (imageResults.contains("ISLR"))
        ui->method6ResultsTextEdit->setText(imageResults["ISLR"]);
        
    if (imageResults.contains("PSLR"))
        ui->method7ResultsTextEdit->setText(imageResults["PSLR"]);
        
    if (imageResults.contains("RangeResolution"))
        ui->method8ResultsTextEdit->setText(imageResults["RangeResolution"]);
        
    if (imageResults.contains("AzimuthResolution"))
        ui->method9ResultsTextEdit->setText(imageResults["AzimuthResolution"]);
        
    if (imageResults.contains("SNR"))
        ui->method1ResultsTextEdit->setText(imageResults["SNR"]);
        
    if (imageResults.contains("InfoContent"))
        ui->method2ResultsTextEdit->setText(imageResults["InfoContent"]);
        
    if (imageResults.contains("Clarity"))
        ui->method3ResultsTextEdit->setText(imageResults["Clarity"]);
        
    if (imageResults.contains("RadiametricAccuracy"))
        ui->method4ResultsTextEdit->setText(imageResults["RadiametricAccuracy"]);
        
    if (imageResults.contains("GLCM"))
        ui->method5ResultsTextEdit->setText(imageResults["GLCM"]);
        
    if (imageResults.contains("NESZ"))
        ui->method10ResultsTextEdit->setText(imageResults["NESZ"]);
        
    if (imageResults.contains("RadiometricResolution"))
        ui->method11ResultsTextEdit->setText(imageResults["RadiometricResolution"]);
        
    if (imageResults.contains("ENL"))
        ui->method12ResultsTextEdit->setText(imageResults["ENL"]);
}

void MainWindow::onAnalysisProgress(int progress, const QString &message)
{
    ui->analysisProgressBar->setValue(progress);
    log(message);
}

void MainWindow::onAnalysisComplete()
{
    ui->analysisProgressBar->setValue(100);
    log(tr("分析完成"));
}

// 拖放事件处理
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    log(tr("主窗口接收到拖放事件"));
    
    // 只接受包含图像文件的拖放
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();
            log(tr("主窗口拖放文件: %1").arg(filePath));
            
            if (isSupportedImageFormat(filePath)) {
                log(tr("主窗口接受拖放: %1").arg(filePath));
                event->acceptProposedAction();
                return;
            }
        }
    }
    
    // 如果没有可接受的文件，记录拒绝信息
    log(tr("主窗口拒绝拖放: 没有支持的图像文件"));
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    log(tr("主窗口处理拖放文件"));
    
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();
            log(tr("主窗口尝试处理拖放文件: %1").arg(filePath));
            
            if (isSupportedImageFormat(filePath)) {
                handleDroppedFile(filePath);
                log(tr("主窗口成功处理拖放文件: %1").arg(filePath));
                event->acceptProposedAction();
                return;
            }
        }
    }
    
    event->ignore();
}
