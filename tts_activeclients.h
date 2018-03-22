#ifndef TTS_ACTIVECLIENTS_H
#define TTS_ACTIVECLIENTS_H

#include <vector>
#include <mutex>
#include "json.hpp"

using json = nlohmann::json;

typedef struct _TTS_ActiveClientEntry {
    uint32_t    sessionId;  // TdxTradeServer Issue的ID
    uint32_t    clientId;   // trade.dll 返回的id,不同的券商可能会重复
    std::string ip;
    short       port;
    std::string version;
    short       yybId;
    std::string accountNo;
    std::string tradeAccount;
    time_t startTs;

    _TTS_ActiveClientEntry(uint32_t sessionId, uint32_t clientId, const char* ip, const short port, const char* version, short yybId, const char* accountNo, const char* tradeAccount, time_t startTs) {
        this->sessionId = sessionId;
        this->clientId = clientId;
        this->ip = ip;
        this->port = port;
        this->version = version;
        this->yybId = yybId;
        this->accountNo = accountNo;
        this->tradeAccount = tradeAccount;
        this->startTs = startTs;
    }
} TTS_ActiveClientEntry;


class TTS_ActiveClients
{ 
public:
    static TTS_ActiveClients* ins();
    void addNewEntry(uint32_t sessionId, uint32_t clientId, const char* ip, const short port, const char* version, short yybId, const char* accountNo, const char* tradeAccount);
    void addNewEntry(const std::shared_ptr<TTS_ActiveClientEntry>& entry);
    bool removeEntryBySessionId(uint32_t sessionId);
    bool sessionIdExists(uint32_t sessionId);
    json toJson();
private:
    static TTS_ActiveClients* _instance;
    TTS_ActiveClients();
    std::vector<std::shared_ptr<TTS_ActiveClientEntry>> activeClients;
};

#endif // TTS_ACTIVECLIENTS_H
