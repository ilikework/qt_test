#include "pch.h"
#include "USBControler.h"

const unsigned short PULSE_TIME = 10;


bool USBControler::SetRGBModeOn()
{
	sendpacket[3] = 0xff;
	sendpacket[4] = 0x00;
	sendpacket[5] = 0x00;
	sendpacket[8] = 0x00;
	sendpacket[9] = 0x00;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;

}

bool USBControler::SetRGBModeOff()
{
	sendpacket[3] = 0x00;
	sendpacket[4] = 0x00;
	sendpacket[5] = 0x00;
	sendpacket[8] = 0x00;
	sendpacket[9] = 0x00;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;

}

bool USBControler::SetUVModeOn()
{
	//sendpacket[2]=0xff;
	sendpacket[3] = 0x00;
	sendpacket[4] = 0xff;
	sendpacket[5] = 0x00;
	sendpacket[8] = 0x00;
	sendpacket[9] = 0x00;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;
}

bool USBControler::SetUVModeOff()
{
	//sendpacket[2]=0xff;
	sendpacket[3] = 0x00;
	sendpacket[4] = 0x00;
	sendpacket[5] = 0x00;
	sendpacket[8] = 0x00;
	sendpacket[9] = 0x00;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;
}

bool USBControler::SetLightTestOn()
{
	sendpacket[6] = 0xff;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;
}

bool USBControler::SetLightTestOff()
{
	sendpacket[6] = 0x00;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;
}

bool USBControler::SetPLModeOn()
{
	sendpacket[3] = 0x00;
	sendpacket[4] = 0x00;
	sendpacket[5] = 0xff;
	sendpacket[8] = 0x00;
	sendpacket[9] = 0x00;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;
}

bool USBControler::SetPLModeOff()
{
	sendpacket[3] = 0x00;
	sendpacket[4] = 0x00;
	sendpacket[5] = 0x00;
	sendpacket[8] = 0x00;
	sendpacket[9] = 0x00;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;
}

//bool USBControler::Set405UVModeOn()
//{
//	sendpacket[3] = 0x00;
//	sendpacket[4] = 0x00;
//	sendpacket[5] = 0x00;
//	sendpacket[9] = 0xff;
//	sendpacket[8] = 0x00;
//	if (m_bOpened)
//		WriteData(sendpacket, 65);
//	return true;
//}
//
//bool USBControler::Set405UVModeOff()
//{
//	sendpacket[3] = 0x00;
//	sendpacket[4] = 0x00;
//	sendpacket[5] = 0x00;
//	sendpacket[8] = 0x00;
//	sendpacket[9] = 0x00;
//	if (m_bOpened)
//		WriteData(sendpacket, 65);
//	return true;
//}

bool USBControler::SetNPLModeOn()
{
	sendpacket[3] = 0x00;
	sendpacket[4] = 0x00;
	sendpacket[5] = 0x00;
	sendpacket[8] = 0xff;
	sendpacket[9] = 0x00;
	if (m_bOpened)
		WriteData(sendpacket, 65);

	return true;
}

bool USBControler::SetNPLModeOff()
{
	sendpacket[3] = 0x00;
	sendpacket[4] = 0x00;
	sendpacket[5] = 0x00;
	sendpacket[8] = 0x00;
	sendpacket[9] = 0x00;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;
}

bool USBControler::SetAnimateOff()
{
	sendpacket[15] = 0xfe;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;
}

bool USBControler::SetAnimateMode(int nMode)
{
	if (nMode == 99)
		nMode = 0xFF;
	sendpacket[15] = nMode;
	if (m_bOpened)
		WriteData(sendpacket, 65);
	return true;
}

bool USBControler::ConnectCameraByTime(int nPulseTime)
{
	sendpacket[7] = 0xFF;
	PulseDataByPulseTime(nPulseTime, 7, sendpacket, 65);

	return true;
}

int USBControler::ReadData(void* buffer, int limit, int timeout)
{
	if (!m_bOpened || m_hFile == NULL)
		return(0);

	BOOL bReadStatus;
	DWORD dwBytesRead, ulBytesSucceed;
	dwBytesRead = (DWORD)limit;
	bReadStatus = ReadFile(m_hReadFile, buffer,
		dwBytesRead, &dwBytesRead, &m_OverlappedRead);

	switch (WaitForSingleObject(m_OverlappedRead.hEvent, timeout))
	{
	case WAIT_OBJECT_0:
		if (!GetOverlappedResult(m_hFile, &m_OverlappedRead, &ulBytesSucceed, 0))
			break;
		return ulBytesSucceed;
		break;

	case WAIT_TIMEOUT:
		CancelIo(m_hReadFile);
		break;
	default:
		break;
	}

	return 0;
}

