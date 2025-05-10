#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QProgressDialog>
#include <QTextEdit>
#include <QPrinter>
#include <QPrintDialog>
#include <QThread>
#include <QPixmap>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

// 添加 ImageHandler 头文件
#include "imagehandler.h"
// 添加 Logger 头文件
#include "logger.h"

// 自定义 GraphicsView 类支持拖放
class DragDropGraphicsView : public QGraphicsView
{
    Q_OBJECT
    
public:
    explicit DragDropGraphicsView(QWidget *parent = nullptr) : QGraphicsView(parent) {
        setAcceptDrops(true);
    }
    
protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        emit dragEnterReceived(event);
    }
    
    void dropEvent(QDropEvent *event) override {
        emit dropReceived(event);
    }
    
signals:
    void dragEnterReceived(QDragEnterEvent *event);
    void dropReceived(QDropEvent *event);
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 添加拖放事件处理函数
    void dragEnterEvent(QDragEnterEvent *event) override;
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
    
    // 导出结果
    void on_pushButton_exportPDF_clicked();
    void on_pushButton_exportTXT_clicked();
    
    // 图像列表操作
    void on_imageListWidget_itemClicked(QListWidgetItem *item);
    
    // 其他
    void on_actionAbout_triggered();
    void updateStatusBar(const QString &message);
    void log(const QString &message);
    void onAnalysisProgress(int progress, const QString &message);
    void onAnalysisComplete();
    
    // 拖放相关槽函数
    void handleViewDragEnter(QDragEnterEvent *event);
    void handleViewDrop(QDropEvent *event);
    
    // 日志系统相关槽
    void onNewLogMessage(const QString &message);
    void clearLog();
    void saveLog();

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
    
    void setupImageViewer();
    void setupConnections();
    void setupLogSystem(); // 设置日志系统
    void configureAnalysisOptions(); // 添加配置分析选项方法声明
    void enableAnalysisButtons(bool enable);
    void clearResults();
    bool loadImage(const QString &filePath);
    void performAnalysis(const QStringList &selectedMethods);
    void generateReport(const QString &format);
    QString getCurrentDateTime();
    void updateResults(const QString &method, const QString &result);
    void showAnalysisResults();
    void handleDroppedFile(const QString &filePath); // 处理拖放文件
    bool isSupportedImageFormat(const QString &filePath); // 检查是否为支持的图像格式
};

#endif // MAINWINDOW_H