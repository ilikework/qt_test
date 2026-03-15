#include "MMLogger.h"
#include <QCoreApplication>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QStringConverter>
#ifdef QT_DEBUG
#include <QDebug>
#endif

MMLogger& MMLogger::instance()
{
    static MMLogger inst;
    return inst;
}

QString MMLogger::getTodayLogFilePath() const
{
    QString baseDir = QCoreApplication::applicationDirPath();
    QString dateStr = QDateTime::currentDateTime().toString(Qt::ISODate).left(10); // yyyy-MM-dd
    return baseDir + QLatin1String("/log_") + dateStr + QLatin1String(".log");
}

void MMLogger::writeLog(const QString& message)
{
    QMutexLocker lock(&mutex_);
    QString timeStr = QDateTime::currentDateTime().toString(QLatin1String("HH:mm:ss"));
    QString line = QLatin1String("[") + timeStr + QLatin1String("] ") + message;

    QString path = getTodayLogFilePath();
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out.setEncoding(QStringConverter::Utf8);
        out << line << Qt::endl;
    }

#ifdef QT_DEBUG
    qDebug().noquote() << line;
#endif
}

void MMLogger::writeLog(const char* message)
{
    writeLog(QString::fromUtf8(message));
}
