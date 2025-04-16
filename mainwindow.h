#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QImage>
#include <QMainWindow>
#include <QMessageBox>
#include <QMimeData>
#include <QPixmap>
#include <QUrl>

// GDAL Includes (确保你的项目配置了 GDAL)
#include "cpl_conv.h" // For CPLMalloc()
#include "gdal_priv.h"

// OpenCV Includes (确保你的项目配置了 OpenCV)
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
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
  // 你可以在这里为每个分析方法添加单独的槽函数，或者在
  // on_startAnalysisButton_clicked 中处理

private:
  Ui::MainWindow *ui;
  GDALDataset *poDataset = nullptr; // GDAL 数据集指针
  cv::Mat currentImage;             // 当前加载的 OpenCV 图像
  QString currentFilename;          // 当前文件名

  /**
   * @brief 在日志窗口记录带有时间戳的消息。
   * @param message 要记录的消息。
   */
  void logMessage(const QString &message);

  /**
   * @brief 在 UI 的 imageDisplayLabel 上显示图像。
   * @param image 要显示的 OpenCV Mat 对象。
   */
  void displayImage(const cv::Mat &image);

  /**
   * @brief 更新 UI 控件以显示当前图像的基本信息（文件名、尺寸、数据类型）。
   */
  void updateImageInfo();

  /**
   * @brief 关闭当前打开的 GDAL 数据集并释放相关资源。
   */
  void closeCurrentImage();

  /**
   * @brief 使用 GDAL 打开指定路径的图像文件，读取数据到 currentImage 并更新 UI。
   * @param filePath 图像文件的完整路径。
   */
  void openImageFile(const QString &filePath);

  // --- 分析方法占位符 ---
  void performSNRAnalysis();
  void performInfoContentAnalysis();
  void performClarityAnalysis();
  void performRadiometricAnalysis();
  void performGLCMAnalysis();

  // --- GLCM 辅助函数 ---
  cv::Mat prepareImageForGLCM(const cv::Mat& inputImage, QString& log);
  void computeGLCM(const cv::Mat& img, cv::Mat& glcm, int dx, int dy, int levels, bool symmetric = true, bool normalize = true);
  void calculateGLCMFeatures(const cv::Mat& glcm, int levels,
                             double& contrast, double& energy, double& homogeneity, double& correlation);

  // 信息量计算
};
#endif // MAINWINDOW_H