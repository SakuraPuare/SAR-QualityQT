#include "include/logger.h"
#include <QDateTime>
#include <QDir>
#include <glog/logging.h>

namespace SAR {
namespace Core {

Logger* Logger::m_instance = nullptr;

void Logger::init(const char* programName, bool logToConsole, const char* logDir) {
    // 初始化 glog
    google::InitGoogleLogging(programName);
    
    // 设置日志目录
    if (logDir) {
        QDir dir(logDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        FLAGS_log_dir = logDir;
    }
    
    // 设置日志选项
    FLAGS_logtostderr = logToConsole;  // 是否输出到 stderr
    FLAGS_colorlogtostderr = true;     // 彩色日志
    FLAGS_minloglevel = google::GLOG_INFO;  // 最小日志级别
    FLAGS_logbufsecs = 0;              // 立即刷新
    
    // 确保创建实例
    instance();
}

Logger* Logger::instance() {
    if (!m_instance) {
        m_instance = new Logger();
    }
    return m_instance;
}

Logger::Logger() : QObject(nullptr) {
}

Logger::~Logger() {
    google::ShutdownGoogleLogging();
}

void Logger::info(const QString& message) {
    std::string stdMsg = message.toStdString();
    LOG(INFO) << stdMsg;
    
    QString formattedMsg = QString("[INFO][%1] %2")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(message);
        
    addToRecentLogs(formattedMsg);
    emit newLogMessage(formattedMsg);
}

void Logger::warning(const QString& message) {
    std::string stdMsg = message.toStdString();
    LOG(WARNING) << stdMsg;
    
    QString formattedMsg = QString("[WARNING][%1] %2")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(message);
        
    addToRecentLogs(formattedMsg);
    emit newLogMessage(formattedMsg);
}

void Logger::error(const QString& message) {
    std::string stdMsg = message.toStdString();
    LOG(ERROR) << stdMsg;
    
    QString formattedMsg = QString("[ERROR][%1] %2")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(message);
        
    addToRecentLogs(formattedMsg);
    emit newLogMessage(formattedMsg);
}

void Logger::fatal(const QString& message) {
    std::string stdMsg = message.toStdString();
    LOG(FATAL) << stdMsg;
    
    QString formattedMsg = QString("[FATAL][%1] %2")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(message);
        
    addToRecentLogs(formattedMsg);
    emit newLogMessage(formattedMsg);
}

void Logger::addToRecentLogs(const QString& message) {
    QMutexLocker locker(&m_mutex);
    
    m_recentLogs.enqueue(message);
    if (m_recentLogs.size() > MAX_LOG_QUEUE_SIZE) {
        m_recentLogs.dequeue();
    }
}

QStringList Logger::getRecentLogs(int maxCount) {
    QMutexLocker locker(&m_mutex);
    
    QStringList result;
    int count = qMin(maxCount, m_recentLogs.size());
    
    for (int i = m_recentLogs.size() - count; i < m_recentLogs.size(); i++) {
        result.append(m_recentLogs[i]);
    }
    
    return result;
}

} // namespace Core
} // namespace SAR 