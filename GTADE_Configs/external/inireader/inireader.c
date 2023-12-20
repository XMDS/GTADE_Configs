#include "inireader.h"
#include "minIni.h"
#include <errno.h>
#include <limits.h>

void SetIniPath(const char* szFileName)
{
    inireader.iniName = szFileName;
}

int ReadInteger(char* szSection, char* szKey, int iDefaultValue)
{
    return ini_getl(szSection, szKey, iDefaultValue, inireader.iniName);
}

float ReadFloat(char* szSection, char* szKey, float fltDefaultValue)
{
    return ini_getf(szSection, szKey, fltDefaultValue, inireader.iniName);
}

bool ReadBoolean(char* szSection, char* szKey, bool bolDefaultValue)
{
    return ini_getbool(szSection, szKey, bolDefaultValue, inireader.iniName);
}

char* ReadString(char* szSection, char* szKey, char* szDefaultValue, char* Buffer, int BufferSize)
{
    ini_gets(szSection, szKey, szDefaultValue, Buffer, BufferSize, inireader.iniName);
    return Buffer;
}

int WriteInteger(char* szSection, char* szKey, int iValue)
{
    return ini_putl(szSection, szKey, iValue, inireader.iniName);
}

int WriteFloat(char* szSection, char* szKey, float fltValue)
{
    return ini_putf(szSection, szKey, fltValue, inireader.iniName);
}

int WriteBoolean(char* szSection, char* szKey, bool bolValue)
{
    return ini_putl(szSection, szKey, bolValue, inireader.iniName);
}

int WriteString(char* szSection, char* szKey, char* szValue)
{
    return ini_puts(szSection, szKey, szValue, inireader.iniName);
}

struct inireader_t inireader =
{
    .iniName = 0,
    .SetIniPath = SetIniPath,
    .ReadInteger = ReadInteger,
    .ReadFloat = ReadFloat,
    .ReadBoolean = ReadBoolean,
    .ReadString = ReadString,
    .WriteInteger = WriteInteger,
    .WriteFloat = WriteFloat,
    .WriteBoolean = WriteBoolean,
    .WriteString = WriteString
};