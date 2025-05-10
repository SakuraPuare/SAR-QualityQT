#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QFileInfo>
#include <QTranslator>
#include <opencv2/opencv.hpp>

// 引入核心模块
#include "../core/imagehandler.h"
#include "../core/analysis/clarity.h"
#include "../core/analysis/glcm.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace SAR {
namespace UI {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 文件操作
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionExport_Report_triggered();
    
    // 分析功能
    void on_actionRadiometric_Analysis_triggered();
    void on_actionSNR_Analysis_triggered();
    void on_actionClarity_Analysis_triggered();
    void on_actionGLCM_Analysis_triggered();
    void on_actionInfo_Content_Analysis_triggered();
    
    // 区域选择
    void on_actionSelect_ROI_triggered();
    
    // 语言切换
    void switchLanguage();
    void setLanguage(const QString& language);

private:
    Ui::MainWindow *ui;
    
    // 核心组件
    SAR::Core::ImageHandler m_imageHandler;
    
    // 图像相关
    QImage m_displayImage;
    cv::Mat m_currentImage;
    cv::Rect m_selectedROI;
    
    // 多语言支持
    QTranslator m_translator;
    QString m_currentLanguage;
    
    // 初始化函数
    void setupConnections();
    void setupLanguageMenu();
    
    // 辅助函数
    void updateImageDisplay();
    void updateStatusBar(const QString& message);
    void showAnalysisResults(const QString& title, const QString& results);
    
    // 分析结果处理
    void processRadiometricAnalysis();
    void processSNRAnalysis();
    void processClarityAnalysis();
    void processGLCMAnalysis();
    void processInfoContentAnalysis();
};

} // namespace UI
} // namespace SAR

#endif // MAINWINDOW_H