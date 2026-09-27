#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class QQmlApplicationEngine;
class QTranslator;

/** 加载 i18n/*.qm，切换语言并触发 QML retranslate */
class AppTranslator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString languagePreference READ languagePreference NOTIFY languageChanged)
    Q_PROPERTY(int revision READ revision NOTIFY languageChanged)

public:
    static AppTranslator &instance();

    QString currentLanguage() const { return currentLanguage_; }
    QString languagePreference() const;
    int revision() const { return revision_; }

    /** 启动时安装翻译器（在 engine.load 之前调用） */
    bool installForStartup(QQmlApplicationEngine *engine);

    /** QML 切换语言：写 MMFace_.json + 重载 .qm + engine.retranslate() */
    Q_INVOKABLE bool setLanguage(const QString &languageCode);

    Q_INVOKABLE QStringList availableLanguages() const;
    Q_INVOKABLE QStringList languageOptions() const;
    Q_INVOKABLE QString languageDisplayName(const QString &languageCode) const;
    /** 与 mmTr / 空 context .qm 一致，供 QML 使用（不依赖 qsTr 文件 context） */
    Q_INVOKABLE QString translateText(const QString &source) const;

signals:
    void languageChanged();

private:
    explicit AppTranslator(QObject *parent = nullptr);
    ~AppTranslator() override;

    QString qmFileName(const QString &languageCode) const;
    bool loadTranslator(const QString &languageCode);
    void bumpRevision();

    QQmlApplicationEngine *engine_ = nullptr;
    QTranslator *translator_ = nullptr;
    QString currentLanguage_;
    int revision_ = 0;
};
