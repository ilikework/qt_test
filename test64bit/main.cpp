#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickGraphicsConfiguration>
#include "cameraImageProvider.h"
#include "cameraclient.h"
#include "CustomerManager.h"
#include "AnalyseManager.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    auto* provider = new CameraImageProvider();
    engine.addImageProvider("camera", provider);

    CameraClient client(provider);
    engine.rootContext()->setContextProperty("camClient", &client);

    CustomerManager customerModule;
    engine.rootContext()->setContextProperty("customerModule", &customerModule);

    AnalyseManager analyseModule;
    engine.rootContext()->setContextProperty("analyseModule", &analyseModule);

    // 注入全局“应用根路径”
    engine.rootContext()->setContextProperty("applicationDirPath", QCoreApplication::applicationDirPath());

    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &client, &CameraClient::closeCamera);
    engine.load("./QMLContent/App.qml");

    QMetaObject::invokeMethod(&client, "startup", Qt::QueuedConnection);

    return app.exec();
}
