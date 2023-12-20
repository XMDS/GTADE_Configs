#ifndef _LOGGER_H
#define _LOGGER_H

#include <stdio.h>
#define TMPBUF_SIZE 2048 // Max logging buf is 4096 btw

/* Define NOLOGGING if you DONT need logs in any form */
/* You can do it like that in Android.mk: LOCAL_CXXFLAGS += -DNOLOGGING */

enum eLogPrio
{
    LogP_Unk = 0,
    LogP_Default,
    LogP_Verbose,
    LogP_Debug,
    LogP_Info,
    LogP_Warn,
    LogP_Error,
    LogP_Fatal,
    LogP_Silent,
};

class Logger;
extern Logger* logger;
class Logger
{
public:
    Logger();
    void ToggleOutput(bool enabled);
    void SetTag(const char* szTag);
    void Print(eLogPrio prio, const char* szMessage, ...);
    void PrintV(eLogPrio prio, const char* szMessage, va_list args);
    void PrintTag(eLogPrio prio, const char* szTag, const char* szMessage, ...);
    void PrintTagV(eLogPrio prio, const char* szTag, const char* szMessage, va_list args);
    void Info(const char* szMessage, ...);
    void InfoV(const char* szMessage, va_list args);
    void Error(const char* szMessage, ...);
    void ErrorV(const char* szMessage, va_list args);
    #ifdef NOLOGGING
    inline bool HasOutput() { return false; }
    #else
    inline bool HasOutput() { return m_bEnabled; }
    #endif
    inline static Logger* GetLogger() { return logger; }
private:
    const char* m_szTag;
    bool m_bEnabled;
};

#endif // _LOGGER_H
