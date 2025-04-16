#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString> // Keep necessary Qt headers like QString

// Forward declare Ui::MainWindow
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Forward declare cv::Mat - needed for analysis function signatures
namespace cv { class Mat; }

// Include the new ImageHandler header
#include "imagehandler.h" // Include ImageHandler definition

// Forward declare event classes
class QDragEnterEvent;
class QDropEvent;
// #include <QDateTime> // Removed, not needed in header

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

private:
  Ui::MainWindow *ui;
  ImageHandler m_imageHandler; // Use ImageHandler instance

  /**
   * @brief 在日志窗口记录带有时间戳的消息。
   * @param message 要记录的消息。
   */
  void logMessage(const QString &message);

  /**
   * @brief 关闭当前图像相关的 UI 状态并通知 ImageHandler。
   */
  void closeCurrentImage(); // Keep declaration

  /**
   * @brief 尝试使用 ImageHandler 打开图像文件，并更新 UI。
   * @param filePath 图像文件的完整路径。
   */
  void openImageFile(const QString &filePath); // Keep declaration

  // --- 分析方法 (实现需要修改以接受 const cv::Mat&) ---
  void performSNRAnalysis(const cv::Mat& imageData);
  void performInfoContentAnalysis(const cv::Mat& imageData);
  void performClarityAnalysis(const cv::Mat& imageData);
  void performRadiometricAnalysis(const cv::Mat& imageData);
  void performGLCMAnalysis(const cv::Mat& imageData);

  // --- GLCM 辅助函数 (实现需要修改以接受 const cv::Mat&) ---
  cv::Mat prepareImageForGLCM(const cv::Mat& inputImage, QString& log);
  void computeGLCM(const cv::Mat& img, cv::Mat& glcm, int dx, int dy, int levels, bool symmetric /* = true */, bool normalize /* = true */);
  void calculateGLCMFeatures(const cv::Mat& glcm, int levels,
                             double& contrast, double& energy, double& homogeneity, double& correlation);
};
#endif // MAINWINDOW_H