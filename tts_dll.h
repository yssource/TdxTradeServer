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

typedef int (__stdcall *LPFN_LOGON)(const char* IP, const short Port, const char* Version, short YybID,  const char* AccountNo, const char* TradeAccount, const char* JyPassword, const char* TxPassword, char* ErrInfo);
typedef void(__stdcall *LPFN_LOGOFF)(int ClientID);
typedef void(__stdcall *LPFN_QUERYDATA)(int ClientID, int Category, char* result, char* errInfo);
typedef void(__stdcall *LPFN_SENDORDER)(int ClientID, int Category ,int PriceType,  const char* Gddm,  const char* Zqdm , float Price, int Quantity,  char* Result, char* ErrInfo);
typedef void(__stdcall *LPFN_CANCELORDER)(int ClientID, const char* ExchangeID, const char* hth, char* Result, char* ErrInfo);
typedef void(__stdcall *LPFN_GETQUOTE)(int ClientID, const char* Zqdm, char* Result, char* ErrInfo);
typedef void(__stdcall *LPFN_REPAY)(int ClientID, const char* Amount, char* Result, char* ErrInfo);
typedef void(__stdcall *LPFN_QUERYHISTORYDATA)(int ClientID, int Category, const char* BeginDate, const char* EndDate, char* Result, char* ErrInfo); //QueryHistoryData

#define P_LOGON         "logon"
#define P_LOGOFF        "logoff"
#define P_QUERYDATA     "query_data"
#define P_SENDORDER     "send_order"
#define P_CANCELORDER   "cancel_order"
#define P_GETQUOTE      "get_quote"
#define P_REPAY         "repay"
#define P_QUERYHISTORYDATA "query_history_data"

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

    QMutex apiCallMutex; // add lock to all network call
    bool outputUtf8;

    void setupErrForJson(const char* errout, json& resultJSON);
    uint32_t    seq;

protected:
    json convertTableToJSON(const char* result, const char* errout);

public:
    ~TTS_Dll();
    std::string makeSig(const std::string& accountNo);
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
    json repay(int ClientID, const char* Amount);
    json queryHistoryData(int ClientID, int Category, const char* BeginDate, const char* EndDate);
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