int USBControler::WriteData(byte* buffer, int size)
{
	if (!m_bOpened || m_hFile == NULL)
		return(0);

	DWORD dwBytesWritten = 0;

	if (!WriteFile(m_hFile, buffer, size, &dwBytesWritten, &m_OverlappedWrite))
		return 0;
	return ((int)dwBytesWritten);
}

int USBControler::PulseData(int nIndex, byte* buffer, int size)
{
	if (!m_bOpened || m_hFile == NULL)
		return(0);

	DWORD dwBytesWritten = 0;
	if (!WriteFile(m_hFile, buffer, size, &dwBytesWritten, &m_OverlappedWrite))
		return 0;
	buffer[nIndex] = 0x00;
	Sleep(PULSE_TIME);
	if (!WriteFile(m_hFile, buffer, size, &dwBytesWritten, &m_OverlappedWrite))
		return 0;
	return ((int)dwBytesWritten);

}

int USBControler::PulseDataByPulseTime(int nPulseTime, int nIndex, byte* buffer, int size)
{
	if (!m_bOpened || m_hFile == NULL)
		return(0);

	DWORD dwBytesWritten = 0;
	if (!WriteFile(m_hFile, buffer, size, &dwBytesWritten, &m_OverlappedWrite))
		return 0;
	buffer[nIndex] = 0x00;
	Sleep(nPulseTime);
	if (!WriteFile(m_hFile, buffer, size, &dwBytesWritten, &m_OverlappedWrite))
		return 0;
	return ((int)dwBytesWritten);

}

int USBControler::WriteDataOn(int nBit, int nIndex, byte* buffer, int size)
{
	if (!m_bOpened || m_hFile == NULL)
		return(0);

	buffer[nIndex] = SetBitOn(nBit, buffer[nIndex]);
	DWORD dwBytesWritten = 0;
	if (!WriteFile(m_hFile, buffer, size, &dwBytesWritten, &m_OverlappedWrite))
		return 0;
	return ((int)dwBytesWritten);

}

int USBControler::WriteDataOff(int nBit, int nIndex, byte* buffer, int size)
{
	if (!m_bOpened || m_hFile == NULL)
		return(0);

	buffer[nIndex] = SetBitOff(nBit, buffer[nIndex]);
	DWORD dwBytesWritten = 0;
	if (!WriteFile(m_hFile, buffer, size, &dwBytesWritten, &m_OverlappedWrite))
		return 0;
	return ((int)dwBytesWritten);
}

byte USBControler::SetBitOn(int nBit, byte byIn)
{
	union bunion byBuff;
	byBuff.bData = byIn;
	switch (nBit)
	{
	case 0:
		byBuff.bit.b0 = ON;
		break;
	case 1:
		byBuff.bit.b1 = ON;
		break;
	case 2:
		byBuff.bit.b2 = ON;
		break;
	case 3:
		byBuff.bit.b3 = ON;
		break;
	case 4:
		byBuff.bit.b4 = ON;
		break;
	case 5:
		byBuff.bit.b5 = ON;
		break;
	case 6:
		byBuff.bit.b6 = ON;
		break;
	case 7:
		byBuff.bit.b7 = ON;
		break;
	default:
		assert(FALSE);
		break;
	}
	return byBuff.bData;
}

byte USBControler::SetBitOff(int nBit, byte byIn)
{
	union bunion byBuff;
	byBuff.bData = byIn;
	switch (nBit)
	{
	case 0:
		byBuff.bit.b0 = OFF;
		break;
	case 1:
		byBuff.bit.b1 = OFF;
		break;
	case 2:
		byBuff.bit.b2 = OFF;
		break;
	case 3:
		byBuff.bit.b3 = OFF;
		break;
	case 4:
		byBuff.bit.b4 = OFF;
		break;
	case 5:
		byBuff.bit.b5 = OFF;
		break;
	case 6:
		byBuff.bit.b6 = OFF;
		break;
	case 7:
		byBuff.bit.b7 = OFF;
		break;
	default:
		assert(FALSE);
		break;
	}
	return byBuff.bData;
}

