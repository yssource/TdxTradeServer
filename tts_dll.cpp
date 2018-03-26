#include "tts_dll.h"
#include "json.hpp"
#include "tts_common.h"
#include <QTextCodec>

using namespace std;
using json = nlohmann::json;

QMutex TTS_Dll::initMutex; // add lock init stage
QMutex TTS_Dll::seqMutex; // seq issue lock;

std::map<std::string, std::shared_ptr<TTS_Dll>> TTS_Dll::dlls;
std::map<uint32_t, std::string> TTS_Dll::seqAccountMapping;

volatile uint32_t TTS_Dll::maxSeq = 0;

TTS_Dll::TTS_Dll(const TTS_SettingObject& so, const std::string& accountNo):apiCallMutex()
{
    outputUtf8 = true;
    string strDllFilePath = so.trade_dll_path.toStdString();

    // 判断是否是单账号模式，如果是单账号模式，则直接再入dll, 如果不是，则检查是否需要生成新的dll
    if (so.multiaccount == false) {
        hDLL = LoadLibraryA(strDllFilePath.c_str());
    } else {
        QDir accountDllDir;
        std::string sig = makeSig(accountNo);

        if (so.dlls_path == "") {
            QRegExp sep("[\\\\/]");
            accountDllDir = QDir(so.trade_dll_path);
            accountDllDir.makeAbsolute();
            accountDllDir.cdUp();
            qDebug() << "Parent Dir of tradedll is : " << accountDllDir.absolutePath();
        } else {
            accountDllDir = QDir(so.dlls_path);
        }

        if (!accountDllDir.exists()) {
            qInfo() << "dlls_path is not exits :" << accountDllDir.absolutePath();
        }
        QString accountDllFileName = QString("trade_") + QString(accountNo.c_str()) + "_" + QString(sig.c_str()) + QString(".dll");
        QString accountDllFilePath = accountDllDir.filePath(accountDllFileName);
        if (!QFileInfo(accountDllFilePath).exists()) {
            // 开始生成dll
            // 临时文件
            QString tmpFileName = QDir::temp().filePath(QUuid::createUuid().toString());
            // 复制模板
            if (!QFile(so.trade_dll_path).copy(tmpFileName)){
                qInfo() << "copy dll from template error : " << tmpFileName;
            }
            // 替换sig
            QFile tmpFile(tmpFileName);
            tmpFile.open(QIODevice::ReadWrite | QIODevice::Unbuffered);
            tmpFile.seek(1132713);
            QByteArray qb(14, 0);
            qb.replace(0, sig.length(), sig.c_str());
            tmpFile.write(qb.data(), 14);
            // move to path
            tmpFile.rename(accountDllFilePath);
            tmpFile.flush();
            tmpFile.close();
            qInfo() << "Account Dll for " << accountNo.c_str() << " path : " << accountDllFilePath;
        } else {
            qInfo() << "[Already Exits] Account Dll for " << accountNo.c_str() << " path : " << accountDllFilePath;
        }
        // fix for windows seperator
        accountDllFilePath.replace(QRegularExpression("/"), "\\");
        strDllFilePath = accountDllFilePath.toStdString();
        hDLL = LoadLibraryA(strDllFilePath.c_str());
    }

    if (0 == hDLL) {
        qInfo() << "Load dll failed " ;
        initSuccess = false;
    }

    // load functions
    lpOpenTdx = (LPFN_OPENTDX) GetProcAddress(hDLL, "OpenTdx");
    lpCloseTdx = (LPFN_CLOSETDX) GetProcAddress(hDLL, "CloseTdx");
    lpLogon = (LPFN_LOGON) GetProcAddress(hDLL, "Logon");
    lpLogoff = (LPFN_LOGOFF) GetProcAddress(hDLL, "Logoff");
    lpQueryData = (LPFN_QUERYDATA) GetProcAddress(hDLL, "QueryData");
    lpSendOrder = (LPFN_SENDORDER) GetProcAddress(hDLL, "SendOrder");
    lpCancelOrder = (LPFN_CANCELORDER) GetProcAddress(hDLL, "CancelOrder");
    lpGetQuote = (LPFN_GETQUOTE) GetProcAddress(hDLL, "GetQuote");
    lpRepay = (LPFN_REPAY) GetProcAddress(hDLL, "Repay");
    lpQueryHistoryData = (LPFN_QUERYHISTORYDATA) GetProcAddress(hDLL, "QueryHistoryData");
    lpQueryDatas = (LPFN_QUERYDATAS) GetProcAddress(hDLL, "QueryDatas");
    lpSendOrders = (LPFN_SENDORDERS) GetProcAddress(hDLL, "SendOrders");
    lpCancelOrders = (LPFN_CANCELORDERS) GetProcAddress(hDLL, "CancelOrders");
    lpGetQuotes = (LPFN_GETQUOTES) GetProcAddress(hDLL, "GetQuotes");
    // end load functioins

    // initialize tdx
    lpOpenTdx();

    errout = new char[TTS_ERROR_CONTENT_MAX_SIZE];
    result = new char[TTS_RESULT_CONTENT_MAX_SIZE];
    initSuccess = true;
    TTS_Dll::seqMutex.lock();
    seq= TTS_Dll::maxSeq++;
    TTS_Dll::seqMutex.unlock();
}

