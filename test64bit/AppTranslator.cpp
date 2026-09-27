#include "AppTranslator.h"

#include "AppConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QTranslator>

namespace {

bool isAutoLanguage(const QString &code)
{
    return code.compare(QStringLiteral("auto"), Qt::CaseInsensitive) == 0;
}

QString normalizeLanguageCode(QString code)
{
    code = code.trimmed();
    if (code.isEmpty())
        return QStringLiteral("zh_CN");
    if (isAutoLanguage(code))
        return QStringLiteral("auto");
    code.replace(QLatin1Char('-'), QLatin1Char('_'));
    if (code.compare(QStringLiteral("zh"), Qt::CaseInsensitive) == 0
        || code.compare(QStringLiteral("zh_cn"), Qt::CaseInsensitive) == 0
        || code == QStringLiteral("chs"))
        return QStringLiteral("zh_CN");
    if (code.compare(QStringLiteral("zh_tw"), Qt::CaseInsensitive) == 0
        || code.compare(QStringLiteral("zh_hk"), Qt::CaseInsensitive) == 0
        || code == QStringLiteral("cht"))
        return QStringLiteral("zh_TW");
    if (code.startsWith(QStringLiteral("en"), Qt::CaseInsensitive))
        return QStringLiteral("en");
    if (code.startsWith(QStringLiteral("ja"), Qt::CaseInsensitive))
        return QStringLiteral("ja");
    return code;
}

QString systemLanguageCode()
{
    const QStringList langs = QLocale::system().uiLanguages();
    for (const QString &lang : langs) {
        const QString norm = normalizeLanguageCode(lang);
        if (norm == QStringLiteral("zh_CN") || norm == QStringLiteral("zh_TW")
            || norm == QStringLiteral("en") || norm == QStringLiteral("ja"))
            return norm;
    }
    return QStringLiteral("en");
}

QString resolveLanguageCode(const QString &stored)
{
    if (isAutoLanguage(stored))
        return systemLanguageCode();
    return normalizeLanguageCode(stored);
}

QString storedLanguagePreference(const QString &code)
{
    if (isAutoLanguage(code))
        return QStringLiteral("auto");
    return normalizeLanguageCode(code);
}

QString uiLanguageTag(const QString &norm)
{
    if (norm == QStringLiteral("zh_CN"))
        return QStringLiteral("zh-CN");
    if (norm == QStringLiteral("zh_TW"))
        return QStringLiteral("zh-TW");
    if (norm == QStringLiteral("ja"))
        return QStringLiteral("ja");
    return QStringLiteral("en");
}

void applyEngineLanguage(QQmlApplicationEngine *engine, const QString &resolved)
{
    if (!engine)
        return;
    engine->setUiLanguage(uiLanguageTag(resolved));
    engine->retranslate();
}

} // namespace

AppTranslator &AppTranslator::instance()
{
    static AppTranslator inst;
    return inst;
}

AppTranslator::AppTranslator(QObject *parent)
    : QObject(parent)
    , translator_(new QTranslator(this))
{
}

AppTranslator::~AppTranslator() = default;

QString AppTranslator::qmFileName(const QString &languageCode) const
{
    const QString norm = normalizeLanguageCode(languageCode);
    return QStringLiteral("mmface_%1.qm").arg(norm);
}

bool AppTranslator::loadTranslator(const QString &languageCode)
{
    const QString norm = normalizeLanguageCode(languageCode);
    QCoreApplication::removeTranslator(translator_);

    const QString i18nDir = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("i18n"));
    const QString qmPath = QDir(i18nDir).filePath(qmFileName(norm));
    if (!QFile::exists(qmPath)) {
        qWarning("AppTranslator: missing %s", qPrintable(qmPath));
        currentLanguage_ = norm;
        return false;
    }

    if (!translator_->load(qmPath)) {
        qWarning("AppTranslator: failed to load %s", qPrintable(qmPath));
        currentLanguage_ = norm;
        return false;
    }

    QCoreApplication::installTranslator(translator_);
    currentLanguage_ = norm;
    return true;
}

void AppTranslator::bumpRevision()
{
    ++revision_;
    emit languageChanged();
}

QString AppTranslator::languagePreference() const
{
    return storedLanguagePreference(AppConfig::instance().language());
}

bool AppTranslator::installForStartup(QQmlApplicationEngine *engine)
{
    engine_ = engine;
    const QString stored = storedLanguagePreference(AppConfig::instance().language());
    const QString resolved = resolveLanguageCode(stored);
    const bool ok = loadTranslator(resolved);
    applyEngineLanguage(engine_, resolved);
    bumpRevision();
    return ok;
}

bool AppTranslator::setLanguage(const QString &languageCode)
{
    const QString stored = storedLanguagePreference(languageCode);
    const QString resolved = resolveLanguageCode(stored);

    if (languagePreference() == stored && resolved == currentLanguage_) {
        const QString i18nDir = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("i18n"));
        if (QFile::exists(QDir(i18nDir).filePath(qmFileName(resolved))))
            return true;
    }

    AppConfig::instance().setLanguage(stored);
    const bool ok = loadTranslator(resolved);
    bumpRevision();
    applyEngineLanguage(engine_, resolved);

    return ok;
}

QStringList AppTranslator::availableLanguages() const
{
    return {
        QStringLiteral("zh_CN"),
        QStringLiteral("zh_TW"),
        QStringLiteral("en"),
        QStringLiteral("ja"),
    };
}

QStringList AppTranslator::languageOptions() const
{
    return {
        QStringLiteral("auto"),
        QStringLiteral("zh_CN"),
        QStringLiteral("zh_TW"),
        QStringLiteral("en"),
        QStringLiteral("ja"),
    };
}

QString AppTranslator::languageDisplayName(const QString &languageCode) const
{
    const QString norm = normalizeLanguageCode(languageCode);
    if (norm == QStringLiteral("auto"))
        return QStringLiteral("auto");
    if (norm == QStringLiteral("zh_CN"))
        return QStringLiteral("简体中文");
    if (norm == QStringLiteral("zh_TW"))
        return QStringLiteral("繁體中文");
    if (norm == QStringLiteral("en"))
        return QStringLiteral("English");
    if (norm == QStringLiteral("ja"))
        return QStringLiteral("日本語");
    return norm;
}

QString AppTranslator::translateText(const QString &source) const
{
    return QCoreApplication::translate("", source.toUtf8().constData());
}