bool USBControler::Open()
{
	if (m_bOpened)
		return m_bOpened;

	static GUID guidHID;
	BOOL bSuccess = FALSE;
	DWORD index;
	DWORD dwMemberIndex = 0;
	for (dwMemberIndex = 0; dwMemberIndex < 20; dwMemberIndex++)
	{
		index = dwMemberIndex;
		Close();
		HidD_GetHidGuid(&guidHID);
		m_hDevInfo = SetupDiGetClassDevs(&guidHID, NULL, 0,
			DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		std::wstring strPath;

		BOOL bDeviceDetected = FALSE;
		SP_DEVICE_INTERFACE_DATA strtInterfaceData;

		do
		{

			strtInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			bSuccess = SetupDiEnumDeviceInterfaces(m_hDevInfo, NULL,
				&guidHID, index++, &strtInterfaceData);
			if (bSuccess == FALSE)
			{
				SetupDiDestroyDeviceInfoList(m_hDevInfo);
				m_sError = L"未找到可用的USB设备";
				continue;
			}

			DWORD Length = 0, Required;
			SetupDiGetDeviceInterfaceDetail(m_hDevInfo,
				&strtInterfaceData, NULL, 0, &Length, NULL);
			PSP_DEVICE_INTERFACE_DETAIL_DATA pstrtDetailData;
			pstrtDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
			pstrtDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			bSuccess = SetupDiGetDeviceInterfaceDetail(m_hDevInfo, &strtInterfaceData,
				pstrtDetailData, Length, &Required, NULL);
			if (bSuccess == FALSE)
			{
				m_sError = L"查找设备路径时出错!\n";
				SetupDiDestroyDeviceInfoList(m_hDevInfo);
				continue;
			}

			strPath = pstrtDetailData->DevicePath;
			free(pstrtDetailData);

			m_hFile = CreateFile(strPath.c_str(), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

			if (m_hFile == INVALID_HANDLE_VALUE)
			{
				m_sError = L"无法打开通信端口\n";
				DeleteFile(strPath.c_str());
				//SetupDiDestroyDeviceInfoList(m_hDevInfo);
				bSuccess = FALSE;
				continue;
			}

			m_OverlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_OverlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			HIDD_ATTRIBUTES strtAttrib;

			if (!HidD_GetAttributes(m_hFile, &strtAttrib))
			{
				m_sError = L"查询设备状态时出错!\n";
				CloseHandle(m_hFile);
				if (m_OverlappedRead.hEvent != NULL)
					CloseHandle(m_OverlappedRead.hEvent);
				if (m_OverlappedWrite.hEvent != NULL)
					CloseHandle(m_OverlappedWrite.hEvent);
				SetupDiDestroyDeviceInfoList(m_hDevInfo);
				bSuccess = FALSE;
				continue;
			}
			UINT usVID = strtAttrib.VendorID;
			UINT usPID = strtAttrib.ProductID;
			UINT usVN = strtAttrib.VersionNumber;
			if (usVID != VID || usPID != PID)
			{
				CloseHandle(m_hFile);
				if (m_OverlappedRead.hEvent != NULL)
					CloseHandle(m_OverlappedRead.hEvent);
				if (m_OverlappedWrite.hEvent != NULL)
					CloseHandle(m_OverlappedWrite.hEvent);
				SetupDiDestroyDeviceInfoList(m_hDevInfo);
				m_sError = L"不是要找的USB设备";
				bSuccess = FALSE;
				continue;
			}
			else
			{
				bSuccess = TRUE;
				m_hReadFile = CreateFile(strPath.c_str(), GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					(LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
				break;
			}

		} while (index < 20 && bSuccess == FALSE);
		if (bSuccess == TRUE)break;
	}
	return m_bOpened = bSuccess == TRUE ? true : false;
}

bool USBControler::Close(void)
{
	if (m_bOpened)
	{
		CloseHandle(m_hFile);
		CloseHandle(m_hReadFile);
		if (m_OverlappedRead.hEvent != NULL)
			CloseHandle(m_OverlappedRead.hEvent);
		if (m_OverlappedWrite.hEvent != NULL)
			CloseHandle(m_OverlappedWrite.hEvent);

		SetupDiDestroyDeviceInfoList(m_hDevInfo);
		m_bOpened = false;

	}
	return true;
}

void USBControler::Init()
{
	memset(sendpacket, 0, 65);
	ReadData(sendpacket, 65, 1000);
	sendpacket[0] = 0x00;
	sendpacket[1] = 0x55;
	WriteData(sendpacket, 65);
}