TTS_Dll::~TTS_Dll()
{
    // close tdx
    lpCloseTdx();
    if (hDLL) {
        FreeLibrary(hDLL);
    }
    delete[] errout;
    delete[] result;
}


std::shared_ptr<TTS_Dll> TTS_Dll::getInstance(const TTS_SettingObject& so, const std::string& accountNo)
{
    // 类似双重否定的单例模式，这里还是可能会出现一些线程同步问题的，但是几率相对不高，效率还是可以的。
    if (dlls.find(accountNo) == dlls.end())
    {
        initMutex.lock();
        // 首先查找dll实例是否创建
        if (dlls.find(accountNo) == dlls.end()) {
            shared_ptr<TTS_Dll> dll(new TTS_Dll(so, accountNo));
            dlls[accountNo] = dll;
            seqAccountMapping[dll->getSeq()] = accountNo;
            qDebug() << dll->getSeq() << ":" << accountNo.c_str();
        }

        initMutex.unlock();
    }
    return dlls[accountNo];
}

std::shared_ptr<TTS_Dll> TTS_Dll::getInstance(const TTS_SettingObject &so, const uint32_t seqNo)
{
    if (seqAccountMapping.find(seqNo) == seqAccountMapping.end()) {
        return nullptr;
    } else {
        return getInstance(so, seqAccountMapping[seqNo]);
    }
}


std::string TTS_Dll::makeSig(const std::string& accountNo)
{
    // 获取奇数位
    size_t oddSize = (accountNo.length() + 1) / 2;
    // cout <<" odd size is " << oddSize << endl;
    std::vector<char> oddStr;
    for (size_t i = 0 ; i < oddSize; i++) {
        oddStr.push_back(accountNo.c_str()[i * 2]);
    }

    uint16_t a3 = 0x55e;

    std::string result = "";
    for (uint16_t c : oddStr) {
        bool _next = true;
        uint16_t a = c;
        uint16_t b = a3 >> 0x8;
        c = a ^ b;
        a3 = (0x207f * (a3 + c) - 0x523d) & 0xffff;
        uint16_t j = 64;
        while (_next){
            j += 1;
            if (j > 90)
                break;
            uint16_t k = 91;
            while (_next) {
                k -= 1;
                if (k < 65)
                    break;

                uint16_t temp = 1755 + c - k;
                if ((temp % 26 == 0) && uint16_t(temp / 26) == j) {

                    result.append({(char)(j & 0xff), (char)(k & 0xff)});
                    _next = false;
                }
            }
        }
    }
    return result;
}

void TTS_Dll::preloadDlls(TTS_SettingObject& so)
{
    if (so.multiaccount) {
        if (!so.preload_accounts.isEmpty()) {
            qInfo() << "Prepare to preload the following account dlls: " << so.preload_accounts.join(",");
            for(const QString& oneAccount: so.preload_accounts) {
                std::string oneAccountStr = oneAccount.toStdString();
                TTS_Dll::getInstance(so, oneAccountStr);
            }
        }
    }
}


void TTS_Dll::setOutputUtf8(bool utf8) {
    outputUtf8 = utf8;
}

