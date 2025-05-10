#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include <QDir>
#include <QDebug>

#include "ui/mainwindow.h"
#include <gdal_priv.h>

int main(int argc, char *argv[])
{
    // 创建应用程序实例
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("SAR-QualityQT");
    QApplication::setApplicationVersion("0.1");
    QApplication::setOrganizationName("SAR-Team");
    QApplication::setOrganizationDomain("sar-quality.org");
    
    // 初始化GDAL库
    GDALAllRegister();
    
    // 创建翻译器
    QTranslator translator;
    
    // 查找翻译文件路径
    QStringList paths;
    paths << QCoreApplication::applicationDirPath() + "/i18n/";
    paths << ":/i18n/";
    
    // 获取系统语言
    QString locale = QLocale::system().name();
    
    // 尝试加载对应的翻译文件
    bool translationLoaded = false;
    for (const QString &path : paths) {
        if (translator.load(path + "SAR-QualityQT_" + locale)) {
            app.installTranslator(&translator);
            qDebug() << "加载翻译文件:" << path + "SAR-QualityQT_" + locale;
            translationLoaded = true;
            break;
        }
    }
    
    if (!translationLoaded && locale.startsWith("zh_")) {
        // 如果特定的中文翻译未找到，尝试加载通用中文翻译
        for (const QString &path : paths) {
            if (translator.load(path + "SAR-QualityQT_zh_CN")) {
                app.installTranslator(&translator);
                qDebug() << "加载翻译文件:" << path + "SAR-QualityQT_zh_CN";
                break;
            }
        }
    }
    
    // 创建并显示主窗口
    SAR::UI::MainWindow mainWindow;
    mainWindow.show();
    
    // 运行应用程序的事件循环
    return app.exec();
} 