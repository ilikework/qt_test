#pragma once

class IControler {
public:
    virtual ~IControler() {}

    virtual bool Open() = 0;
    virtual bool Close(void) = 0;
    virtual void Init() = 0;
    virtual bool SetRGBModeOn() = 0;
    virtual bool SetRGBModeOff() = 0;
    virtual bool SetUVModeOn() = 0;
    virtual bool SetUVModeOff() = 0;
    virtual bool SetLightTestOn() = 0;
    virtual bool SetLightTestOff() = 0;
    virtual bool SetPLModeOn() = 0;
    virtual bool SetPLModeOff() = 0;
    virtual bool SetNPLModeOn() = 0;
    virtual bool SetNPLModeOff() = 0;
    virtual bool SetAnimateOff() = 0;
    virtual bool SetAnimateMode(int nMode) = 0;
    virtual bool ConnectCameraByTime(int nPulseTime) = 0;
};
