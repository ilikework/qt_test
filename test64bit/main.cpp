//#include <QGuiApplication>
//#include <QQmlApplicationEngine>
//#include <QSurfaceFormat>
//#include "mycustom3ditem.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickGraphicsConfiguration>
#include "cameraImageProvider.h"
#include "cameraclient.h"

int main(int argc, char *argv[])
{
    // // --- ここから追加 ---
    // QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    // QSurfaceFormat format;
    // // マルチサンプリングバッファを有効にし、サンプル数を設定
    // format.setSamples(4); // 4倍MSAA を要求
    // QSurfaceFormat::setDefaultFormat(format);
    // // ------------------

    // QGuiApplication app(argc, argv);
    // QQmlApplicationEngine engine;

    // // ... 通常のエンジンロードと実行 ...
    // //const QUrl url(u"qrc:/YourProjectName/main.qml"_qs);
    // QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
    //                  &app, []() { QCoreApplication::exit(-1); },
    //                  Qt::QueuedConnection);
    // engine.load("D:/MagicMirror/git/test64bit/QMLContent/App.qml");

    // return app.exec();

    //QGuiApplication app(argc, argv);

    // // 1. 强制使用 OpenGL 作为 Qt Quick 的渲染后端
    // QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    // // 2. 可选：指定默认的 OpenGL format（版本、深度缓冲等）
    // QSurfaceFormat fmt;
    // fmt.setRenderableType(QSurfaceFormat::OpenGL);
    // fmt.setProfile(QSurfaceFormat::CoreProfile);
    // fmt.setVersion(3, 3);
    // fmt.setDepthBufferSize(24);
    // fmt.setStencilBufferSize(8);
    // QSurfaceFormat::setDefaultFormat(fmt);


    QApplication app(argc, argv);

    // qmlRegisterType<MyCustom3DItem>("Custom3D", 1, 0, "My3DItem");
   // QSurfaceFormat format;
   // format.setSamples(4); // 4倍MSAA を要求
   // QSurfaceFormat::setDefaultFormat(format);

    QQmlApplicationEngine engine;

    // QObject::connect(
    //     &engine,
    //     &QQmlApplicationEngine::objectCreationFailed,
    //     &app,
    //     []() { QCoreApplication::exit(-1); },
    //     Qt::QueuedConnection);

    //engine.addImportPath(":/");
    //engine.addImportPath("qrc:///");


    auto* provider = new CameraImageProvider();
    engine.addImageProvider("camera", provider);

    CameraClient client(provider);
    engine.rootContext()->setContextProperty("camClient", &client);
    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &client, &CameraClient::closeCamera);

    engine.load("./QMLContent/App.qml");

    //engine.loadFromModule("test64bit", "TestWindow");
    //engine.load("D:/MagicMirror/git/test64bit/QMLContent/testWindow.qml");
    //engine.loadFromModule("test64bit", "qrc:/QMLContent/testWindow.qml");
    //engine.addImportPath("D:/MagicMirror/git/test64bit/QMLContent");

    // ⭐ QML 创建完之后，立即自检相机是否可以connect
    QMetaObject::invokeMethod(&client, "startup", Qt::QueuedConnection);


    return app.exec();
}
