#ifndef MMLOGGER_H
#define MMLOGGER_H

#include <QString>
#include <QMutex>

/**
 * 通用单例日志类，按天写文件 log_yyyy-MM-dd.log，线程安全。DEBUG 版同时输出到标准输出。
 * 使用方式：LOG("message") 或 LOG(QString("code=%1").arg(code))
 */
class MMLogger
{
public:
    static MMLogger& instance();

    void writeLog(const QString& message);
    void writeLog(const char* message);

private:
    MMLogger() = default;
    ~MMLogger() = default;
    MMLogger(const MMLogger&) = delete;
    MMLogger& operator=(const MMLogger&) = delete;

    QString getTodayLogFilePath() const;
    QMutex mutex_;
};

#define LOG(msg) MMLogger::instance().writeLog(msg)

#endif // MMLOGGER_H
