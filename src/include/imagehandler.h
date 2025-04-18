#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QString>
#include <QPixmap>
#include <QSize>
#include <functional> // 用于 std::function

#include <opencv2/core/mat.hpp> // 包含 cv::Mat 定义

// 前向声明 GDAL 数据集类型，避免在头文件中包含重量级的 gdal_priv.h
// 但实际操作中，方法实现需要完整定义，所以包含 gdal_priv.h 可能更直接
// #include <gdal_priv.h>
class GDALDataset; // 使用前向声明，需要在 cpp 文件中包含 gdal_priv.h

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

private:
  // 内部日志记录方法
  void log(const QString &message) const;
  // 准备用于显示的单通道灰度图像
  cv::Mat prepareDisplayMat() const;

  GDALDataset *poDataset = nullptr; // GDAL 数据集指针
  cv::Mat currentImage;             // OpenCV 图像数据
  QString currentFilename;          // 当前文件名
  bool isComplex = false;           // 图像是否为复数

  std::function<void(const QString &)> m_logger; // 日志记录器函数对象
};

#endif // IMAGEHANDLER_H 