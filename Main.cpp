#include "Globals.h"
#include "Params.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <thread>
#include <iostream>
#include <fstream>


int FtdiRec();
int CommRec();
int CommSend();


int main(
    const int paramCount,
    const char* const* const ppParams
)
{
    int errorCode = ParseParams(paramCount, ppParams);
    if (0 != errorCode)
    {
        return errorCode;
    }

    int(* pRec)();
    {
        const std::string api = GetParamLower(pParamKey_api);
        if ("d2xx" == api)
        {
            pRec = FtdiRec;
        }
        else if ("vcp" == api)
        {
            pRec = CommRec;
        }
        else
        {
            std::cout << "\nUnknown API. Enter d2xx or vcp" << std::endl;
            return -1;
        }
    }

    std::thread threadRec(pRec);
    {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [] {return readyToReceive; });
    }

    CommSend();

    ::Sleep(100);
    isToFinish = true;

    if (threadRec.joinable())
    {
        threadRec.join();
    }

    std::cout << "readCharCounter = " << charCounter << std::endl;

    {
        bool logSafe = false;
        const std::string receivingLogFileName = GetParam(pParamKey_receiver_log);
        if (0 < receivingLogFileName.length())
        {
            try
            {
                std::ofstream receivingLogFile(receivingLogFileName);
                PrintReceivingLog(receivingLog, receivingLogFile);
                logSafe = true;
            }
            catch (...)
            {
            }
        }

        if (false == logSafe)
        {
            PrintReceivingLog(receivingLog);
        }
    }

    return 0;
}
