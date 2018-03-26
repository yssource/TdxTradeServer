#ifndef TTS_DLL_H
#define TTS_DLL_H
#include <QtCore>
#include <Windows.h>
#include "tts_common.h"
#include "tts_setting.h"
#include <map>
#include <memory>
#include "json.hpp"
#include "tts_common.h"
#include <QTextCodec>

using json = nlohmann::json;

typedef void (__stdcall *LPFN_OPENTDX)();
typedef void (__stdcall *LPFN_CLOSETDX)();

typedef int (__stdcall *LPFN_LOGON)(const char* ip, const short port, const char* version, short yybId,  const char* accountNo, const char* tradeAccount, const char* jyPassword, const char* txPassword, char* errInfo);
typedef void(__stdcall *LPFN_LOGOFF)(int clientId);
typedef void(__stdcall *LPFN_QUERYDATA)(int clientId, int category, char* result, char* errInfo);
typedef void(__stdcall *LPFN_SENDORDER)(int clientId, int category ,int priceType,  const char* gddm,  const char* zqdm , float price, int quantity,  char* result, char* errInfo);
typedef void(__stdcall *LPFN_CANCELORDER)(int clientId, const char* exchangeId, const char* hth, char* result, char* errInfo);
typedef void(__stdcall *LPFN_GETQUOTE)(int clientId, const char* zqdm, char* result, char* errInfo);
typedef void(__stdcall *LPFN_REPAY)(int clientId, const char* amount, char* result, char* errInfo);
typedef void(__stdcall *LPFN_QUERYHISTORYDATA)(int clientId, int category, const char* beginDate, const char* endDate, char* result, char* errInfo); //QueryHistoryData
typedef void(__stdcall *LPFN_SENDORDERS)(int clientId, int categories[], int priceTypes[], const char* gddms[], const char* zqdms[], float prices[], int quantities[], int count, char** results, char** errInfos); // SendOrders
typedef void(__stdcall *LPFN_CANCELORDERS)(int clientId, const char* exchangeIds[], const char* hths[], int count, char** results, char** errInfos); // SendOrders
typedef void(__stdcall *LPFN_QUERYDATAS)(int clientId, int categories[], int count, char** results, char** errInfos);
typedef void(__stdcall *LPFN_GETQUOTES)(int clientId, const char*  zqdms[], int count, char** results, char** errInfos);


#define P_LOGON         "logon"
#define P_LOGOFF        "logoff"
#define P_QUERYDATA     "query_data"
#define P_SENDORDER     "send_order"
#define P_CANCELORDER   "cancel_order"
#define P_GETQUOTE      "get_quote"
#define P_REPAY         "repay"
#define P_QUERYHISTORYDATA "query_history_data"
#define P_GETACTIVECLIENTS "get_active_clients"
#define P_SENDORDERS    "send_orders"
#define P_QUERYDATAS    "query_datas"
#define P_CANCELORDERS  "cancel_orders"
#define P_GETQUOTES      "get_quotes"

class TTS_Dll
{
private:
    HINSTANCE hDLL;
    TTS_Dll(const TTS_SettingObject& so, const std::string& accountNo);
    bool initSuccess;
    // store error and result;
    char* errout;
    char* result;

    /// api far call
    LPFN_OPENTDX lpOpenTdx;
    LPFN_CLOSETDX lpCloseTdx;
    LPFN_LOGON lpLogon;
    LPFN_LOGOFF lpLogoff;
    LPFN_QUERYDATA lpQueryData;
    LPFN_SENDORDER lpSendOrder;
    LPFN_CANCELORDER lpCancelOrder;
    LPFN_GETQUOTE lpGetQuote;
    LPFN_REPAY lpRepay;
    LPFN_QUERYHISTORYDATA lpQueryHistoryData;
    LPFN_SENDORDERS lpSendOrders;
    LPFN_QUERYDATAS lpQueryDatas;
    LPFN_CANCELORDERS lpCancelOrders;
    LPFN_GETQUOTES lpGetQuotes;

    QMutex apiCallMutex; // add lock to all network call
    bool outputUtf8;

    void setupErrForJson(const char* errout, json& resultJSON);
    uint32_t    seq;
    void allocResultsAndErrorInfos(int count, /* out */ char**& results, /* out */ char**& errorInfos);
    void freeResulsAndErrorInfos(int count, /* out */ char**& results, /* out */ char**& errorInfos);
protected:
    json convertTableToJSON(const char* result, const char* errout);
    json convertMultiTableToJSON(int count, char**& results, char**& errout);

    void _strToTable(const char *result, json& j);

public:
    ~TTS_Dll();
    std::string makeSig(const std::string& accountNo);
    void setOutputUtf8(bool utf8);

    json logon(const char* ip, const short rort,
              const char* version, short yybId,
              const char* accountNo, const char* tradeAccount,
              const char* jyPassword, const char* txPassword);

    json logoff(int clientId);
    json queryData(int clientId, int category);
    json sendOrder(int clientId, int category ,int priceType, const char* gddm, const char* zqdm , float price, int quantity);
    json cancelOrder(int clientId, const char* exchangeId, const char* hth);
    json getQuote(int clientId, const char* zqdm);
    json repay(int clientId, const char* amount);
    json queryHistoryData(int clientId, int category, const char* beginDate, const char* endDate);
    json sendOrders(int clientId, int categories[], int priceTypes[], const char* gddms[], const char* zqdms[], float prices[], int quantities[], int count);
    json queryDatas(int clientId, int categories[], int count);
    json cancelOrders(int clientId, const char* exchangeIds[], const char* hths[], int count);
    json getQuotes(int clientId, const char*  zqdms[], int count);
    const uint32_t getSeq() const {return seq; }

// 实现一个多例模式，针对不同的帐号名，返回不同的
private:
     static std::map<std::string, std::shared_ptr<TTS_Dll>> dlls;
     static std::map<uint32_t, std::string> seqAccountMapping;
     static QMutex initMutex; // add lock init stage
     static QMutex seqMutex;
public:
     static std::shared_ptr<TTS_Dll> getInstance(const TTS_SettingObject& so, const std::string& accountNo);
     static std::shared_ptr<TTS_Dll> getInstance(const TTS_SettingObject& so, const uint32_t seqNo);
     static void preloadDlls(TTS_SettingObject& so);
     const static std::map<std::string, std::shared_ptr<TTS_Dll>>& allDlls() {
        return dlls;
     }
     static volatile uint32_t maxSeq; // 最大id, 从0开始

};

#endif // TTS_DLL_H
