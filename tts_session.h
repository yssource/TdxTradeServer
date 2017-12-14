#ifndef TTS_SESSION_H
#define TTS_SESSION_H


typedef struct _TTS_Session {
    uint32_t    sessionNo;      // tts session to backend dll
    uint32_t    loginId;        // login id for account
    std::string accountNo;  // account id of dll file
} TTS_Session;

#endif // TTS_SESSION_H
