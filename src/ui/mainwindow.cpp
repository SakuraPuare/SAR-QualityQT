#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QStatusBar>

namespace SAR {
namespace UI {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_imageHandler()
{
    ui->setupUi(this);
    
    // 设置窗口属性
    setWindowTitle(tr("SAR-QualityQT - SAR 图像质量评估工具"));
    resize(1024, 768);
    
    // 初始化组件
    setupConnections();
    setupLanguageMenu();
    
    // 设置状态栏
    statusBar()->showMessage(tr("准备就绪"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 文件操作
void MainWindow::on_actionOpen_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("打开 SAR 图像文件"),
        QString(),
        tr("图像文件 (*.tif *.tiff *.jpg *.jpeg *.png);;所有文件 (*.*)"));
    
    if (filePath.isEmpty())
        return;
    
    if (m_imageHandler.loadImage(filePath)) {
        m_currentImage = m_imageHandler.getImage();
        updateImageDisplay();
        updateStatusBar(tr("已加载图像：%1").arg(QFileInfo(filePath).fileName()));
    } else {
        QMessageBox::warning(this, tr("错误"), tr("无法加载图像文件"));
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (m_currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("没有图像可以保存"));
        return;
    }
    
    QString filePath = QFileDialog::getSaveFileName(this,
        tr("保存图像"),
        QString(),
        tr("TIFF 文件 (*.tif *.tiff);;JPEG 文件 (*.jpg *.jpeg);;PNG 文件 (*.png)"));
    
    if (filePath.isEmpty())
        return;
    
    if (m_imageHandler.saveImage(filePath)) {
        updateStatusBar(tr("图像已保存到：%1").arg(filePath));
    } else {
        QMessageBox::warning(this, tr("错误"), tr("保存图像失败"));
    }
}

void MainWindow::on_actionExport_Report_triggered()
{
    QMessageBox::information(this, tr("功能开发中"), tr("导出报告功能尚未实现"));
}

// 分析功能
void MainWindow::on_actionRadiometric_Analysis_triggered()
{
    if (m_currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先加载图像"));
        return;
    }
    
    processRadiometricAnalysis();
}

void MainWindow::on_actionSNR_Analysis_triggered()
{
    if (m_currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先加载图像"));
        return;
    }
    
    processSNRAnalysis();
}

void MainWindow::on_actionClarity_Analysis_triggered()
{
    if (m_currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先加载图像"));
        return;
    }
    
    processClarityAnalysis();
}

void MainWindow::on_actionGLCM_Analysis_triggered()
{
    if (m_currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先加载图像"));
        return;
    }
    
    processGLCMAnalysis();
}

void MainWindow::on_actionInfo_Content_Analysis_triggered()
{
    if (m_currentImage.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先加载图像"));
        return;
    }
    
    processInfoContentAnalysis();
}

// 区域选择
void MainWindow::on_actionSelect_ROI_triggered()
{
    QMessageBox::information(this, tr("功能开发中"), tr("区域选择功能尚未实现"));
}

// 语言切换
void MainWindow::switchLanguage()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        setLanguage(action->data().toString());
    }
}

void MainWindow::setLanguage(const QString& language)
{
    if (m_currentLanguage == language)
        return;
    
    m_currentLanguage = language;
    
    // 移除当前翻译
    qApp->removeTranslator(&m_translator);
    
    // 加载新翻译
    if (language != "en") {
        if (m_translator.load(QString(":/i18n/SAR-QualityQT_%1.qm").arg(language))) {
            qApp->installTranslator(&m_translator);
        }
    }
    
    // 重新翻译 UI
    ui->retranslateUi(this);
    updateStatusBar(tr("语言已切换到：%1").arg(language == "zh_CN" ? "中文" : "English"));
}

// 私有辅助函数
void MainWindow::setupConnections()
{
    // 暂时留空
}

void MainWindow::setupLanguageMenu()
{
    // 暂时留空
}

void MainWindow::updateImageDisplay()
{
    if (!m_currentImage.empty()) {
        m_displayImage = SAR::Core::ImageHandler::cvMatToQImage(m_currentImage);
        if (!m_displayImage.isNull() && ui->imageDisplayLabel) {
            ui->imageDisplayLabel->setPixmap(QPixmap::fromImage(m_displayImage).scaled(
                ui->imageDisplayLabel->size(), 
                Qt::KeepAspectRatio, 
                Qt::SmoothTransformation));
        }
    }
}

void MainWindow::updateStatusBar(const QString& message)
{
    statusBar()->showMessage(message);
}

void MainWindow::showAnalysisResults(const QString& title, const QString& results)
{
    QMessageBox::information(this, title, results);
}

// 分析结果处理
void MainWindow::processRadiometricAnalysis()
{
    QMessageBox::information(this, tr("功能开发中"), tr("辐射分析功能尚未实现"));
}

void MainWindow::processSNRAnalysis()
{
    QMessageBox::information(this, tr("功能开发中"), tr("信噪比分析功能尚未实现"));
}

void MainWindow::processClarityAnalysis()
{
    // 使用新的清晰度分析模块
    double clarity = SAR::Analysis::ClarityAnalysis::calculateClarity(m_currentImage);
    double gradientEnergy = SAR::Analysis::ClarityAnalysis::calculateGradientEnergy(m_currentImage);
    double tenengradVariance = SAR::Analysis::ClarityAnalysis::calculateTenengradVariance(m_currentImage);
    double entropy = SAR::Analysis::ClarityAnalysis::calculateEntropy(m_currentImage);
    
    QString results = tr("清晰度指标：%1\n梯度能量：%2\nTenengrad 方差：%3\n熵：%4")
                        .arg(clarity)
                        .arg(gradientEnergy)
                        .arg(tenengradVariance)
                        .arg(entropy);
    
    showAnalysisResults(tr("清晰度分析"), results);
}

void MainWindow::processGLCMAnalysis()
{
    // 使用新的 GLCM 分析模块
    SAR::Analysis::GLCMAnalysis::GLCMFeatures features = 
        SAR::Analysis::GLCMAnalysis::calculateGLCMFeatures(m_currentImage);
    
    QString results = SAR::Analysis::GLCMAnalysis::getGLCMFeaturesDescription(features);
    showAnalysisResults(tr("GLCM 分析"), results);
}

void MainWindow::processInfoContentAnalysis()
{
    QMessageBox::information(this, tr("功能开发中"), tr("信息内容分析功能尚未实现"));
}

} // namespace UI
} // namespace SAR 