void TTS_Dll::allocResultsAndErrorInfos(int count, char**& results, char**& errorInfos)
{
    results = new char*[count];
    errorInfos = new char*[count];
    for (int n = 0; n < count; n++) {
        results[n] = new char[TTS_RESULT_CONTENT_MAX_SIZE];
        errorInfos[n] = new char[TTS_ERROR_CONTENT_MAX_SIZE];
    }
}

void TTS_Dll::freeResulsAndErrorInfos(int count, char**& results, char**& errorInfos)
{
    for (int n = 0; n < count; n++) {
        delete[] results[n];
        delete[] errorInfos[n];
        results[n] = NULL;
        errorInfos[n] = NULL;
    }

    delete[] results;
    delete[] errorInfos;
}

/**
 * @brief TTS_Dll::logon 登陆到服务器
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
json TTS_Dll::logon(const char* ip, const short port,
                        const char* version, short yybId,
                        const char* accountNo, const char* tradeAccount,
                        const char* jyPassword, const char* txPassword) {
    QMutexLocker ml(&apiCallMutex);
    json j;
    int ret = lpLogon(ip, port, version, yybId, accountNo, tradeAccount, jyPassword, txPassword, errout);
    if (ret == -1) {
        j[TTS_SUCCESS] = false;
        setupErrForJson(errout, j);
        return j;
    } else {
        j[TTS_SUCCESS] = true;
        j[TTS_DATA]["client_id"] = ret;
    }
    return j;
}

/**
 * @brief TTS_Dll::logoff 登出
 * @param ClientID cilent_id
 * @return success => true/false
 */
json TTS_Dll::logoff(int clientId) {
    QMutexLocker ml(&apiCallMutex);
    lpLogoff(clientId);
    json j;
    j[TTS_SUCCESS] = true;
    return j;
}

/**
 * @brief TTS_Dll::queryData 查询信息
 * @param ClientID client_id
 * @param Category 信息类别
 * @return [{}, {}, {} ]
 */
json TTS_Dll::queryData(int clientId, int category) {
    QMutexLocker ml(&apiCallMutex);
    lpQueryData(clientId, category, result, errout);
    return convertTableToJSON(result, errout);
}


json TTS_Dll::sendOrder(int clientId, int category ,int priceType, const char* gddm,  const char* zqdm , float price, int quantity) {
    QMutexLocker ml(&apiCallMutex);
    lpSendOrder(clientId, category, priceType, gddm, zqdm, price, quantity, result, errout);
    return convertTableToJSON(result, errout);
}

json TTS_Dll::cancelOrder(int clientId, const char *exchangeId, const char *hth) {
    QMutexLocker ml(&apiCallMutex);
    lpCancelOrder(clientId, exchangeId, hth, result, errout);
    return convertTableToJSON(result, errout);
}

json TTS_Dll::getQuote(int clientId, const char *zqdm) {
    QMutexLocker ml(&apiCallMutex);
    lpGetQuote(clientId, zqdm, result, errout);
    return convertTableToJSON(result, errout);
}

json TTS_Dll::repay(int clientId, const char *amount) {
    QMutexLocker ml(&apiCallMutex);
    lpRepay(clientId, amount, result, errout);
    return convertTableToJSON(result, errout);
}

json TTS_Dll::queryHistoryData(int clientId, int category, const char* beginDate, const char* endDate) {
    QMutexLocker ml(&apiCallMutex);
    lpQueryHistoryData(clientId, category, beginDate, endDate, result, errout);
    return convertTableToJSON(result, errout);
}

json TTS_Dll::sendOrders(int clientId, int categories[], int priceTypes[], const char *gddms[], const char *zqdms[], float prices[], int quantities[], int count) {
    QMutexLocker ml(&apiCallMutex);
    char** results;
    char** errorInfos;
    allocResultsAndErrorInfos(count, results, errorInfos);
    lpSendOrders(clientId, categories, priceTypes, gddms, zqdms, prices, quantities, count, results, errorInfos);
    json resultJSON = convertMultiTableToJSON(count, results, errorInfos);
    freeResulsAndErrorInfos(count, results, errorInfos);
    return resultJSON;
}

