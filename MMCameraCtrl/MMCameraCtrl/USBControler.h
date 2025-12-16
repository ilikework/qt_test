#pragma once
#include "IControler.h"

extern  "C" {
#include "hidsdi.h"
#include "setupapi.h"
}
#pragma comment(lib,"hid.lib")
#pragma comment(lib,"setupapi.lib")

#define ON  1
#define OFF 0

struct bitstru
{
	unsigned b0 : 1;
	unsigned b1 : 1;
	unsigned b2 : 1;
	unsigned b3 : 1;
	unsigned b4 : 1;
	unsigned b5 : 1;
	unsigned b6 : 1;
	unsigned b7 : 1;
};
union bunion
{
	unsigned char bData;
	struct bitstru bit;
};

class USBControler :
    public IControler
{
public:


	USBControler()
    {
		//m_nType = 0;
		m_hFile = NULL;
		m_hReadFile = NULL;
		m_hDevInfo = NULL;
		m_bOpened = false;
		m_sError = L"";

		memset(&m_OverlappedRead, 0, sizeof(OVERLAPPED));
		memset(&m_OverlappedWrite, 0, sizeof(OVERLAPPED));

		VID = 0x10C4;
		PID = 0x82CD;
		VN = 0x0000;

    }

private:
	unsigned short VID; //= 0x10C4;
	unsigned short PID; //= 0x82CD;
	unsigned short VN; //= 0x0000;

	OVERLAPPED	m_OverlappedRead,m_OverlappedWrite;

	HANDLE m_hFile;
	HANDLE m_hReadFile;
	HDEVINFO m_hDevInfo;
	//LONG m_nType;
	bool m_bOpened;
	std::wstring m_sError;
	byte sendpacket[65];

	// 通过 IControler 继承
	void Init() override;
	bool SetRGBModeOn() override;
	bool SetRGBModeOff() override;
	bool SetUVModeOn() override;
	bool SetUVModeOff() override;
	bool SetLightTestOn() override;
	bool SetLightTestOff() override;
	bool SetPLModeOn() override;
	bool SetPLModeOff() override;
	bool SetNPLModeOn() override;
	bool SetNPLModeOff() override;
	bool SetAnimateOff() override;
	bool SetAnimateMode(int nMode) override;
	bool ConnectCameraByTime(int nPulseTime) override;


	int ReadData(void* buffer, int limit, int timeout);
	int WriteData(byte* buffer, int size);
	int PulseData(int nIndex, byte* buffer, int size);
	int PulseDataByPulseTime(int nPulseTime, int nIndex, byte* buffer, int size);
	int WriteDataOn(int nBit, int nIndex, byte* buffer, int size);
	int WriteDataOff(int nBit, int nIndex, byte* buffer, int size);
	byte SetBitOn(int nBit, byte byIn);
	byte SetBitOff(int nBit, byte byIn);


	// 通过 IControler 继承
	bool Open() override;


	// 通过 IControler 继承
	bool Close(void) override;

};

