#pragma once
#include <QObject>
#include <QJsonObject>

class AppConfig : public QObject {
    Q_OBJECT

public:
    static AppConfig& instance();

    int CameraSeries() const;
    int GetBeforeInterval(const QString & capture_type) const;
    int GetAfterInterval(const QString & capture_type) const;
    int BeforeRGBShootInterval() const;
    int AfterRGBShootInterval() const;
    int BeforeUVShootInterval() const;
    int AfterUVShootInterval() const;
    int BeforePLShootInterval() const;
    int AfterPLShootInterval() const;
    int BeforeNPLShootInterval() const;
    int AfterNPLShootInterval() const;
    QString GrayCreateFrom() const;
    QString RedCreateFrom() const;
    QString BrownCreateFrom() const;
    QString WholeCreateFrom() const;


    bool save();   // 写回 MMFace_.json
    bool reload(); // 重新读

signals:

private:
    explicit AppConfig(QObject* parent = nullptr);
    ~AppConfig() = default;

    Q_DISABLE_COPY_MOVE(AppConfig)

    bool load();
    QString configPath() const;

private:
    QJsonObject m_root;
};
