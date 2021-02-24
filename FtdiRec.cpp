#include "Params.h"
#include "Globals.h"

#include "ftd2xx.h"

#include <iostream>
#include <sstream>
#include <array>
#include <valarray>
#include <vector>


static int Enumerating(
	int& receiveDeviceId
)
{
	FT_STATUS ftStatus = FT_OK;

	int deviceCount = 0;
	ftStatus = ::FT_ListDevices(&deviceCount, NULL, FT_LIST_NUMBER_ONLY);
	if (FT_OK != ftStatus)
	{
		std::stringstream ss;
		ss << "FT_ListDevices() ftStatus = " << ftStatus << std::endl;
		std::cerr << ss.str();
		return -1;
	}

	std::vector<std::array<char, 256>> Buf(deviceCount);
	std::valarray<char*> BufPtrs(deviceCount + 1);
	std::valarray<LONG> ComPortNumbers(deviceCount);
	for (int deviceId = 0; deviceCount > deviceId; ++deviceId)
	{
		BufPtrs[deviceId] = Buf[deviceId].data();

		FT_HANDLE ftHandle = nullptr;
		ftStatus = ::FT_Open(deviceId, &ftHandle);
		if (FT_OK != ftStatus)
		{
			std::stringstream ss;
			ss << "FT_Open() ftStatus = " << ftStatus << std::endl;
			std::cerr << ss.str();
			return -1;
		}

		ftStatus = ::FT_GetComPortNumber(ftHandle, &ComPortNumbers[deviceId]);
		if (FT_OK != ftStatus)
		{
			std::stringstream ss;
			ss << "FT_GetComPortNumber() ftStatus = " << ftStatus << std::endl;
			std::cerr << ss.str();
			return -1;
		}

		if (ftHandle != nullptr)
		{
			::FT_Close(ftHandle);
			ftHandle = nullptr;
		}
	}

	ftStatus = FT_ListDevices(&BufPtrs[0], &deviceCount, FT_LIST_ALL | FT_OPEN_MASK);
	if (ftStatus != FT_OK)
	{
		std::stringstream ss;
		ss << "FT_ListDevices() ftStatus = " << ftStatus << std::endl;
		std::cerr << ss.str();
		return -1;
	}

	const std::wstring& portName = GetReceiverPort(); //com port id

	receiveDeviceId = -1;
	for (int deviceId = 0; deviceCount > deviceId; ++deviceId)
	{
		std::array<wchar_t, 256> comPort;
		wsprintf(comPort.data(), L"COM%d", ComPortNumbers[deviceId]);

		if (0 == ::wcscmp(comPort.data(), portName.c_str()))
		{
			receiveDeviceId = deviceId;
		}

		std::cout << deviceId << ": ";
		std::wcout << std::wstring(comPort.data());
		std::cout << "; " << BufPtrs[deviceId] << std::endl;
	}

	if (0 > receiveDeviceId)
	{
		std::cout << "Can't find ";
		std::wcout << portName << std::endl;

		return -1;
	}

	return 0;
}


static int Initialize(
	const FT_HANDLE ftHandle,
	const HANDLE hEventNotification
)
{
	FT_STATUS ftStatus = FT_OK;

	ftStatus = ::FT_SetBaudRate(ftHandle, COMM_BaudRate);
	if (FT_OK != ftStatus)
	{
		std::stringstream ss;
		ss << "FT_SetBaudRate() ftStatus = " << ftStatus << std::endl;
		std::cerr << ss.str();
		return -1;
	}

	ftStatus = ::FT_SetDataCharacteristics(ftHandle, COMM_ByteSize, COMM_StopBits, COMM_Parity);
	if (FT_OK != ftStatus)
	{
		std::stringstream ss;
		ss << "FT_SetDataCharacteristics() ftStatus = " << ftStatus << std::endl;
		std::cerr << ss.str();
		return -1;
	}

	ftStatus = ::FT_SetLatencyTimer(ftHandle, 0);
	if (FT_OK != ftStatus)
	{
		std::stringstream ss;
		ss << "FT_SetLatencyTimer() ftStatus = " << ftStatus << std::endl;
		std::cerr << ss.str();
		return -1;
	}

	const std::string notificationMaskStr = GetParam(pParamKey_notification_mask);
	const DWORD notificationMask = std::stoi(notificationMaskStr);

	ftStatus = ::FT_SetEventNotification(ftHandle, notificationMask, hEventNotification);
	if (FT_OK != ftStatus)
	{
		std::stringstream ss;
		ss << "FT_SetEventNotification() ftStatus = " << ftStatus << std::endl;
		std::cerr << ss.str();
		return -1;
	}

	return 0;
}


