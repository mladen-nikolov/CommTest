#pragma once

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>


class ReceivingLogItem
{
public:
    double dt_us = {};

    bool isNotificated = {};

    bool isRxChar = {};
    bool isModemStatus = {};
    bool isLineStatus = {};

    DWORD dwBytesRequested = {};
    DWORD dwBytesReturned = {};
};

using ReceivingLog = std::vector<ReceivingLogItem>;


class ScopeFinished
{
    bool& _flag;
    std::mutex& _mtx;
    std::condition_variable& _cv;

public:
    ScopeFinished(
        std::mutex& mtx_,
        std::condition_variable& cv_,
        bool& flag_
    );

    ~ScopeFinished();
    operator bool() const;
};


extern std::atomic<int> charCounter;
extern std::atomic<bool> isToFinish;
extern std::mutex mtx;
extern std::condition_variable cv;
extern bool readyToReceive;
extern bool receivingFinished;

extern ReceivingLog receivingLog;


void SetReadyToReceive();

void PrintReceivingLog(
    const ReceivingLog& receivingLog,
    std::ostream& out
);

void PrintReceivingLog(
    const ReceivingLog& receivingLog
);
