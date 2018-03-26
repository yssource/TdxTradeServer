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

    json logon(const char* ip, const short port,
              const char* version, short yybId,
              const char* accountNo, const char* tradeAccount,
              const char* jyPassword, const char* txPassword);

    json logoff(int clientId);
    json queryData(int clientId, int category);
    json sendOrder(int clientId, int category ,int priceType, const char* gddm, const char* zqdm , float price, int quantity);
    json cancelOrder(int clientId, const char* exchangeID, const char* hth);
    json getQuote(int clientId, const char* zqdm);
    json queryHistoryData(int clientId, int category, const char* beginDate, const char* endDate);
    json repay(int clientId, const char* amount);
    json queryDatas(int clientId, int categories[], int count);
    json sendOrders(int clientId, int categories[], int priceTypes[], const char* gddms[], const char* zqdms[], float prices[], int quantities[], int count);
    json cancelOrders(int clientId, const char* exchangeIds[], const char* hths[], int count);
    json getQuotes(int clientId, const char*  zqdms[], int count);
    json jsonError(QString str);
};

#endif // TTS_TRADEAPI_H
