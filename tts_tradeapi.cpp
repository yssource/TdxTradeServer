#include "tts_tradeapi.h"
#include "Windows.h"
#include <QDebug>
#include "json.hpp"
#include "tts_common.h"
#include "tts_dll.h"
#include "tts_activeclients.h"
#include <QTextCodec>

using namespace std;
using json = nlohmann::json;

#define INVALID_CLIENT_ID "无效的clientID或者Trade.DLL无效"

TTS_TradeApi::TTS_TradeApi(const TTS_SettingObject& so):outputUtf8(true),_so(so)
{

}

TTS_TradeApi::~TTS_TradeApi()
{

}

void TTS_TradeApi::setOutputUtf8(bool utf8) {
    outputUtf8 = utf8;
    auto dlls = TTS_Dll::allDlls();
    for (auto& it = dlls.begin(); it != dlls.end(); it++) {
        std::shared_ptr<TTS_Dll> dll = it->second;
        dll->setOutputUtf8(utf8);
    }
}

/**
 * @brief TTS_TradeApi::logon 登陆到服务器
 * @param IP 服务器ip地址
 * @param Port 端口号
 * @param Version 客户端版本
 * @param YybID 营业部id
 * @param AccountNo 帐号
 * @param TradeAccount 交易帐号
 * @param JyPassword 交易密码
 * @param TxPassword 通讯密码
 * @return data -> { "client_id" : xxx }
 *
 */
json TTS_TradeApi::logon(const char* IP, const short Port,
                        const char* Version, short YybID,
                        const char* AccountNo, const char* TradeAccount,
                        const char* JyPassword, const char* TxPassword) {
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, AccountNo);
    json j = dll->logon(IP, Port, Version, YybID, AccountNo, TradeAccount, JyPassword, TxPassword);
    if (j[TTS_SUCCESS].get<bool>() == true) {
        uint32_t clientId = j[TTS_DATA]["client_id"].get<uint32_t>();
        uint32_t sessionId = genSessionId(dll->getSeq(), clientId);
        j[TTS_DATA]["client_id"] = sessionId;
        // 记录登陆行为
        TTS_ActiveClients::ins()->addNewEntry(
                    sessionId,
                    clientId,
                    IP,
                    Port,
                    Version,
                    YybID,
                    AccountNo,
                    TradeAccount
                    );
    }
    return j;
}

/**
 * @brief TTS_TradeApi::logoff 登出
 * @param ClientID cilent_id
 * @return success => true/false
 */
json TTS_TradeApi::logoff(int ClientID) {
    int loginId = getLoginIdBySessionId(ClientID);
    int seq = getSeqBySessionId(ClientID);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->logoff(loginId);
    // 如果登出成功
    if (j["success"].get<bool>()) {
        TTS_ActiveClients::ins()->removeEntryBySessionId(ClientID);
    }

    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

/**
 * @brief TTS_TradeApi::queryData 查询信息
 * @param ClientID client_id
 * @param Category 信息类别
 * @return [{}, {}, {} ]
 */
json TTS_TradeApi::queryData(int ClientID, int Category) {
    int loginId = getLoginIdBySessionId(ClientID);
    int seq = getSeqBySessionId(ClientID);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->queryData(loginId, Category);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}


json TTS_TradeApi::sendOrder(int ClientID, int Category ,int PriceType, const char* Gddm,  const char* Zqdm , float Price, int Quantity) {
    int loginId = getLoginIdBySessionId(ClientID);
    int seq = getSeqBySessionId(ClientID);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->sendOrder(loginId, Category, PriceType, Gddm, Zqdm, Price, Quantity);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::cancelOrder(int ClientID, const char *ExchangeID, const char *hth) {
    int loginId = getLoginIdBySessionId(ClientID);
    int seq = getSeqBySessionId(ClientID);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->cancelOrder(loginId, ExchangeID, hth);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::getQuote(int ClientID, const char *Zqdm) {
    int loginId = getLoginIdBySessionId(ClientID);
    int seq = getSeqBySessionId(ClientID);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->getQuote(loginId, Zqdm);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::repay(int ClientID, const char *Amount) {
    int loginId = getLoginIdBySessionId(ClientID);
    int seq = getSeqBySessionId(ClientID);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->repay(loginId, Amount);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::queryHistoryData(int ClientID, int Category, const char* BeginDate, const char* EndDate) {
    int loginId = getLoginIdBySessionId(ClientID);
    int seq = getSeqBySessionId(ClientID);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->queryHistoryData(loginId, Category, BeginDate, EndDate);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::queryDatas(int clientId, int categories[], int count) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->queryDatas(loginId, categories, count);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}


json TTS_TradeApi::jsonError(QString str) {
    string value;
    if (outputUtf8) {
        value = str.toUtf8();
    } else {
        value = str.toLocal8Bit();
    }
    json j;
    j[TTS_SUCCESS] = false;
    j[TTS_ERROR] = value;
    return j;
}



