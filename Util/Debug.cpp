#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <Shlwapi.h>

#include "Debug.h"
#include "String.h"

#include <string>

using namespace std;

#ifdef _DEBUG
void* operator new(size_t nSize, const char * lpszFileName, int nLine)
{
    return ::operator new(nSize, 1, lpszFileName, nLine);
}
#endif

#pragma comment(lib, "Shlwapi.lib")

namespace Util {

    void DBG_MSG(LPCWSTR format, ...)
    {
        va_list args;
        wstring wstr;

        va_start(args, format);
        if (!FormatArgsToWstring(wstr, format, args))
            goto END;

        OutputDebugString(wstr.c_str());
    END:
        va_end(args);
    }
}