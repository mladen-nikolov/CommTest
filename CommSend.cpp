#include "Params.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int CommSend()
{
    DCB dcbSerialParams = { 0 };  // Initializing DCB structure
    COMMTIMEOUTS timeouts = { 0 };  //Initializing timeouts structure
    char SerialBuffer[1] = { 0 }; //Buffer to send and receive data
    DWORD BytesWritten = 0;          // No of bytes written to the port
    const std::wstring& portName = GetSenderPort(); //com port id
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

    printf_s("\nStart sending");
    {
        for (UINT32 i = 0; SEND_PACK_COUNT > i; ++i)
        {
            //Writing data to Serial Port
            const BOOL Status = ::WriteFile(hComm,// Handle to the Serialport
                &i,            // Data to be written to the port
                sizeof(i),   // No of bytes to write into the port
                &BytesWritten,  // No of bytes written to the port
                NULL);

            if (Status == FALSE)
            {
                printf_s("\nFail to Written");
                goto Exit;
            }

            ::Sleep(0);
        }
    }
    printf_s("\nSending finished\n");

Exit:
    if (hComm != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(hComm);//Closing the Serial Port
    }

    return 0;
}
