#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickGraphicsConfiguration>
#include "cameraImageProvider.h"
#include "cameraclient.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    auto* provider = new CameraImageProvider();
    engine.addImageProvider("camera", provider);

    CameraClient client(provider);
    engine.rootContext()->setContextProperty("camClient", &client);
    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &client, &CameraClient::closeCamera);
    engine.load("./QMLContent/App.qml");

    QMetaObject::invokeMethod(&client, "startup", Qt::QueuedConnection);

    return app.exec();
}
