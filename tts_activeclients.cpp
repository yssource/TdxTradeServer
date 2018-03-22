#include "tts_activeclients.h"
#include "json.hpp"
#include <vector>
#include <mutex>
#include <time.h>

TTS_ActiveClients* TTS_ActiveClients::_instance;


TTS_ActiveClients* TTS_ActiveClients::ins()
{
    static std::once_flag oc;
    std::call_once(oc, [&]() { _instance = new TTS_ActiveClients(); });
    return _instance;
}

void TTS_ActiveClients::addNewEntry(uint32_t sessionId, uint32_t clientId, const char *ip, const short port, const char *version, short yybId, const char *accountNo, const char *tradeAccount)
{
    time_t ts;
    time(&ts);
    activeClients.push_back(std::make_shared<TTS_ActiveClientEntry>(
                                sessionId, clientId, ip, port, version, yybId, accountNo, tradeAccount, ts
                                ));
}

void TTS_ActiveClients::addNewEntry(const std::shared_ptr<TTS_ActiveClientEntry>& entry)
{
    activeClients.push_back(entry);
}

bool TTS_ActiveClients::removeEntryBySessionId(uint32_t sessionId)
{
    auto pos = std::remove_if(activeClients.begin(),
                          activeClients.end(),
                          [sessionId](const std::shared_ptr<TTS_ActiveClientEntry>& entry){
                                    return entry->sessionId == sessionId;
                          });
    if (pos != activeClients.end()) {
        activeClients.erase(pos, activeClients.end());
        return true;
    } else {
        return false;
    }
}

bool TTS_ActiveClients::sessionIdExists(uint32_t sessionId) {
    return find_if(activeClients.begin(),
                   activeClients.end(),
                   [sessionId](const std::shared_ptr<TTS_ActiveClientEntry>& entry){
        return entry->sessionId == sessionId;
    }) != activeClients.end();
}

json TTS_ActiveClients::toJson()
{
    json j;
    j["success"] = true;
    j["data"] = json::array();

    for (const std::shared_ptr<TTS_ActiveClientEntry>& entry: activeClients) {
        j["data"].push_back({
                {"client_id", entry->sessionId},
                {"inner_client_id", entry->clientId},
                {"ip", entry->ip},
                {"port", entry->port},
                {"version", entry->version},
                {"yyb_no", entry->yybId},
                {"account_no", entry->accountNo},
                {"trade_account", entry->tradeAccount},
                {"start_ts", entry->startTs}
                    });
    }
    return j;
}

TTS_ActiveClients::TTS_ActiveClients()
{

}
