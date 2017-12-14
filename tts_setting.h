#ifndef TTS_SETTING_H
#define TTS_SETTING_H

#include <QObject>
#include <QSettings>

#define DEFAULT_TRADE_DLL_NAME  "trade.dll"
#define DEFAULT_PORT            19820
#define DEFAULT_BIND            "127.0.0.1"

typedef struct _TTS_SettingObject {
    int32_t     port;               // 监听端口
    QString     trade_dll_path;     // dll路径
    QString     bind;               // 邦定ip
    bool        ssl_enabled;        // 是否启用ssl
    QString     ssl_private_key;    // 私钥
    QString     ssl_certificate;    // CA 证书
    QString     transport_enc_key; // 对传输进行签名的密钥
    QString     transport_enc_iv;  //
    bool        transport_enc_enabled; // 是否开启了签名加密
    bool        multiaccount;           // 多用户模式
    QStringList preload_accounts;   // 多用户模式下，预加载的帐号
    QString     dlls_path;          // 多用户下，新生成的dll文件的目录，如果为空，则默认使用trade_dll_path所在的目录
}  TTS_SettingObject;

class TTS_Setting : public QObject
{
    Q_OBJECT
public:
    explicit TTS_Setting(QObject *parent = nullptr);


public:
    static QSettings* loadSettingsFile();
    static TTS_SettingObject loadSettings();

signals:

public slots:
};

#endif // TTS_SETTING_H
