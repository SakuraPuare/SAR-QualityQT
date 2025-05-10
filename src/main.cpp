#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include "ui/include/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序基本信息
    QCoreApplication::setOrganizationName("SAR");
    QCoreApplication::setApplicationName("SAR-QualityQT");
    QCoreApplication::setApplicationVersion("1.0");
    
    // 加载翻译文件
    QTranslator translator;
    QStringList paths;
    paths << QApplication::applicationDirPath() + "/i18n/"
          << QApplication::applicationDirPath() + "/../i18n/"
          << QApplication::applicationDirPath() + "/../../i18n/";
    
    // 查找语言环境
    QString locale = QLocale::system().name();
    
    if (locale.startsWith("zh_")) {
        // 如果特定的中文翻译未找到，尝试加载通用中文翻译
        for (const QString &path : paths) {
            if (translator.load(path + "SAR-QualityQT_zh_CN")) {
                app.installTranslator(&translator);
                qDebug() << "加载翻译文件：" << path + "SAR-QualityQT_zh_CN";
                break;
            }
        }
    }
    
    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.show();
    
    // 运行应用程序的事件循环
    return app.exec();
} 