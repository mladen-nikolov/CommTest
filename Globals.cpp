#include "Globals.h"

#include <iostream>
#include <iomanip>
#include <sstream>


std::atomic<int> charCounter;
std::atomic<bool> isToFinish;
std::mutex mtx;
std::condition_variable cv;
bool readyToReceive;
bool receivingFinished;
ReceivingLog receivingLog;


ScopeFinished::ScopeFinished(
    std::mutex& mtx_,
    std::condition_variable& cv_,
    bool& flag_
)
    : _mtx(mtx_)
    , _cv(cv_)
    , _flag(flag_)
{

}


ScopeFinished::~ScopeFinished()
{
    _flag = true;
}


ScopeFinished::operator bool() const
{
    return _flag;
}


void SetReadyToReceive()
{
    readyToReceive = true;
    cv.notify_one();
}


void PrintReceivingLog(
    const ReceivingLog& receivingLog,
    std::ostream& out
)
{
    constexpr const char* pColumnSeparator = "; ";

    {
        std::stringstream ss;

        ss << "dt_us" << pColumnSeparator;
              
        ss << "isNotificated" << pColumnSeparator;
              
        ss << "isRxChar" << pColumnSeparator;
        ss << "isModemStatus" << pColumnSeparator;
        ss << "isLineStatus" << pColumnSeparator;
              
        ss << "dwBytesRequested" << pColumnSeparator;
        ss << "dwBytesReturned" << pColumnSeparator;

        ss << '\n';

        out << ss.str();
    }

    for(const ReceivingLogItem& logItem : receivingLog)
    {
        std::stringstream ss;

        ss << std::setw(8) << std::setprecision(1) << std::fixed;
        ss << logItem.dt_us << pColumnSeparator;

        ss << logItem.isNotificated << pColumnSeparator;

        ss << logItem.isRxChar << pColumnSeparator;
        ss << logItem.isModemStatus << pColumnSeparator;
        ss << logItem.isLineStatus << pColumnSeparator;

        ss << std::setw(4);
        ss << logItem.dwBytesRequested << pColumnSeparator;

        ss << std::setw(4);
        ss << logItem.dwBytesReturned << pColumnSeparator;

        ss << '\n';

        out << ss.str();
    }
}


void PrintReceivingLog(
    const ReceivingLog& receivingLog
)
{
    PrintReceivingLog(receivingLog, std::cout);
}
