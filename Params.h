#pragma once

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#include <string>

#define COMM_BaudRate CBR_115200
#define COMM_ByteSize 8
#define COMM_StopBits ONESTOPBIT
#define COMM_Parity EVENPARITY

#define SEND_PACK_COUNT 1000
#define SEND_PACK_SIZE 4
#define SEND_BYTES_COUNT (SEND_PACK_COUNT * SEND_PACK_SIZE) 


constexpr const char* const pParamKey_receiver_log = "receiver_log";
constexpr const char* const pParamKey_sender_port = "sender_port";
constexpr const char* const pParamKey_receiver_port = "receiver_port";
constexpr const char* const pParamKey_api = "api";
constexpr const char* const pParamKey_notification_mask = "notification_mask";


int ParseParams(
    const int paramCount,
    const char* const* const ppParams
);


std::string GetParam(
    const std::string& paramKey
);


std::string GetParamLower(
    const std::string& paramKey
);


std::wstring GetSenderPort();
std::wstring GetReceiverPort();
