#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QFileInfo>
#include <QTextStream>
#include <QPdfWriter>
#include <QDir>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QImageReader>
#include <QUrl>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    imageScene(new QGraphicsScene(this)),
    hasSelectedRegion(false)
{
    ui->setupUi(this);
    setupImageViewer();
    setupConnections();
    
    // 初始状态下禁用分析按钮
    enableAnalysisButtons(false);
    
    // 初始化状态栏
    statusBar()->showMessage(tr("就绪"));
    
    // 启用接收拖拽
    setAcceptDrops(true);
    
    // 记录日志
    log(tr("应用程序已启动"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupImageViewer()
{
    // 创建图像视图并替换占位符
    imageView = new QGraphicsView(imageScene, this);
    imageView->setRenderHint(QPainter::Antialiasing);
    imageView->setRenderHint(QPainter::SmoothPixmapTransform);
    imageView->setDragMode(QGraphicsView::ScrollHandDrag);
    imageView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    imageView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    
    // 启用图像视图接受拖放
    imageView->setAcceptDrops(false); // 禁用GraphicsView自身的拖放，使用MainWindow的拖放处理
    
    // 替换 UI 中的占位符
    QLayout* layout = ui->verticalLayout_image_display;
    if (layout->count() > 0) {
        QLayoutItem* item = layout->takeAt(0);
        delete item->widget();
        delete item;
    }
    layout->addWidget(imageView);
}

void MainWindow::setupConnections()
{
    // 复选框连接
    connect(ui->checkBoxSelectAll, &QCheckBox::toggled, [this](bool checked) {
        // 当"全选"被勾选或取消时，设置所有其他复选框的状态
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
        tr("图像文件 (*.tif *.tiff *.jpg *.jpeg *.png *.bmp);;所有文件 (*)"));
        
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
    QImageReader reader(filePath);
    QImage image = reader.read();
    
    if (image.isNull()) {
        QMessageBox::warning(this, tr("图像加载失败"),
            tr("无法加载图像文件：%1\n错误：%2")
            .arg(filePath)
            .arg(reader.errorString()));
        return false;
    }
    
    // 清除当前场景
    imageScene->clear();
    
    // 添加图像到场景
    QGraphicsPixmapItem *item = imageScene->addPixmap(QPixmap::fromImage(image));
    imageScene->setSceneRect(item->boundingRect());
    
    // 调整视图以适应图像
    on_actionFitToWindow_triggered();
    
    return true;
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

void MainWindow::on_actionFullScreen_triggered()
{
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void MainWindow::on_actionSelectAssessmentRegion_triggered()
{
    // 这里简单实现，实际应用中可能需要更复杂的区域选择机制
    QMessageBox::information(this, tr("选择区域"),
        tr("请使用鼠标在图像上拖动来选择分析区域。\n此功能在当前示例中未完全实现。"));
        
    // 假设用户选择了整个图像区域
    hasSelectedRegion = true;
    if (!imageScene->items().isEmpty()) {
        selectedRegion = imageScene->items().first()->boundingRect().toRect();
    }
}

void MainWindow::clearResults()
{
    ui->overviewResultsTextEdit->clear();
    ui->method1ResultsTextEdit->clear();
    ui->method2ResultsTextEdit->clear();
    ui->method3ResultsTextEdit->clear();
    ui->method4ResultsTextEdit->clear();
    ui->method5ResultsTextEdit->clear();
    ui->method6ResultsTextEdit->clear();
    ui->method7ResultsTextEdit->clear();
    ui->method8ResultsTextEdit->clear();
    ui->method9ResultsTextEdit->clear();
    ui->method10ResultsTextEdit->clear();
    ui->method11ResultsTextEdit->clear();
    ui->method12ResultsTextEdit->clear();
    
    imageResults.clear();
}

void MainWindow::on_checkBoxSelectAll_toggled(bool checked)
{
    // 已在 setupConnections 中实现
}

void MainWindow::on_startAnalysisButton_clicked()
{
    if (currentImagePath.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先加载图像"));
        return;
    }
    
    QStringList selectedMethods;
    
    // 收集选中的分析方法
    if (ui->checkBoxISLR->isChecked()) selectedMethods << "ISLR";
    if (ui->checkBoxPSLR->isChecked()) selectedMethods << "PSLR";
    if (ui->checkBoxRangeResolution->isChecked()) selectedMethods << "RangeResolution";
    if (ui->checkBoxAzimuthResolution->isChecked()) selectedMethods << "AzimuthResolution";
    if (ui->checkBoxSNR->isChecked()) selectedMethods << "SNR";
    if (ui->checkBoxInfoContent->isChecked()) selectedMethods << "InfoContent";
    if (ui->checkBoxClarity->isChecked()) selectedMethods << "Clarity";
    if (ui->checkBoxRadiometricAccuracy->isChecked()) selectedMethods << "RadiometricAccuracy";
    if (ui->checkBoxGLCM->isChecked()) selectedMethods << "GLCM";
    if (ui->checkBoxNESZ->isChecked()) selectedMethods << "NESZ";
    if (ui->checkBoxRadiometricResolution->isChecked()) selectedMethods << "RadiometricResolution";
    if (ui->checkBoxENL->isChecked()) selectedMethods << "ENL";
    
    if (selectedMethods.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请至少选择一个分析方法"));
        return;
    }
    
    // 清除之前的结果
    clearResults();
    
    // 开始分析
    log(tr("开始分析图像：%1").arg(QFileInfo(currentImagePath).fileName()));
    performAnalysis(selectedMethods);
}

void MainWindow::on_actionStartAssessment_triggered()
{
    on_startAnalysisButton_clicked();
}

void MainWindow::performAnalysis(const QStringList &selectedMethods)
{
    // 在实际应用中，这里应该调用实际的 SAR 图像分析算法
    // 这里仅模拟分析过程
    
    ui->analysisProgressBar->setValue(0);
    ui->analysisProgressBar->setMaximum(selectedMethods.size());
    
    // 禁用分析按钮
    ui->startAnalysisButton->setEnabled(false);
    ui->actionStartAssessment->setEnabled(false);
    
    int totalSteps = selectedMethods.size();
    for (int i = 0; i < totalSteps; ++i) {
        QString method = selectedMethods.at(i);
        
        // 更新进度
        onAnalysisProgress(i + 1, tr("正在分析 %1...").arg(method));
        
        // 模拟分析时间
        QThread::msleep(500);
        
        // 生成模拟结果
        QString result;
        if (method == "SNR") {
            result = tr("信噪比：12.5 dB\n空间分布：见图表");
        } else if (method == "InfoContent") {
            result = tr("信息熵：7.85\n统计直方图：见图表");
        } else if (method == "Clarity") {
            result = tr("清晰度指标：0.83\n边缘检测结果：见图表");
        } else if (method == "RadiometricAccuracy") {
            result = tr("辐射精度：0.5 dB\n校准曲线：见图表");
        } else if (method == "GLCM") {
            result = tr("GLCM 特征:\n对比度：12.4\n相关性：0.78\n能量：0.15\n同质性：0.92");
        } else if (method == "ISLR") {
            result = tr("积分旁瓣比：-13.2 dB\n点目标响应函数：见图表");
        } else if (method == "PSLR") {
            result = tr("峰值旁瓣比：-18.5 dB\n点目标响应函数：见图表");
        } else if (method == "RangeResolution") {
            result = tr("距离模糊度：1.2 m\n分辨率评估图：见图表");
        } else if (method == "AzimuthResolution") {
            result = tr("方位模糊度：1.5 m\n分辨率评估图：见图表");
        } else if (method == "NESZ") {
            result = tr("噪声等效后向散射系数：-25 dB\n噪声评估曲线：见图表");
        } else if (method == "RadiometricResolution") {
            result = tr("辐射分辨率：1.8 dB\n灰度级分布：见图表");
        } else if (method == "ENL") {
            result = tr("等效视数：8.5\n多视处理效果：见图表");
        }
        
        // 保存结果
        updateResults(method, result);
    }
    
    // 完成分析
    onAnalysisComplete();
}

void MainWindow::updateResults(const QString &method, const QString &result)
{
    imageResults[method] = result;
    
    // 更新对应选项卡的文本
    if (method == "SNR") {
        ui->method1ResultsTextEdit->setText(result);
    } else if (method == "InfoContent") {
        ui->method2ResultsTextEdit->setText(result);
    } else if (method == "Clarity") {
        ui->method3ResultsTextEdit->setText(result);
    } else if (method == "RadiometricAccuracy") {
        ui->method4ResultsTextEdit->setText(result);
    } else if (method == "GLCM") {
        ui->method5ResultsTextEdit->setText(result);
    } else if (method == "ISLR") {
        ui->method6ResultsTextEdit->setText(result);
    } else if (method == "PSLR") {
        ui->method7ResultsTextEdit->setText(result);
    } else if (method == "RangeResolution") {
        ui->method8ResultsTextEdit->setText(result);
    } else if (method == "AzimuthResolution") {
        ui->method9ResultsTextEdit->setText(result);
    } else if (method == "NESZ") {
        ui->method10ResultsTextEdit->setText(result);
    } else if (method == "RadiometricResolution") {
        ui->method11ResultsTextEdit->setText(result);
    } else if (method == "ENL") {
        ui->method12ResultsTextEdit->setText(result);
    }
}

void MainWindow::onAnalysisProgress(int progress, const QString &message)
{
    ui->analysisProgressBar->setValue(progress);
    updateStatusBar(message);
    log(message);
}

void MainWindow::onAnalysisComplete()
{
    // 生成概览
    QString overview = tr("SAR 图像质量评估概览\n");
    overview += tr("图像：%1\n").arg(QFileInfo(currentImagePath).fileName());
    overview += tr("评估时间：%1\n\n").arg(getCurrentDateTime());
    
    if (imageResults.contains("SNR"))
        overview += tr("信噪比：12.5 dB\n");
    if (imageResults.contains("InfoContent"))
        overview += tr("信息熵：7.85\n");
    if (imageResults.contains("Clarity"))
        overview += tr("清晰度指标：0.83\n");
    if (imageResults.contains("ISLR"))
        overview += tr("积分旁瓣比：-13.2 dB\n");
    if (imageResults.contains("PSLR"))
        overview += tr("峰值旁瓣比：-18.5 dB\n");
    if (imageResults.contains("RangeResolution"))
        overview += tr("距离模糊度：1.2 m\n");
    if (imageResults.contains("AzimuthResolution"))
        overview += tr("方位模糊度：1.5 m\n");
    
    ui->overviewResultsTextEdit->setText(overview);
    
    // 启用分析按钮
    ui->startAnalysisButton->setEnabled(true);
    ui->actionStartAssessment->setEnabled(true);
    
    log(tr("图像分析完成"));
    updateStatusBar(tr("分析完成"));
    
    // 显示结果
    showAnalysisResults();
}

void MainWindow::showAnalysisResults()
{
    // 切换到结果标签
    ui->resultsTabWidget->setCurrentIndex(0);
}

void MainWindow::on_pushButton_exportPDF_clicked()
{
    generateReport("pdf");
}

void MainWindow::on_actionExportPDF_triggered()
{
    on_pushButton_exportPDF_clicked();
}

void MainWindow::on_pushButton_exportTXT_clicked()
{
    generateReport("txt");
}

void MainWindow::on_actionExportTXT_triggered()
{
    on_pushButton_exportTXT_clicked();
}

void MainWindow::generateReport(const QString &format)
{
    if (imageResults.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("没有可导出的分析结果"));
        return;
    }
    
    QString fileName;
    if (format == "pdf") {
        fileName = QFileDialog::getSaveFileName(this, tr("导出为 PDF"),
            QDir::homePath() + "/SAR_分析报告_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".pdf",
            tr("PDF 文件 (*.pdf)"));
            
        if (fileName.isEmpty())
            return;
            
        QPdfWriter writer(fileName);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageMargins(QMarginsF(30, 30, 30, 30));
        
        QPainter painter(&writer);
        painter.setFont(QFont("SimSun", 12));
        
        // 绘制报告标题
        painter.drawText(QRect(0, 300, writer.width(), 50),
            Qt::AlignHCenter | Qt::AlignTop,
            tr("SAR 图像质量评估报告"));
            
        // 绘制基本信息
        int yPos = 400;
        painter.drawText(QRect(100, yPos, writer.width() - 200, 50),
            Qt::AlignLeft | Qt::AlignTop,
            tr("图像：%1").arg(QFileInfo(currentImagePath).fileName()));
            
        yPos += 50;
        painter.drawText(QRect(100, yPos, writer.width() - 200, 50),
            Qt::AlignLeft | Qt::AlignTop,
            tr("评估时间：%1").arg(getCurrentDateTime()));
            
        // 绘制结果
        yPos += 100;
        QMap<QString, QString>::const_iterator i = imageResults.constBegin();
        while (i != imageResults.constEnd()) {
            QString methodName;
            if (i.key() == "SNR") methodName = tr("信噪比");
            else if (i.key() == "InfoContent") methodName = tr("信息熵");
            else if (i.key() == "Clarity") methodName = tr("清晰度");
            else if (i.key() == "RadiometricAccuracy") methodName = tr("辐射精度");
            else if (i.key() == "GLCM") methodName = tr("GLCM 特征");
            else if (i.key() == "ISLR") methodName = tr("积分旁瓣比");
            else if (i.key() == "PSLR") methodName = tr("峰值旁瓣比");
            else if (i.key() == "RangeResolution") methodName = tr("距离模糊度");
            else if (i.key() == "AzimuthResolution") methodName = tr("方位模糊度");
            else if (i.key() == "NESZ") methodName = tr("噪声等效后向散射系数");
            else if (i.key() == "RadiometricResolution") methodName = tr("辐射分辨率");
            else if (i.key() == "ENL") methodName = tr("等效视数");
            else methodName = i.key();
            
            // 绘制方法名称
            painter.setFont(QFont("SimSun", 10, QFont::Bold));
            painter.drawText(QRect(100, yPos, writer.width() - 200, 30),
                Qt::AlignLeft | Qt::AlignTop, methodName);
                
            // 绘制结果
            painter.setFont(QFont("SimSun", 9));
            QStringList lines = i.value().split("\n");
            for (const QString &line : lines) {
                yPos += 30;
                painter.drawText(QRect(120, yPos, writer.width() - 240, 30),
                    Qt::AlignLeft | Qt::AlignTop, line);
            }
            
            yPos += 50;
            ++i;
        }
        
        painter.end();
        
    } else if (format == "txt") {
        fileName = QFileDialog::getSaveFileName(this, tr("导出为 TXT"),
            QDir::homePath() + "/SAR_分析报告_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt",
            tr("文本文件 (*.txt)"));
            
        if (fileName.isEmpty())
            return;
            
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("错误"), tr("无法创建文件"));
            return;
        }
        
        QTextStream out(&file);
        // out.setCodec("UTF-8");
        
        // 写入报告标题
        out << tr("SAR 图像质量评估报告") << "\n\n";
        
        // 写入基本信息
        out << tr("图像：%1").arg(QFileInfo(currentImagePath).fileName()) << "\n";
        out << tr("评估时间：%1").arg(getCurrentDateTime()) << "\n\n";
        
        // 写入结果
        QMap<QString, QString>::const_iterator i = imageResults.constBegin();
        while (i != imageResults.constEnd()) {
            QString methodName;
            if (i.key() == "SNR") methodName = tr("信噪比");
            else if (i.key() == "InfoContent") methodName = tr("信息熵");
            else if (i.key() == "Clarity") methodName = tr("清晰度");
            else if (i.key() == "RadiometricAccuracy") methodName = tr("辐射精度");
            else if (i.key() == "GLCM") methodName = tr("GLCM 特征");
            else if (i.key() == "ISLR") methodName = tr("积分旁瓣比");
            else if (i.key() == "PSLR") methodName = tr("峰值旁瓣比");
            else if (i.key() == "RangeResolution") methodName = tr("距离模糊度");
            else if (i.key() == "AzimuthResolution") methodName = tr("方位模糊度");
            else if (i.key() == "NESZ") methodName = tr("噪声等效后向散射系数");
            else if (i.key() == "RadiometricResolution") methodName = tr("辐射分辨率");
            else if (i.key() == "ENL") methodName = tr("等效视数");
            else methodName = i.key();
            
            out << methodName << ":\n";
            out << i.value() << "\n\n";
            
            ++i;
        }
        
        file.close();
    }
    
    if (!fileName.isEmpty()) {
        log(tr("已导出报告：%1").arg(fileName));
        QMessageBox::information(this, tr("导出成功"), tr("报告已成功导出到:\n%1").arg(fileName));
    }
}

QString MainWindow::getCurrentDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
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

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("关于 SAR 图像质量评估工具"),
        tr("SAR 图像质量评估工具 v1.0\n\n"
           "该工具用于评估 SAR 图像的质量指标，包括：\n"
           "- 信噪比\n"
           "- 信息熵\n"
           "- 清晰度\n"
           "- 辐射精度\n"
           "- GLCM 纹理特征\n"
           "- 积分旁瓣比\n"
           "- 峰值旁瓣比\n"
           "- 距离模糊度\n"
           "- 方位模糊度\n"
           "- 噪声等效后向散射系数\n"
           "- 辐射分辨率\n"
           "- 等效视数\n\n"
           "版权所有 © 2023"));
}

// 添加判断图像格式是否支持的方法
bool MainWindow::isSupportedImageFormat(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return (suffix == "tif" || suffix == "tiff" || suffix == "jpg" || 
            suffix == "jpeg" || suffix == "png" || suffix == "bmp");
}

// 修改dragEnterEvent方法使用新方法
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    // 检查拖拽的数据是否包含URL（文件路径）
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            // 只接受本地文件
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                if (isSupportedImageFormat(filePath)) {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        
        // 处理拖放的每个文件
        for (const QUrl &url : urlList) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                handleDroppedFile(filePath);
            }
        }
        
        event->acceptProposedAction();
    }
}

// 修改handleDroppedFile方法使用新方法
void MainWindow::handleDroppedFile(const QString &filePath)
{
    if (isSupportedImageFormat(filePath)) {
        if (loadImage(filePath)) {
            currentImagePath = filePath;
            QFileInfo fileInfo(filePath);
            
            // 添加到图像列表
            if (!loadedImages.contains(filePath)) {
                loadedImages.append(filePath);
                ui->imageListWidget->addItem(fileInfo.fileName());
                ui->imageListWidget->setCurrentRow(ui->imageListWidget->count() - 1);
            }
            
            log(tr("已通过拖放加载图像：%1").arg(fileInfo.fileName()));
            updateStatusBar(tr("图像已加载：%1").arg(fileInfo.fileName()));
            
            // 启用分析按钮
            enableAnalysisButtons(true);
        }
    } else {
        QFileInfo fileInfo(filePath);
        QMessageBox::warning(this, tr("不支持的文件格式"),
            tr("无法加载文件：%1\n只支持图像文件格式（tif, jpg, png等）").arg(fileInfo.fileName()));
    }
}