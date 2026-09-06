#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "CameraImageProvider.h"
#include "cameraclient.h"
#include "CustomerManager.h"
#include "appconfig.h"
#include "AppTranslator.h"
#include "MmI18n.h"
#include "AnalyseManager.h"
#include "BackupManager.h"
#include "PreRecordManager.h"
#include "CustomerReportManager.h"
#include "PrintHelper.h"
#include "ImageEditor.h"
#include "MM3DManager.h"
#include "FaceAnalyseManager.h"
#include <QCoreApplication> // For applicationDirPath

int main(int argc, char *argv[])
{

    QApplication app(argc, argv); // Use QApplication for widgets integration if needed
    QQmlApplicationEngine engine;

    // --- Register C++ objects and types to QML ---

    // Register Image Provider
    auto* provider = new CameraImageProvider();
    engine.addImageProvider("camera", provider);

    // Expose applicationDirPath to QML
    engine.rootContext()->setContextProperty("applicationDirPath", QCoreApplication::applicationDirPath());

    // Register Singleton/Global Modules
    engine.rootContext()->setContextProperty("appConfig", &AppConfig::instance());
    engine.rootContext()->setContextProperty("appTranslator", &AppTranslator::instance());
    engine.rootContext()->setContextProperty("mmI18n", &MmI18n::instance());

    QObject::connect(&AppTranslator::instance(), &AppTranslator::languageChanged,
                     &MmI18n::instance(), &MmI18n::retranslate);

    // Register invokable C++ objects
    CameraClient client(provider); // client depends on provider
    engine.rootContext()->setContextProperty("camClient", &client);

    CustomerManager customerModule;
    engine.rootContext()->setContextProperty("customerModule", &customerModule);
    
    AnalyseManager analyseModule;
    engine.rootContext()->setContextProperty("analyseModule", &analyseModule);

    BackupManager backupMgr;
    engine.rootContext()->setContextProperty("backupManager", &backupMgr);
    
    PreRecordManager preRecordMgr;
    engine.rootContext()->setContextProperty("preRecordManager", &preRecordMgr);

    CustomerReportManager customerReportMgr;
    engine.rootContext()->setContextProperty("customerReportManager", &customerReportMgr);

    PrintHelper printHelper;
    engine.rootContext()->setContextProperty("printHelper", &printHelper);

    MM3DManager mm3dManager;
    engine.rootContext()->setContextProperty("mm3dManager", &mm3dManager);

    FaceAnalyseManager faceAnalyseManager;
    engine.rootContext()->setContextProperty("faceAnalyseManager", &faceAnalyseManager);

    // Register custom QML types
    qmlRegisterType<ImageEditor>("com.magicmirror.components", 1, 0, "ImageEditor");

    AppTranslator::instance().installForStartup(&engine);

    // --- Load main QML file from Qt Resource System ---
    const QUrl url(QStringLiteral("qrc:/App.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &client, &CameraClient::closeCamera);
    engine.load(url);

    // --- Final setup and execution ---
    QMetaObject::invokeMethod(&client, "startup", Qt::QueuedConnection);

    return app.exec();
}
