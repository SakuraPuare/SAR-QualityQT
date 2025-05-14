#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QProgressDialog>
#include <QTextEdit>
#include <QThread>
#include <QPixmap>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QRegularExpression>
#include <QTextDocument>
#include <QMap>
#include <QStringList>

// C++标准库头文件放在一起
#include <cmath>
#include <limits>

// 添加核心模块头文件
#include "../../core/include/analysis_result.h"
#include "../../core/include/imagehandler.h"
#include "../../core/imagefilters.h" // 添加滤波器头文件
#include "threshold_settings_dialog.h"

// 部分类仍使用前向声明
namespace SAR {
namespace Core {
    class Logger;
    class AnalysisController;
}
namespace UI {
    class ReportGenerator;
}
}

// 自定义拖放图形视图的前向声明
class DragDropGraphicsView;

namespace Ui {
class MainWindow;
}

// 声明滤波器设置对话框类
class FilterSettingsDialog;

/**
 * @brief 主窗口类
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~MainWindow();

protected:
    /**
     * @brief 拖放进入事件处理函数
     * @param event 拖放进入事件
     */
    void dragEnterEvent(QDragEnterEvent *event) override;
    
    /**
     * @brief 拖放事件处理函数
     * @param event 拖放事件
     */
    void dropEvent(QDropEvent *event) override;

private slots:
    // 文件操作
    void on_actionOpenImage_triggered();
    void on_actionCloseImage_triggered();
    void on_actionExportPDF_triggered();
    void on_actionExportTXT_triggered();
    
    // 视图操作
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionFitToWindow_triggered();
    void on_actionPan_toggled(bool checked);
    void on_actionFullScreen_triggered();
    
    // 分析操作
    void on_startAnalysisButton_clicked();
    void on_actionStartAssessment_triggered();
    void on_actionSelectAssessmentRegion_triggered();
    void on_checkBoxSelectAll_toggled(bool checked);
    void on_actionThresholdSettings_triggered();
    
    // 导出结果
    void on_pushButton_exportPDF_clicked();
    void on_pushButton_exportTXT_clicked();
    
    // 图像列表操作
    void on_imageListWidget_itemClicked(QListWidgetItem *item);
    
    // 滤波操作
    void on_actionFilterSettings_triggered();
    void on_actionLowPassFilter_triggered();
    void on_actionHighPassFilter_triggered();
    void on_actionBandPassFilter_triggered();
    void on_actionMedianFilter_triggered();
    void on_actionGaussianFilter_triggered();
    void on_actionBilateralFilter_triggered();
    void on_actionLeeFilter_triggered();
    void on_actionFrostFilter_triggered();
    void on_actionKuanFilter_triggered();
    
    // 其他
    void on_actionAbout_triggered();
    void updateStatusBar(const QString &message);
    void log(const QString &message);
    void onAnalysisProgress(int progress, const QString &message);
    void onAnalysisComplete(const SAR::Core::AnalysisResult &results);
    
    // 拖放相关槽函数
    void handleViewDragEnter(QDragEnterEvent *event);
    void handleViewDrop(QDropEvent *event);
    
    // 日志系统相关槽
    void onNewLogMessage(const QString &message);
    void clearLog();
    void saveLog();
    
    // 新增的图像增强相关槽函数
    void onDisplayModeChanged(int index);
    void onClipPercentileChanged();
    void onAutoEnhanceClicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *imageScene;
    DragDropGraphicsView *imageView; // 更改为自定义视图类
    QString currentImagePath;
    bool hasSelectedRegion;
    QRect selectedRegion;
    QStringList loadedImages;
    QMap<QString, QString> imageResults;
    
    // 添加 ImageHandler 成员变量，用于 GDAL 图像处理
    SAR::Core::ImageHandler *imageHandler;
    
    // 新添加的成员
    SAR::Core::AnalysisController *analysisController;
    SAR::UI::ReportGenerator *reportGenerator;
    
    SAR::Core::AnalysisResult currentResults; // 当前分析结果
    
    // 滤波器设置
    SAR::Core::FilterParameters currentFilterParams;
    
    void setupImageViewer();
    void setupConnections();
    void configureAnalysisOptions(); // 添加配置分析选项方法声明
    void configureResultTabs(); // 配置结果选项卡的显示状态
    void enableAnalysisButtons(bool enable);
    void clearResults();
    bool loadImage(const QString &filePath);
    void performAnalysis(const QStringList &selectedMethods);
    void generateReport(const QString &format);
    QString getCurrentDateTime();
    void updateResults(const QString &method, const QString &result);
    void showAnalysisResults();
    void showAnalysisResult(const SAR::Core::AnalysisResult &result);
    void handleDroppedFile(const QString &filePath); // 处理拖放文件
    bool isSupportedImageFormat(const QString &filePath);
    
    // 滤波器相关方法
    void applyFilter(SAR::Core::FilterType filterType);
    void showFilterSettingsDialog(SAR::Core::FilterType filterType = SAR::Core::FilterType::Gaussian);
    
    // 新增的图像增强相关方法
    void setupImageEnhancementControls();
    void refreshImageDisplay();
    void applyImageEnhancement();
};

#endif // MAINWINDOW_H