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

#define INVALID_CLIENT_ID "无效的clientId或者Trade.DLL无效"

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
json TTS_TradeApi::logon(const char* ip, const short port,
                        const char* version, short yybId,
                        const char* accountNo, const char* tradeAccount,
                        const char* jyPassword, const char* txPassword) {
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, accountNo);
    json j = dll->logon(ip, port, version, yybId, accountNo, tradeAccount, jyPassword, txPassword);
    if (j[TTS_SUCCESS].get<bool>() == true) {
        uint32_t clientId = j[TTS_DATA]["client_id"].get<uint32_t>();
        uint32_t sessionId = genSessionId(dll->getSeq(), clientId);
        j[TTS_DATA]["client_id"] = sessionId;
        // 记录登陆行为
        TTS_ActiveClients::ins()->addNewEntry(
                    sessionId,
                    clientId,
                    ip,
                    port,
                    version,
                    yybId,
                    accountNo,
                    tradeAccount
                    );
    }
    return j;
}

/**
 * @brief TTS_TradeApi::logoff 登出
 * @param clientId cilent_id
 * @return success => true/false
 */
json TTS_TradeApi::logoff(int clientId) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->logoff(loginId);
    // 如果登出成功
    if (j["success"].get<bool>()) {
        TTS_ActiveClients::ins()->removeEntryBySessionId(clientId);
    }

    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

/**
 * @brief TTS_TradeApi::queryData 查询信息
 * @param clientId client_id
 * @param Category 信息类别
 * @return [{}, {}, {} ]
 */
json TTS_TradeApi::queryData(int clientId, int category) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->queryData(loginId, category);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}


json TTS_TradeApi::sendOrder(int clientId, int category ,int priceType, const char* gddm,  const char* zqdm , float price, int quantity) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->sendOrder(loginId, category, priceType, gddm, zqdm, price, quantity);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::cancelOrder(int clientId, const char *exchangeID, const char *hth) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->cancelOrder(loginId, exchangeID, hth);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::getQuote(int clientId, const char *zqdm) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->getQuote(loginId, zqdm);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::repay(int clientId, const char *amount) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->repay(loginId, amount);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::queryHistoryData(int clientId, int category, const char* beginDate, const char* endDate) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->queryHistoryData(loginId, category, beginDate, endDate);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::queryDatas(int clientId, int categories[], int count) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->queryDatas(loginId, categories, count);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::sendOrders(int clientId, int categories[], int priceTypes[], const char *gddms[], const char *zqdms[], float prices[], int quantities[], int count) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->sendOrders(loginId, categories, priceTypes, gddms, zqdms, prices, quantities, count);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::cancelOrders(int clientId, const char *exchangeIds[], const char *hths[], int count) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->cancelOrders(loginId, exchangeIds, hths, count);
    return (dll == nullptr) ? jsonError(INVALID_CLIENT_ID): j;
}

json TTS_TradeApi::getQuotes(int clientId, const char *zqdms[], int count) {
    int loginId = getLoginIdBySessionId(clientId);
    int seq = getSeqBySessionId(clientId);
    std::shared_ptr<TTS_Dll> dll = TTS_Dll::getInstance(_so, seq);
    json j = dll->getQuotes(loginId, zqdms, count);
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



