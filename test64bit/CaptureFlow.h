// CaptureFlow.h
#pragma once
#include "FlowBase.h"
#include <QTimer>
#include <QJsonObject>

class CameraClient;

class CaptureFlow : public FlowBase {
    Q_OBJECT
public:
    CaptureFlow(CameraClient* cli, QString strCustomID, int nGroupID, QObject* parent=nullptr)
        : FlowBase(parent), cli_(cli),strCustomID_(strCustomID),nGroupID_(nGroupID) {}

    void start();

signals:
    void progress(int done, int total);
    //void finished(bool ok, const QString& msg);

private:
    CameraClient* cli_ = nullptr;
    QString strCustomID_ ="";
    int nGroupID_ = 0;

    bool running_ = false;
    int index_ = 0;   // 当前第几张（0-based）
    int done_ = 0;    // 已完成张数
    QStringList capture_types = {"RGB","UV","PL","NPL"};

    void setting_camera_params(const bool bcontinue=true);
    void stepDelayBeforeCapture();
    void stepDelayAfterCapture();
    void stepSendCapture();
    void next();
};
