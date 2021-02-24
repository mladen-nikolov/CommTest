#include "Params.h"
#include "Globals.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <chrono>


int CommRec()
{
    ScopeFinished scopeFinished(mtx, cv, receivingFinished);

    DCB dcbSerialParams = { 0 };  // Initializing DCB structure
    COMMTIMEOUTS timeouts = { 0 };  //Initializing timeouts structure
    DWORD dwEventMask;     // Event mask to trigger
    char  ReadData[1024 * 4];        //temperory Character
    DWORD NoBytesRead;     // Bytes read by ReadFile()
    const std::wstring& portName = GetReceiverPort(); //com port id
    wchar_t PortNo[20] = { 0 }; //contain friendly name

    swprintf_s(PortNo, 20, L"\\\\.\\%s", portName.c_str());

    //Open the serial com port
    const HANDLE hComm = ::CreateFile(PortNo, //friendly name
        GENERIC_READ | GENERIC_WRITE,      // Read/Write Access
        0,                                 // No Sharing, ports cant be shared
        NULL,                              // No Security
        OPEN_EXISTING,                     // Open existing port only
        0,                                 // Non Overlapped I/O
        NULL);                             // Null for Comm Devices
    if (hComm == INVALID_HANDLE_VALUE)
    {
        printf_s("\n Port can't be opened\n\n");
        goto Exit;
    }
    //Setting the Parameters for the SerialPort
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    
    {
        const BOOL Status = ::GetCommState(hComm, &dcbSerialParams); //retreives  the current settings
        if (Status == FALSE)
        {
            printf_s("\nError to Get the Com state\n\n");
            goto Exit;
        }
    }
    dcbSerialParams.BaudRate = COMM_BaudRate;
    dcbSerialParams.ByteSize = COMM_ByteSize;
    dcbSerialParams.StopBits = COMM_StopBits;
    dcbSerialParams.Parity = COMM_Parity;

    {
        const BOOL Status = ::SetCommState(hComm, &dcbSerialParams);
        if (Status == FALSE)
        {
            printf_s("\nError to Setting DCB Structure\n\n");
            goto Exit;
        }
    }

    //Setting Timeouts
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.ReadTotalTimeoutMultiplier = 0;

    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (::SetCommTimeouts(hComm, &timeouts) == FALSE)
    {
        printf_s("\nError to Setting Time outs");
        goto Exit;
    }

    //Setting Receive Mask
    {
        const BOOL Status = ::SetCommMask(hComm, EV_RXCHAR);
        if (Status == FALSE)
        {
            printf_s("\nError to in Setting CommMask\n\n");
            goto Exit;
        }
    }

    SetReadyToReceive();
    printf_s("\nStart receiving loop.");

    {
        using Clock = std::chrono::high_resolution_clock;
        auto t1 = Clock::now();

        for (int burstCounter = 0; false == isToFinish; ++burstCounter)
        {
            while (false == isToFinish)
            {
                if (0)
                {
                    dwEventMask = 0;
                    {
                        const BOOL Status = ::WaitCommEvent(hComm, &dwEventMask, NULL); //Wait for the character to be received
                        if (Status == FALSE)
                        {
                            printf_s("\nError! in Setting WaitCommEvent()\n\n");
                            goto Exit;
                        }
                    }

                    if (0 == (dwEventMask & EV_RXCHAR))
                    {
                        // internal error
                        continue;
                    }
                }

                {
                    constexpr DWORD dwBytesRequested = sizeof(ReadData);
                    const BOOL Status = ::ReadFile(hComm, ReadData, dwBytesRequested, &NoBytesRead, NULL);
                    if (Status == FALSE)
                    {
                        printf_s("\nError! in ReadFile()\n\n");
                        goto Exit;
                    }

                    if (0 == NoBytesRead)
                    {
                        continue;
                    }

                    auto t2 = Clock::now();
                    auto dt = t2 - t1;

                    receivingLog.resize(burstCounter + 1);
                    ReceivingLogItem& logItem = receivingLog[burstCounter];
                    logItem.dt_us = dt.count() * 1e-3;
                    logItem.dwBytesRequested = dwBytesRequested;
                    logItem.dwBytesReturned = NoBytesRead;

                    t1 = t2;
                    charCounter += NoBytesRead;
                }

                break;
            }
        }
    }

Exit:
    if (hComm != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(hComm);//Closing the Serial Port
    }

    printf_s("\nFinished receiving loop.\n");
    return 0;
}
