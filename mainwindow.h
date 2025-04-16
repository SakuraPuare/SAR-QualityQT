#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QDebug>
#include <QDateTime>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>

// GDAL Includes (确保你的项目配置了 GDAL)
#include "gdal_priv.h"
#include "cpl_conv.h" // For CPLMalloc()

// OpenCV Includes (确保你的项目配置了 OpenCV)
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_actionOpenImage_triggered();
    void on_startAnalysisButton_clicked();
    void on_checkBoxSelectAll_toggled(bool checked);
    // 你可以在这里为每个分析方法添加单独的槽函数，或者在 on_startAnalysisButton_clicked 中处理

private:
    Ui::MainWindow *ui;
    GDALDataset *poDataset = nullptr; // GDAL 数据集指针
    cv::Mat currentImage;             // 当前加载的 OpenCV 图像
    QString currentFilename;          // 当前文件名

    void logMessage(const QString &message);
    void displayImage(const cv::Mat &image);
    void updateImageInfo();
    void closeCurrentImage();
    void openImageFile(const QString& filePath);

    // --- 分析方法占位符 ---
    void performSNRAnalysis();
    void performInfoContentAnalysis();
    void performClarityAnalysis();
    void performRadiometricAnalysis();
    void performGLCMAnalysis();
    void performPointTargetAnalysis(); // 对应 UI 中的点目标分析
};
#endif // MAINWINDOW_H