#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "CameraImageProvider.h"
#include "cameraclient.h"
#include "CustomerManager.h"
#include "appconfig.h"
#include "AnalyseManager.h"
#include "BackupManager.h"
#include "PreRecordManager.h"
#include "ImageEditor.h"
#include "MM3DManager.h"
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

    MM3DManager mm3dManager;
    engine.rootContext()->setContextProperty("mm3dManager", &mm3dManager);

    // Register custom QML types
    qmlRegisterType<ImageEditor>("com.magicmirror.components", 1, 0, "ImageEditor");

    // --- Load main QML file from Qt Resource System ---
    const QUrl url(QStringLiteral("qrc:/App.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    // --- Final setup and execution ---
    QMetaObject::invokeMethod(&client, "startup", Qt::QueuedConnection);

    return app.exec();
}
