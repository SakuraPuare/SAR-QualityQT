#ifndef DRAG_DROP_GRAPHICS_VIEW_H
#define DRAG_DROP_GRAPHICS_VIEW_H

#include <QGraphicsView>
#include <QDragEnterEvent>
#include <QDropEvent>

/**
 * @brief 自定义 GraphicsView 类，支持拖放功能
 */
class DragDropGraphicsView : public QGraphicsView
{
    Q_OBJECT
    
public:
    explicit DragDropGraphicsView(QWidget *parent = nullptr) : QGraphicsView(parent) {
        setAcceptDrops(true);
    }
    
protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        emit dragEnterReceived(event);
    }
    
    void dropEvent(QDropEvent *event) override {
        emit dropReceived(event);
    }
    
signals:
    void dragEnterReceived(QDragEnterEvent *event);
    void dropReceived(QDropEvent *event);
};

#endif // DRAG_DROP_GRAPHICS_VIEW_H 