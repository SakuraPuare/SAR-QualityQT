#ifndef DRAG_DROP_HANDLER_H
#define DRAG_DROP_HANDLER_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QMimeData>
#include <functional>

namespace SAR {
namespace UI {

/**
 * @brief 处理拖放进入事件
 * @param event 拖放进入事件
 * @param logCallback 日志回调函数
 * @param supportedFormatChecker 支持格式检查函数
 * @return 是否接受该事件
 */
bool handleDragEnter(QDragEnterEvent *event, 
                     const std::function<void(const QString&)>& logCallback,
                     const std::function<bool(const QString&)>& supportedFormatChecker);

/**
 * @brief 处理拖放事件
 * @param event 拖放事件
 * @param fileHandlerCallback 文件处理回调函数
 * @param logCallback 日志回调函数
 * @param supportedFormatChecker 支持格式检查函数
 * @return 是否处理成功
 */
bool handleDrop(QDropEvent *event,
               const std::function<bool(const QString&)>& fileHandlerCallback,
               const std::function<void(const QString&)>& logCallback,
               const std::function<bool(const QString&)>& supportedFormatChecker);

} // namespace UI
} // namespace SAR

#endif // DRAG_DROP_HANDLER_H 