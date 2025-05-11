#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QObject>
#include <QQueue>
#include <QMutex>

// 添加必要的宏定义以正确包含 glog
#define GLOG_NO_ABBREVIATED_SEVERITIES
#define GOOGLE_GLOG_DLL_DECL // 添加这个宏定义
#define GLOG_CUSTOM_PREFIX_SUPPORT
#include <glog/logging.h>

#include <functional>

namespace SAR {
namespace Core {

/**
 * @brief Logger类 - 集中管理应用程序日志
 * 
 * 该类封装了glog库的功能，同时支持将日志同步到UI窗口
 */
class Logger : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 初始化日志系统
     * @param programName 程序名称，用于glog初始化
     * @param logToConsole 是否输出到控制台
     * @param logDir 日志文件目录，默认为空表示当前目录
     */
    static void init(const char* programName, bool logToConsole = true, const char* logDir = nullptr);
    
    /**
     * @brief 获取Logger单例实例
     * @return Logger实例
     */
    static Logger* instance();
    
    /**
     * @brief 记录INFO级别日志
     * @param message 日志消息
     * @param file 调用者文件名
     * @param line 调用者行号
     */
    void info(const QString& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 记录WARNING级别日志
     * @param message 日志消息
     * @param file 调用者文件名
     * @param line 调用者行号
     */
    void warning(const QString& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 记录ERROR级别日志
     * @param message 日志消息
     * @param file 调用者文件名
     * @param line 调用者行号
     */
    void error(const QString& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 记录FATAL级别日志
     * @param message 日志消息
     * @param file 调用者文件名
     * @param line 调用者行号
     */
    void fatal(const QString& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 获取最新的日志条目
     * @param maxCount 最大获取数量
     * @return 日志条目列表
     */
    QStringList getRecentLogs(int maxCount = 100);
    
signals:
    /**
     * @brief 新日志信号
     * @param message 日志消息
     */
    void newLogMessage(const QString& message);

private:
    Logger();
    ~Logger();
    
    static Logger* m_instance;
    QQueue<QString> m_recentLogs;
    QMutex m_mutex;
    static const int MAX_LOG_QUEUE_SIZE = 1000;
    
    void addToRecentLogs(const QString& message);
};

// 定义便捷宏，使用类似 glog 风格的接口，但传递文件和行号信息
#define LOG_INFO(msg) SAR::Core::Logger::instance()->info(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) SAR::Core::Logger::instance()->warning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) SAR::Core::Logger::instance()->error(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg) SAR::Core::Logger::instance()->fatal(msg, __FILE__, __LINE__)

} // namespace Core
} // namespace SAR

#endif // LOGGER_H 