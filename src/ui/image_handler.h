#ifndef IMAGE_HANDLER_H
#define IMAGE_HANDLER_H

#include <QString>
#include <QPixmap>
#include <QSize>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMessageBox>
#include <QImageReader>
#include <QListWidget>

// 前向声明
namespace SAR {
namespace Core {
class ImageHandler;
}
}

namespace SAR {
namespace UI {

/**
 * @brief 加载图像
 * @param parent 父窗口
 * @param filePath 文件路径
 * @param imageHandler 图像处理器
 * @param imageScene 图像场景
 * @param imageView 图像视图
 * @param logCallback 日志回调函数
 * @return 是否加载成功
 */
bool loadImage(QWidget *parent, const QString &filePath,
               SAR::Core::ImageHandler *imageHandler,
               QGraphicsScene *imageScene,
               QGraphicsView *imageView,
               const std::function<void(const QString&)>& logCallback);

/**
 * @brief 判断文件是否是支持的图像格式
 * @param filePath 文件路径
 * @return 是否支持
 */
bool isSupportedImageFormat(const QString &filePath);

/**
 * @brief 处理拖放的文件
 * @param parent 父窗口
 * @param filePath 文件路径
 * @param imageHandler 图像处理器
 * @param imageScene 图像场景
 * @param imageView 图像视图
 * @param currentImagePath 当前图像路径的引用
 * @param loadedImages 已加载图像列表
 * @param imageListWidget 图像列表控件
 * @param logCallback 日志回调函数
 * @param statusCallback 状态更新回调函数
 * @param enableButtonsCallback 启用按钮回调函数
 * @return 是否处理成功
 */
bool handleDroppedFile(QWidget *parent, const QString &filePath,
                     SAR::Core::ImageHandler *imageHandler,
                     QGraphicsScene *imageScene,
                     QGraphicsView *imageView,
                     QString &currentImagePath,
                     QStringList &loadedImages,
                     QListWidget *imageListWidget,
                     const std::function<void(const QString&)>& logCallback,
                     const std::function<void(const QString&)>& statusCallback,
                     const std::function<void(bool)>& enableButtonsCallback);

} // namespace UI
} // namespace SAR

#endif // IMAGE_HANDLER_H 