static int Run(
	const FT_HANDLE ftHandle,
	const HANDLE hEventNotification
	)
{
	FT_STATUS ftStatus = FT_OK;

	receivingLog.reserve(SEND_BYTES_COUNT * 4);
	receivingLog.resize(0);

	std::array<char, SEND_BYTES_COUNT> receivingBuffer;

	SetReadyToReceive();
	{
		std::stringstream ss;
		ss << "Start receiving loop." << std::endl;
		std::cout << ss.str();
	}

	{
		int burstCounter = 0;

		using Clock = std::chrono::high_resolution_clock;
		auto t1 = Clock::now();

		for (; false == isToFinish; ++burstCounter)
		{
			while (false == isToFinish)
			{
				const DWORD dwWaitCode = ::WaitForSingleObject(hEventNotification, 0);
				const bool isNotificated = (WAIT_OBJECT_0 == dwWaitCode);

				DWORD dwRxBytes = {};
				DWORD dwTxBytes = {};
				DWORD dwEventDWord = {};
				ftStatus = ::FT_GetStatus(ftHandle, &dwRxBytes, &dwTxBytes, &dwEventDWord);
				if (FT_OK != ftStatus)
				{
					std::stringstream ss;
					ss << "FT_GetStatus() ftStatus = " << ftStatus << std::endl;
					std::cerr << ss.str();
					return -1;
				}

				const bool isRxChar = dwEventDWord & FT_EVENT_RXCHAR;
				const bool isModemStatus = dwEventDWord & FT_EVENT_MODEM_STATUS;
				const bool isLineStatus = dwEventDWord & FT_EVENT_LINE_STATUS;

				DWORD dwBytesReturned = 0;

				if (0 < dwRxBytes)
				{
					ftStatus = ::FT_Read(ftHandle, receivingBuffer.data(), dwRxBytes, &dwBytesReturned);
					if (FT_OK != ftStatus)
					{
						std::stringstream ss;
						ss << "FT_Read() ftStatus = " << ftStatus << std::endl;
						std::cerr << ss.str();
						return -1;
					}

					charCounter += dwBytesReturned;
				}

				{
					const auto t2 = Clock::now();
					const auto dt_us = t2 - t1;

					{
						receivingLog.resize(burstCounter + 1);
						ReceivingLogItem& logItem = receivingLog[burstCounter];

						logItem.dt_us = dt_us.count() * 1e-3;

						logItem.dwBytesRequested = dwRxBytes;
						logItem.dwBytesReturned = dwBytesReturned;

						logItem.isNotificated = isNotificated;

						logItem.isRxChar = isRxChar;
						logItem.isModemStatus = isModemStatus;
						logItem.isLineStatus = isLineStatus;
					}

					t1 = t2;
				}

				if ((0 < dwRxBytes) || isNotificated || (0 != dwEventDWord))
				{
					break;
				}
			}
		}

		{
			std::stringstream ss;
			ss << "burstCounter = " << burstCounter << std::endl;
			std::cout << ss.str();
		}
	}

	{
		std::stringstream ss;
		ss << "Finished receiving loop." << std::endl;
		std::cout << ss.str();
	}

	return 0;
}


int FtdiRec()
{
	ScopeFinished scopeFinished(mtx, cv, receivingFinished);

	int errorCode = 0;
	FT_HANDLE ftHandle = nullptr;
	HANDLE hEventNotification = 0;

	{
		int receiveDeviceId;
		errorCode = Enumerating(receiveDeviceId);
		if (0 != errorCode)
		{
			goto onError;
		}

		const FT_STATUS ftStatus = ::FT_Open(receiveDeviceId, &ftHandle);
		if (FT_OK != ftStatus)
		{
			std::stringstream ss;
			ss << "FT_Open() ftStatus = " << ftStatus << std::endl;
			std::cerr << ss.str();
			errorCode = -1;
			goto onError;
		}
	}

	hEventNotification = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (0 == hEventNotification)
	{
		std::stringstream ss;
		ss << "CreateEvent() hEventNotification = " << hEventNotification << std::endl;
		std::cerr << ss.str();
		errorCode = -1;
		goto onError;
	}

	errorCode = Initialize(ftHandle, hEventNotification);
	if (0 != errorCode)
	{
		goto onError;
	}

	errorCode = Run(ftHandle, hEventNotification);
	if (0 != errorCode)
	{
		goto onError;
	}

onError:
	if (0 != hEventNotification)
	{
		::CloseHandle(hEventNotification);
	}

	if (ftHandle != nullptr)
	{
		::FT_Close(ftHandle);
	}

	return errorCode;
}