json TTS_Dll::queryDatas(int clientId, int categories[], int count) {
    QMutexLocker ml(&apiCallMutex);
    char** results;
    char** errorInfos;
    allocResultsAndErrorInfos(count, results, errorInfos);
    lpQueryDatas(clientId, categories, count, results, errorInfos);
    json resultJSON = convertMultiTableToJSON(count, results, errorInfos);
    freeResulsAndErrorInfos(count, results, errorInfos);
    return resultJSON;
}

json TTS_Dll::cancelOrders(int clientId, const char *exchangeIds[], const char *hths[], int count) {
    QMutexLocker ml(&apiCallMutex);
    char** results;
    char** errorInfos;
    allocResultsAndErrorInfos(count, results, errorInfos);
    lpCancelOrders(clientId, exchangeIds, hths, count, results, errorInfos);
    json resultJSON = convertMultiTableToJSON(count, results, errorInfos);
    freeResulsAndErrorInfos(count, results, errorInfos);
    return resultJSON;
}

json TTS_Dll::getQuotes(int clientId, const char *zqdms[], int count) {
    QMutexLocker ml(&apiCallMutex);
    char** results;
    char** errorInfos;
    allocResultsAndErrorInfos(count, results, errorInfos);
    lpGetQuotes(clientId, zqdms, count, results, errorInfos);
    json resultJSON = convertMultiTableToJSON(count, results, errorInfos);
    freeResulsAndErrorInfos(count, results, errorInfos);
    return resultJSON;
}

void TTS_Dll::setupErrForJson(const char* errout, json& resultJSON)
{
    if (outputUtf8) {
        QString qErrout = QString::fromLocal8Bit(errout);
        string utf8Errout = qErrout.toUtf8().constData();
        resultJSON[TTS_ERROR] = utf8Errout;
    } else {
        resultJSON[TTS_ERROR] = errout;
    }
}

void TTS_Dll::_strToTable(const char *result, json& j)
{
    QString strResult = QString::fromLocal8Bit(result);
    // qInfo() << strResult;
    QStringList sl = strResult.split("\n");
    if (sl.length() > 1) {
        QString head = sl[0];
        QStringList hlist = head.split("\t");

        int line = 0;
        for (QString row : sl) {
            line++;
            if (line==1) continue;

            QStringList rowlist = row.split("\t");

            json oneRecord = json({});
            for(int i = 0; i < hlist.length(); i++) {
                string key, value;
                if (outputUtf8) {
                    key = hlist[i].toUtf8();
                    value = rowlist[i].toUtf8();
                } else {
                    key = hlist[i].toLocal8Bit();
                    value = rowlist[i].toLocal8Bit();
                }

                oneRecord[key]= value;
            }
            j.push_back(oneRecord);
        }
    }
}

/**
 * @brief TTS_Dll::convertTableToJSON 将\n分割行\t分割字符的类似 csv格式的信息转换为json格式
 * @param result
 * @return  json结构的 [{line1}, {line2} ... ] 信息
 */

json TTS_Dll::convertTableToJSON(const char *result, const char* errout) {
    json resultJSON;
    if (result[0] == 0) {
        resultJSON[TTS_SUCCESS] = false;
        setupErrForJson(errout, resultJSON);
        return resultJSON;
    }

    json j;
    j = json::array();
    _strToTable(result, j);
    resultJSON[TTS_SUCCESS] = true;
    resultJSON[TTS_DATA] = j;
    return resultJSON;
}

json TTS_Dll::convertMultiTableToJSON(int count, char**& results, char**& errouts) {
    json resultJSON;
    if (count <=0) {
        resultJSON[TTS_SUCCESS] = false;
        setupErrForJson("count is less than zero", resultJSON);
        return resultJSON;
    }

    resultJSON[TTS_SUCCESS] = true;
    resultJSON[TTS_DATA] = json::array();

    for (int n = 0; n < count; n++) {
        json oneJsonEntry;
        char* result = results[n];
        char* errout = errouts[n];
        if (result[0] == 0) {
            oneJsonEntry[TTS_SUCCESS] = false;
            setupErrForJson(errout, oneJsonEntry);
        }
        json j;
        j = json::array();
        _strToTable(result, j);
        oneJsonEntry[TTS_SUCCESS] = true;
        oneJsonEntry[TTS_DATA] = j;
        resultJSON[TTS_DATA].push_back(oneJsonEntry);
    }

    return resultJSON;
}



