#include "drag_drop_handler.h"

namespace SAR {
namespace UI {

bool handleDragEnter(QDragEnterEvent *event, 
                     const std::function<void(const QString&)>& logCallback,
                     const std::function<bool(const QString&)>& supportedFormatChecker) {
    logCallback(QObject::tr("接收到拖放事件"));

    // 只接受包含图像文件的拖放
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();
            logCallback(QObject::tr("拖放文件：%1").arg(filePath));

            if (supportedFormatChecker(filePath)) {
                logCallback(QObject::tr("接受拖放：%1").arg(filePath));
                event->acceptProposedAction();
                return true;
            }
        }
    }

    // 如果没有可接受的文件，记录拒绝信息并拒绝拖放
    logCallback(QObject::tr("拒绝拖放：没有支持的图像文件"));
    event->ignore();
    return false;
}

bool handleDrop(QDropEvent *event,
               const std::function<bool(const QString&)>& fileHandlerCallback,
               const std::function<void(const QString&)>& logCallback,
               const std::function<bool(const QString&)>& supportedFormatChecker) {
    logCallback(QObject::tr("处理拖放文件"));

    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();
            logCallback(QObject::tr("尝试处理拖放文件：%1").arg(filePath));

            if (supportedFormatChecker(filePath)) {
                bool result = fileHandlerCallback(filePath);
                if (result) {
                    logCallback(QObject::tr("成功处理拖放文件：%1").arg(filePath));
                    event->acceptProposedAction();
                    return true;
                }
            }
        }
    }

    event->ignore();
    return false;
}

} // namespace UI
} // namespace SAR 