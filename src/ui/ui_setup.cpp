#include "ui_setup.h"
#include "ui_mainwindow.h"
#include "logger.h"
#include "drag_drop_handler.h"
#include "include/drag_drop_graphics_view.h"
#include <QLayout>
#include <QTabWidget>
#include <QCheckBox>

namespace SAR {
namespace UI {

void setupImageViewer(QMainWindow *parent, Ui::MainWindow *ui, 
                     QGraphicsScene *imageScene, DragDropGraphicsView *&imageView) {
    // 创建一个自定义的 DragDropGraphicsView
    imageView = new DragDropGraphicsView(parent);
    imageView->setScene(imageScene);
    imageView->setDragMode(QGraphicsView::ScrollHandDrag);

    // 将新创建的 DragDropGraphicsView 添加到界面布局中
    if (ui->imageDisplayLabel) {
        // 获取占位符的父 widget
        QWidget *parentWidget = ui->imageDisplayLabel->parentWidget();
        if (parentWidget) {
            // 获取占位符在布局中的位置
            QLayout *layout = parentWidget->layout();
            if (layout) {
                // 移除占位符标签
                layout->removeWidget(ui->imageDisplayLabel);
                // 添加图形视图到相同位置
                layout->addWidget(imageView);
                // 隐藏原标签
                ui->imageDisplayLabel->hide();
            }
        }
    }

    LOG_INFO("已设置图像视图 (支持拖放功能)");
}

void setupConnections(QMainWindow *parent, Ui::MainWindow *ui, DragDropGraphicsView *imageView) {
    // 连接图像视图的拖放信号
    QObject::connect(imageView, SIGNAL(dragEnterReceived(QDragEnterEvent*)),
                     parent, SLOT(handleViewDragEnter(QDragEnterEvent*)));
    QObject::connect(imageView, SIGNAL(dropReceived(QDropEvent*)),
                     parent, SLOT(handleViewDrop(QDropEvent*)));

    // 连接日志系统信号
    QObject::connect(SAR::Core::Logger::instance(), SIGNAL(newLogMessage(const QString&)),
                     parent, SLOT(onNewLogMessage(const QString&)));

    // 全选/取消全选按钮
    QObject::connect(ui->checkBoxSelectAll, &QCheckBox::toggled, [=](bool checked) {
        // 根据配置启用或禁用各个分析选项的代码...
        // (会由MainWindow中的槽函数处理)
    });

    // 当有图像加载时启用分析按钮
    QObject::connect(ui->imageListWidget, &QListWidget::itemSelectionChanged, [=]() {
        enableAnalysisButtons(ui, !ui->imageListWidget->selectedItems().isEmpty());
    });
}

void enableAnalysisButtons(Ui::MainWindow *ui, bool enable) {
    ui->startAnalysisButton->setEnabled(enable);
    ui->actionStartAssessment->setEnabled(enable);
    ui->actionSelectAssessmentRegion->setEnabled(enable);
    ui->pushButton_exportPDF->setEnabled(enable);
    ui->pushButton_exportTXT->setEnabled(enable);
}

void configureAnalysisOptions(Ui::MainWindow *ui, const std::function<void(const QString&)>& logCallback) {
    // 配置分析选项
    logCallback(QObject::tr("已加载分析选项"));

    // 默认禁用所有分析选项复选框
    ui->checkBoxSelectAll->setChecked(false);

    // 根据编译选项启用/禁用对应的分析选项
#if !CONFIG_ENABLE_ISLR
    ui->checkBoxISLR->setVisible(false);
#endif

#if !CONFIG_ENABLE_PSLR
    ui->checkBoxPSLR->setVisible(false);
#endif

#if !CONFIG_ENABLE_RANGE_RES
    if (ui->actionRangeResolution) ui->actionRangeResolution->setVisible(false);
#endif

#if !CONFIG_ENABLE_AZIMUTH_RES
    if (ui->actionAzimuthResolution) ui->actionAzimuthResolution->setVisible(false);
#endif

#if !CONFIG_ENABLE_RASR
    ui->checkBoxRASR->setVisible(false);
#endif

#if !CONFIG_ENABLE_AASR
    ui->checkBoxAASR->setVisible(false);
#endif

#if !CONFIG_ENABLE_SNR
    ui->checkBoxSNR->setVisible(false);
#endif

#if !CONFIG_ENABLE_NESZ
    ui->checkBoxNESZ->setVisible(false);
#endif

#if !CONFIG_ENABLE_RADIOMETRIC_ACC
    ui->checkBoxRadiometricAccuracy->setVisible(false);
#endif

#if !CONFIG_ENABLE_RADIOMETRIC_RES
    ui->checkBoxRadiometricResolution->setVisible(false);
#endif

#if !CONFIG_ENABLE_ENL
    ui->checkBoxENL->setVisible(false);
#endif

    // 配置结果选项卡的显示状态
    configureResultTabs(ui);

    logCallback(QObject::tr("已配置分析选项"));
}

void configureResultTabs(Ui::MainWindow *ui) {
    // 获取结果标签页组件
    QTabWidget *tabs = ui->resultsTabWidget;

    // 默认显示概览选项卡
    int overviewIndex = tabs->indexOf(ui->tabOverview);

    // 隐藏未启用的分析方法对应的结果选项卡
    for (int i = 0; i < tabs->count(); i++) {
        QWidget *tab = tabs->widget(i);
        bool shouldShow = true;

        // 跳过概览选项卡
        if (i == overviewIndex) {
            continue;
        }

        // 根据编译选项决定是否显示特定选项卡
        if (tab == ui->tabISLR) { // 积分旁瓣比
#if !CONFIG_ENABLE_ISLR
            shouldShow = false;
#endif
        } else if (tab == ui->tabPSLR) { // 峰值旁瓣比
#if !CONFIG_ENABLE_PSLR
            shouldShow = false;
#endif
        } else if (tab == ui->tabRASR) { // 距离模糊度
#if !CONFIG_ENABLE_RASR
            shouldShow = false;
#endif
        } else if (tab == ui->tabAASR) { // 方位模糊度
#if !CONFIG_ENABLE_AASR
            shouldShow = false;
#endif
        } else if (tab == ui->tabSNR) { // 信噪比
#if !CONFIG_ENABLE_SNR
            shouldShow = false;
#endif
        } else if (tab == ui->tabRadAccuracy) { // 辐射精度
#if !CONFIG_ENABLE_RADIOMETRIC_ACC
            shouldShow = false;
#endif
        } else if (tab == ui->tabNESZ) { // NESZ
#if !CONFIG_ENABLE_NESZ
            shouldShow = false;
#endif
        } else if (tab == ui->tabRadResolution) { // 辐射分辨率
#if !CONFIG_ENABLE_RADIOMETRIC_RES
            shouldShow = false;
#endif
        } else if (tab == ui->tabENL) { // 等效视数
#if !CONFIG_ENABLE_ENL
            shouldShow = false;
#endif
        }

        // 如果不应显示，则移除选项卡
        if (!shouldShow) {
            tabs->removeTab(i);
            i--; // 因为移除了当前选项卡，索引需要回退
        }
    }

    // 将当前选项卡设置为概览
    tabs->setCurrentIndex(tabs->indexOf(ui->tabOverview));
}

} // namespace UI
} // namespace SAR 