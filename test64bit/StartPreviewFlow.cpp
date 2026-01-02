#include "StartPreviewFlow.h"
#include "CameraClient.h"
#include "AppConfig.h"
#include "AppDb.h"

void StartPreviewFlow::start()
{

    QJsonObject obj;
    obj["cmd"] = "startpreview";

    cli_->sendCommandRequest2(obj, [this](const QJsonObject& resp){
        if (aborted_) return;
        if (!isOkResult(resp)) {
            aborted_ = true;
            emit finished(false, QString("StartPreviewFlow failed at startpreview"));
            deleteLater();
            return;
        }
        setting_camera_params();
    });
}

void StartPreviewFlow::setting_camera_params()
{
    CaptureSetting s;
    AppDb::instance().loadCaptureSettingBySeries(AppConfig::instance().CameraSeries(),s);
    QStringList keys = {"iso", "exposuretime", "aperture","wb"};
    int values[] = {s.rgb_iso,s.rgb_exposureTime,s.rgb_aperture,s.rgb_wb};
    QJsonObject obj;
    obj["cmd"] = "preview_setting";
    obj["imageSize"] = s.imageSize;
    obj["imageQuality"] = s.imageQuality;

    QJsonObject params;
    for (int i = 0; i < keys.size(); ++i) {
        const QString& key = keys[i];
        int& value = values[i];

        params[key] = value;
    }
    obj["params"] = params;

    cli_->sendCommandRequest2(obj, [this](const QJsonObject& resp){
        if (aborted_) return;
        if (!isOkResult(resp)) {
            aborted_ = true;
            emit finished(false, QString("StartPreviewFlow failed at setting_camera_params"));
            deleteLater();
            return;
        }
        emit finished(true, QString("StartPreviewFlow finished"));

    });
}
