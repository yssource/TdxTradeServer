#ifndef TTS_TRADEAPI_H
#define TTS_TRADEAPI_H
#include <QtCore>
#include <Windows.h>
#include "json.hpp"
#include "tts_common.h"
#include "tts_dll.h"

using json = nlohmann::json;

class TTS_TradeApi
{

private:
    bool outputUtf8;
    TTS_SettingObject _so;
    inline uint32_t genSessionId(const uint32_t& seq,  const uint32_t& loginId) const { return (seq << 16) + loginId; }
    inline uint32_t getSeqBySessionId(const uint32_t& sessionId) const { return sessionId >> 16; }
    inline uint32_t getLoginIdBySessionId(const uint32_t& sessionId) { return sessionId & 0xffff; }

public:
    TTS_TradeApi(const TTS_SettingObject& so);
    ~TTS_TradeApi();

    void setOutputUtf8(bool utf8);

    json logon(const char* IP, const short Port,
              const char* Version, short YybID,
              const char* AccountNo, const char* TradeAccount,
              const char* JyPassword, const char* TxPassword);

    json logoff(int ClientID);
    json queryData(int ClientID, int Category);
    json sendOrder(int ClientID, int Category ,int PriceType, const char* Gddm, const char* Zqdm , float Price, int Quantity);
    json cancelOrder(int ClientID, const char* ExchangeID, const char* hth);
    json getQuote(int ClientID, const char* Zqdm);
    json queryHistoryData(int ClientID, int Category, const char* BeginDate, const char* EndDate);
    json repay(int ClientID, const char* Amount);
    json jsonError(QString str);
};

#endif // TTS_TRADEAPI_H
