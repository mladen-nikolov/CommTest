#include "Params.h"

#include <cassert>

#include <iostream>
#include <unordered_map>
#include <string>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <cctype>

namespace
{
    using ParamsMap = std::unordered_map<std::string, std::string>;
    using KeyValue = std::pair<std::string, std::string>;

    const std::string delimiter("=");

    ParamsMap paramsMap;

    void SetParamsToDefault()
    {
        paramsMap.insert(KeyValue(pParamKey_receiver_log, "receiver_log.txt"));
        paramsMap.insert(KeyValue(pParamKey_sender_port, "COM3"));
        paramsMap.insert(KeyValue(pParamKey_receiver_port, "COM6"));
        paramsMap.insert(KeyValue(pParamKey_api, "d2xx")); // vcp
        paramsMap.insert(KeyValue(pParamKey_notification_mask, "7"));
    }


    void PrintParams()
    {
        for (const KeyValue& keyValue : paramsMap)
        {
            const std::string& key = keyValue.first;
            const std::string& value = keyValue.second;

            std::cout << key << delimiter << value << std::endl;
        }
    }
}


int ParseParams(
    const int paramCount,
    const char* const* const ppParams
)
{
    SetParamsToDefault();

    for (int ind = 1; paramCount > ind; ++ind)
    {
        std::string param = ppParams[ind];

        const size_t pos = param.find(delimiter);

        if ( pos != std::string::npos)
        {
            std::string key = param.substr(0, pos);

            std::transform(key.begin(), key.end(), key.begin(),
                [](unsigned char c) 
                { 
                    return std::tolower(c); 
                }
            );

            const auto iter = paramsMap.find(key);
            if (paramsMap.end() == iter)
            {
                std::cout << "\nUnknown key: " << key << std::endl;
            }
            else
            {
                param.erase(0, pos + delimiter.length());
                paramsMap[key] = param;
            }
        }
    }

    PrintParams();
    return 0;
}


std::string GetParam(
    const std::string& paramKey
)
{
    const auto iter = paramsMap.find(paramKey);
    if (paramsMap.end() == iter)
    {
        assert(false);
        return std::string();
    }

    const std::string& value = iter->second;
    return value;
}


std::string GetParamLower(
    const std::string& paramKey
)
{
    std::string param = GetParam(paramKey);

    std::transform(param.begin(), param.end(), param.begin(),
        [](unsigned char c)
        {
            return std::tolower(c);
        }
    );

    return param;
}


std::wstring GetSenderPort()
{
    const std::string param = GetParam(pParamKey_sender_port);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const std::wstring wideParam = converter.from_bytes(param);
    return wideParam;
}


std::wstring GetReceiverPort()
{
    const std::string param = GetParam(pParamKey_receiver_port);
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    const std::wstring wideParam = converter.from_bytes(param);
    return wideParam;
}
