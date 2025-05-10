#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString> // Keep necessary Qt headers like QString
#include <QList>
#include <QFileInfo>
#include <QListWidgetItem>

// Forward declare Ui::MainWindow
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Forward declare UI classes
class QCheckBox;
class QPushButton;
class QProgressBar;

// Include the new ImageHandler header
#include "imagehandler.h" // Include ImageHandler definition

// Include the analysis utilities header
#include "analysis_utils.h" // <--- 添加这一行

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
  
  // 导出功能
  void on_pushButton_exportPDF_clicked();
  void on_pushButton_exportTXT_clicked();
  void on_actionExportPDF_triggered();
  void on_actionExportTXT_triggered();
  
  // 图像操作
  void on_actionCloseImage_triggered();
  void on_actionStartAssessment_triggered();
  void on_actionSelectAssessmentRegion_triggered();
  
  // 缩放控制
  void on_actionZoomIn_triggered();
  void on_actionZoomOut_triggered();
  void on_actionFitToWindow_triggered();
  void on_actionPan_toggled(bool checked);
  
  // 其他功能
  void on_actionSaveReport_toolbar_triggered();
  void on_actionFullScreen_triggered();
  
  // 图像列表功能
  void on_imageListWidget_itemClicked(QListWidgetItem *item);
  void on_actionToggleImageList_toggled(bool checked);
  
  // 新增分析算法相关功能
  void on_actionISLR_triggered();
  void on_actionPSLR_triggered();
  void on_actionRangeResolution_triggered();
  void on_actionAzimuthResolution_triggered();
  void on_actionNESZ_triggered();
  void on_actionRadiometricResolution_triggered();
  void on_actionENL_triggered();

private:
  Ui::MainWindow *ui;
  ImageHandler m_imageHandler; // Use ImageHandler instance
  QList<QFileInfo> m_loadedImages; // 已加载的图像文件列表
  int m_currentImageIndex; // 当前显示图像的索引
  
  // 动态创建的 UI 控件
  QCheckBox *m_checkBoxSelectAll;
  QPushButton *m_startAnalysisButton;
  QProgressBar *m_progressBar;
  
  // 局部质量评价控件
  QCheckBox *m_checkBoxISLR;
  QCheckBox *m_checkBoxPSLR;
  QCheckBox *m_checkBoxRangeResolution;
  QCheckBox *m_checkBoxAzimuthResolution;
  
  // 全局质量评价控件
  QCheckBox *m_checkBoxSNR;
  QCheckBox *m_checkBoxInfoContent;
  QCheckBox *m_checkBoxClarity;
  QCheckBox *m_checkBoxRadiometricAccuracy;
  QCheckBox *m_checkBoxGLCM;
  QCheckBox *m_checkBoxNESZ;
  QCheckBox *m_checkBoxRadiometricResolution;
  QCheckBox *m_checkBoxENL;

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
  
  /**
   * @brief 导出评估报告为PDF格式
   */
  void exportReportToPDF();
  
  /**
   * @brief 导出评估报告为TXT格式
   */
  void exportReportToTXT();
  
  /**
   * @brief 选择评估区域
   */
  void selectAssessmentRegion();
  
  /**
   * @brief 控制图像显示的缩放级别
   * @param factor 缩放因子，大于1表示放大，小于1表示缩小
   */
  void zoomImage(double factor);
  
  /**
   * @brief 更新图像列表显示
   */
  void updateImageList();
  
  /**
   * @brief 创建质量评估面板
   */
  void createQualityPanel();
  
  /**
   * @brief 加载并显示指定索引的图像
   * @param index 图像索引
   */
  void loadAndDisplayImage(int index);
};
#endif // MAINWINDOW_H