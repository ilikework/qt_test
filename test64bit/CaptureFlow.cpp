#include "CaptureFlow.h"
#include "CameraClient.h"
#include "AppDb.h"
#include "AppConfig.h"
#include <QCoreApplication>
#include <qjsonarray.h>
#include "libs/dllProc.h"
#include <vector>
#include <functional>
using FunctionType = std::function<int(wchar_t*, wchar_t*)>;

#define DIR_CUSTOMERS "/customers"

void CaptureFlow::start()
{
    if (running_) return;
    running_ = true;

    setting_camera_params();
}

void CaptureFlow::setting_camera_params(const bool bcontinue)
{
    CaptureSetting s;
    AppDb::instance().loadCaptureSettingBySeries(AppConfig::instance().CameraSeries(),s);
    QStringList keys = {"iso", "exposuretime", "aperture","wb"};
    int values[4] ;
    if(capture_types[index_]=="RGB")
    {
        values[0] = s.rgb_iso;
        values[1] = s.rgb_exposureTime;
        values[2] = s.rgb_aperture;
        values[3] = s.rgb_wb;
    }
    else if(capture_types[index_]=="UV")
    {
        values[0] = s.uv_iso;
        values[1] = s.uv_exposureTime;
        values[2] = s.uv_aperture;
        values[3] = s.uv_wb;
    }
    else if(capture_types[index_]=="PL")
    {
        values[0] = s.pl_iso;
        values[1] = s.pl_exposureTime;
        values[2] = s.pl_aperture;
        values[3] = s.pl_wb;
    }
    else if(capture_types[index_]=="NPL")
    {
        values[0] = s.npl_iso;
        values[1] = s.npl_exposureTime;
        values[2] = s.npl_aperture;
        values[3] = s.npl_wb;
    }

    QJsonObject obj;
    obj["cmd"] = "capture_setting";

    QJsonObject params;
    for (int i = 0; i < keys.size(); ++i) {
        const QString& key = keys[i];
        int& value = values[i];

        params[key] = value;
    }
    params["capture_type"] =capture_types[index_];
    obj["params"] = params;

    cli_->sendCommandRequest2(obj, [this,bcontinue](const QJsonObject& resp){
        if (aborted_) return;
        if (!isOkResult(resp)) {
            aborted_ = true;
            emit finished(false, QString("capture failed at #%1").arg(index_ + 1));
            deleteLater();
            return;
        }
        if(bcontinue)
            stepDelayBeforeCapture();
    });
}

void CaptureFlow::stepDelayBeforeCapture()
{
    if (aborted_) return;
    QTimer::singleShot(AppConfig::instance().GetBeforeInterval(capture_types[index_]), this, [this]{
        stepSendCapture();
    });
}

void CaptureFlow::stepDelayAfterCapture()
{
    if (aborted_) return;
    QTimer::singleShot(AppConfig::instance().GetAfterInterval(capture_types[index_]), this, [this]{
        next();
    });
}

void CaptureFlow::stepSendCapture()
{
    if (aborted_) return;

    QJsonObject cap;
    cap["cmd"] = "capture";
    cap["save_folder"] =  QCoreApplication::applicationDirPath()
                         + DIR_CUSTOMERS
                         + "/" +strCustomID_
                         + "/" +QString("%1").arg(nGroupID_, 2, 10, QChar('0'));

    cap["capture_id"] = index_+1;
    cap["capture_type"] = capture_types[index_];

    cli_->sendCommandRequest2(cap, [this](const QJsonObject& resp){
        if (aborted_) return;
        if (!isOkResult(resp)) {
            aborted_ = true;
            emit finished(false, QString("capture failed at #%1").arg(index_ + 1));
            deleteLater();
            return;
        }

        // 本张完成
        QJsonArray arr = resp.value("created_files").toArray();
        cli_->addleft(arr[1].toString());
        cli_->addright(arr[2].toString());

        done_++;
        emit progress(done_, index_);
        stepDelayAfterCapture();
    });
}

QString CaptureFlow::get_captured_filename(const int index) const
{
    QString ret = QCoreApplication::applicationDirPath()
                + DIR_CUSTOMERS
                    + "/" +strCustomID_
                    + "/" +QString("%1").arg(nGroupID_, 2, 10, QChar('0'));
    ret += QString("/%1").arg(index, 2, 10, QChar('0'));
    return ret;
}

void CaptureFlow::next()
{
    if (done_ >= capture_types.length()) {
        // add auto create pictures.

        // from UV to GRAY.
        QString strFrom = get_captured_filename(2);
        strFrom += LEFT;
        strFrom += JPG;
        QString strTo = get_captured_filename(5);
        strTo += LEFT;
        strTo += JPG;

        Gray_Enhance(strFrom.toStdWString().data(),strTo.toStdWString().data());

        emit finished(true, "capture finished");
        index_=0;
        setting_camera_params(false);
        auto_create_pictures();
        return;
    }
    index_++;
    setting_camera_params();

}

int CaptureFlow::get_capture_index(const QString & str) const
{
    for(int i=0;i<capture_types.size();i++)
    {
        if(str==capture_types[i])
            return i+1;
    }
    return 0;
}

void CaptureFlow::auto_create_pictures()
{
    const int nFromCapType[] = {get_capture_index(AppConfig::instance().GrayCreateFrom()),
                      get_capture_index(AppConfig::instance().RedCreateFrom()),
                      get_capture_index(AppConfig::instance().BrownCreateFrom()),
                      get_capture_index(AppConfig::instance().WholeCreateFrom())};
    std::vector<FunctionType> functions = {
            Gray_Enhance,
            Extract_Red_Skin,
            Extract_Melanin,
            Enhance_Spots
    };

    for (int i = 0; i < autocreate_types.size(); ++i) {
        QString strFrom = get_captured_filename(nFromCapType[i]);
        strFrom += LEFT;
        strFrom += JPG;
        QString strTo = get_captured_filename(i+5);
        strTo += LEFT;
        strTo += JPG;
        if(functions[i](strFrom.toStdWString().data(),strTo.toStdWString().data())<0)
            qWarning() << "Failed to create from: " << strFrom << " to: " << strTo;
        cli_->addleft(strTo);

        strFrom = get_captured_filename(nFromCapType[i]);
        strFrom += RIGHT;
        strFrom += JPG;
        strTo = get_captured_filename(i+5);
        strTo += RIGHT;
        strTo += JPG;
        if(functions[i](strFrom.toStdWString().data(),strTo.toStdWString().data())<0)
            qWarning() << "Failed to create from: " << strFrom << " to: " << strTo;
        cli_->addright(strTo);

        strFrom = get_captured_filename(nFromCapType[i]);
        strFrom += JPG;
        strTo = get_captured_filename(i+5);
        strTo += JPG;
        if(functions[i](strFrom.toStdWString().data(),strTo.toStdWString().data())<0)
            qWarning() << "Failed to create from: " << strFrom << " to: " << strTo;

    }
}
