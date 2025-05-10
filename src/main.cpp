#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include "ui/include/mainwindow.h"
#include "core/include/logger.h"
#include <gdal_priv.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序基本信息
    QCoreApplication::setOrganizationName("SAR");
    QCoreApplication::setApplicationName("SAR-QualityQT");
    QCoreApplication::setApplicationVersion("1.0");
    
    // 初始化日志系统
    QString logDir = QDir::current().absolutePath() + "/logs";
    QDir().mkpath(logDir); // 确保日志目录存在
    SAR::Core::Logger::init(argv[0], true, logDir.toStdString().c_str());
    
    // 记录应用程序启动日志
    LOG_INFO(QString("SAR-QualityQT 应用程序启动，版本：%1")
        .arg(QCoreApplication::applicationVersion()));
    
    // 初始化 GDAL，注册所有驱动
    GDALAllRegister();
    LOG_INFO("GDAL 初始化完成，已注册所有驱动");
    
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
                LOG_INFO(QString("加载翻译文件：%1").arg(path + "SAR-QualityQT_zh_CN"));
                break;
            }
        }
    }
    
    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.show();
    
    LOG_INFO("主窗口已显示，应用程序启动完成");
    
    // 运行应用程序的事件循环
    return app.exec();
} 