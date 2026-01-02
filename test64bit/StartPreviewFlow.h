#pragma once
#include "FlowBase.h"
#include <QJsonObject>

class CameraClient;

class StartPreviewFlow : public FlowBase
{
    Q_OBJECT
public:
    StartPreviewFlow(CameraClient* cli, QObject* parent=nullptr)
        : FlowBase(parent), cli_(cli) {}
    void start();

signals:
    void finished(bool ok, const QString& msg);


private:
    CameraClient* cli_ = nullptr;

    void setting_camera_params();

};

