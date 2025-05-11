#include "image_handler.h"
#include "../core/include/image_handler.h"
#include <QGraphicsPixmapItem>
#include <QDir>

namespace SAR {
namespace UI {

bool loadImage(QWidget *parent, const QString &filePath,
              SAR::Core::ImageHandler *imageHandler,
              QGraphicsScene *imageScene,
              QGraphicsView *imageView,
              const std::function<void(const QString&)>& logCallback) {
    // 使用 GDAL (ImageHandler) 加载图像
    if (imageHandler->loadImage(filePath)) {
        // 从 ImageHandler 获取显示用的 QPixmap
        QPixmap pixmap = imageHandler->getDisplayPixmap(imageView->size());

        if (pixmap.isNull()) {
            QMessageBox::warning(
                parent, QObject::tr("图像显示失败"),
                QObject::tr("无法将 GDAL 图像转换为可显示格式：%1").arg(filePath));
            return false;
        }

        // 清除当前场景
        imageScene->clear();

        // 添加图像到场景
        QGraphicsPixmapItem *item = imageScene->addPixmap(pixmap);
        imageScene->setSceneRect(item->boundingRect());

        // 显示图像信息
        QString dimensions = imageHandler->getDimensionsString();
        QString dataType = imageHandler->getDataTypeString();
        logCallback(QObject::tr("图像信息：尺寸=%1, 数据类型=%2").arg(dimensions).arg(dataType));

        // 调整视图以适应图像
        imageView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);

        return true;
    } else {
        // GDAL 加载失败，尝试使用 Qt 的图像读取器作为备选方案
        QImageReader reader(filePath);
        QImage image = reader.read();

        if (image.isNull()) {
            QMessageBox::warning(parent, QObject::tr("图像加载失败"),
                             QObject::tr("无法使用 GDAL 或 Qt 加载图像文件：%1\n错误：%2")
                                 .arg(filePath)
                                 .arg(reader.errorString()));
            return false;
        }

        // 清除当前场景
        imageScene->clear();

        // 添加图像到场景
        QGraphicsPixmapItem *item =
            imageScene->addPixmap(QPixmap::fromImage(image));
        imageScene->setSceneRect(item->boundingRect());

        // 记录日志
        logCallback(QObject::tr("使用 Qt 加载图像：%1（GDAL 加载失败）")
              .arg(QFileInfo(filePath).fileName()));

        // 调整视图以适应图像
        imageView->fitInView(imageScene->sceneRect(), Qt::KeepAspectRatio);

        return true;
    }
}

bool isSupportedImageFormat(const QString &filePath) {
    // 对于使用 GDAL 后，支持更多格式
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    // 基本图像格式 + 一些常见的遥感格式
    return (suffix == "tif" || suffix == "tiff" || suffix == "jpg" ||
            suffix == "jpeg" || suffix == "png" || suffix == "bmp" ||
            suffix == "img" || suffix == "ceos" || suffix == "hdf" ||
            suffix == "hdf5" || suffix == "ers" || suffix == "bil" ||
            suffix == "bsq" || suffix == "nitf" || suffix == "h5");
}

bool handleDroppedFile(QWidget *parent, const QString &filePath,
                     SAR::Core::ImageHandler *imageHandler,
                     QGraphicsScene *imageScene,
                     QGraphicsView *imageView,
                     QString &currentImagePath,
                     QStringList &loadedImages,
                     QListWidget *imageListWidget,
                     const std::function<void(const QString&)>& logCallback,
                     const std::function<void(const QString&)>& statusCallback,
                     const std::function<void(bool)>& enableButtonsCallback) {
    if (loadImage(parent, filePath, imageHandler, imageScene, imageView, logCallback)) {
        currentImagePath = filePath;
        QFileInfo fileInfo(filePath);

        // 添加到图像列表
        if (!loadedImages.contains(filePath)) {
            loadedImages.append(filePath);
            imageListWidget->addItem(fileInfo.fileName());
            imageListWidget->setCurrentRow(imageListWidget->count() - 1);
        }

        logCallback(QObject::tr("已加载拖放图像：%1").arg(fileInfo.fileName()));
        statusCallback(QObject::tr("拖放图像已加载：%1").arg(fileInfo.fileName()));

        // 启用分析按钮
        enableButtonsCallback(true);
        
        return true;
    }
    
    return false;
}

} // namespace UI
} // namespace SAR 