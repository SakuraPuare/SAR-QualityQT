#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QString>
#include <QPixmap>
#include <QSize>
#include <functional> // 用于 std::function

#include <opencv2/core/mat.hpp> // 包含 cv::Mat 定义
#include "../imagefilters.h" // 包含滤波器定义

// 前向声明 GDAL 数据集类型，避免在头文件中包含重量级的 gdal_priv.h
// 但实际操作中，方法实现需要完整定义，所以包含 gdal_priv.h 可能更直接
// #include <gdal_priv.h>
class GDALDataset; // 使用前向声明，需要在 cpp 文件中包含 gdal_priv.h

namespace SAR {
namespace Core {

/**
 * @brief 图像显示模式
 * 定义不同的SAR图像显示模式
 */
enum class ImageDisplayMode {
    Linear,       // 线性缩放
    Logarithmic,  // 对数缩放
    Sqrt,         // 平方根缩放
    ClipPercent   // 按百分比裁剪
};

/**
 * @brief 图像处理类
 * 提供SAR图像加载、处理和转换功能
 */
class ImageHandler {
public:
  // 构造函数，可选接收一个日志记录器函数
  explicit ImageHandler(std::function<void(const QString &)> logger = nullptr);
  // 析构函数
  ~ImageHandler();

  // 禁止拷贝和赋值，因为包含原始指针 (GDALDataset*)
  ImageHandler(const ImageHandler &) = delete;
  ImageHandler &operator=(const ImageHandler &) = delete;
  // 可以考虑移动构造和移动赋值，如果需要的话

  // 加载图像文件
  bool loadImage(const QString &filePath);
  // 关闭当前图像
  void closeImage();
  // 设置日志记录器
  void setLogger(std::function<void(const QString &)> logger);

  // --- 查询方法 ---
  // 检查是否有有效图像加载
  bool isValid() const;
  // 获取当前图像文件名
  QString getFilename() const;
  // 获取图像尺寸字符串 ("Width x Height")
  QString getDimensionsString() const;
  // 获取图像数据类型字符串
  QString getDataTypeString() const;
  // 获取用于显示的 QPixmap，会进行缩放
  QPixmap getDisplayPixmap(const QSize &targetSize) const;
  // 获取原始图像数据 (cv::Mat) 的常量引用
  const cv::Mat &getImageData() const;

  // --- 图像处理方法 ---
  // 应用滤波器并返回处理后的图像
  cv::Mat applyFilter(const FilterParameters& params, QString* log = nullptr) const;
  // 应用滤波器并更新当前图像
  bool applyFilterInplace(const FilterParameters& params, QString* log = nullptr);
  // 获取支持的滤波器类型列表
  static QStringList getAvailableFilterTypes();
  
  // --- SAR图像高动态范围处理方法 ---
  // 设置图像显示模式
  void setDisplayMode(ImageDisplayMode mode);
  // 获取当前显示模式
  ImageDisplayMode getDisplayMode() const;
  // 设置裁剪百分比 (用于ClipPercent模式)
  void setClipPercentile(double lower, double upper);
  // 获取当前裁剪百分比
  QPair<double, double> getClipPercentile() const;
  // 自动增强SAR图像显示
  bool autoEnhance();

private:
  // 内部日志记录方法
  void log(const QString &message) const;
  // 准备用于显示的单通道灰度图像
  cv::Mat prepareDisplayMat() const;
  // 应用线性缩放
  cv::Mat applyLinearScaling(const cv::Mat& image) const;
  // 应用对数缩放
  cv::Mat applyLogScaling(const cv::Mat& image) const;
  // 应用平方根缩放
  cv::Mat applySqrtScaling(const cv::Mat& image) const;
  // 应用百分比裁剪
  cv::Mat applyClipPercentScaling(const cv::Mat& image) const;

  GDALDataset *poDataset = nullptr; // GDAL 数据集指针
  cv::Mat currentImage;             // OpenCV 图像数据
  QString currentFilename;          // 当前文件名
  bool isComplex = false;           // 图像是否为复数

  std::function<void(const QString &)> m_logger; // 日志记录器函数对象
  
  // 图像显示相关
  ImageDisplayMode m_displayMode = ImageDisplayMode::Linear; // 默认显示模式
  double m_lowerClipPercent = 1.0;  // 默认下裁剪百分比
  double m_upperClipPercent = 99.0; // 默认上裁剪百分比
};

} // namespace Core
} // namespace SAR

#endif // IMAGEHANDLER_H 