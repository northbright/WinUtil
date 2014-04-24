
#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>  // PathFileExists()

#include "Debug.h"
#include "Path.h"
#include "Console.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Shlwapi.lib")

namespace Util {

    bool RunCmd(LPCWSTR lpszCmdLine, wstring& strOutput, int nShowCmd, DWORD dwWaitTime)
    {
        bool fRet = false;
        DWORD dwRet = 0;
        UINT nRet = 0;
        HANDLE hEvent = INVALID_HANDLE_VALUE;
        HANDLE hFileOutputSize = NULL;
        HANDLE hFile = NULL;
        DWORD* pdwOutputSize = NULL;
        DWORD dwOutputSize = 0;
        BYTE* pOutput = NULL;
        wstring strMyCmdExePath;

        strOutput.clear();

        if ((!lpszCmdLine) || (!_tcslen(lpszCmdLine)))
            goto END;

        fRet = GetFullPathRelativeToCurrentModule(MY_CMD_EXE, strMyCmdExePath);
        if ((!fRet) || (!PathFileExists(strMyCmdExePath.c_str())))
            goto END;

        nRet = (UINT)ShellExecute(NULL, NULL, strMyCmdExePath.c_str(), lpszCmdLine, NULL, nShowCmd);
        if (nRet <= 32)
            goto END;

        hEvent = CreateEvent(NULL, false, false, MY_CMD_OUTPUT_EVENT);
        if (hEvent == INVALID_HANDLE_VALUE)
            goto END;

        dwRet = WaitForSingleObject(hEvent, dwWaitTime);
        if (dwRet != WAIT_OBJECT_0)
            goto END;

        hFileOutputSize = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, MY_CMD_OUTPUT_SIZE_FILE_MAPPING);
        if (!hFileOutputSize)
            goto END;

        pdwOutputSize = (DWORD*)MapViewOfFile(hFileOutputSize, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD));
        if (!pdwOutputSize)
            goto END;

        dwOutputSize = *pdwOutputSize; 

        hFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, MY_CMD_OUTPUT_FILE_MAPPING);
        if (!hFile)
            goto END;

        pOutput = (BYTE*)MapViewOfFile(hFile, FILE_MAP_ALL_ACCESS, 0, 0, dwOutputSize);
        if (dwOutputSize)
            strOutput = (TCHAR*)pOutput;

        // ---------- Done -----------
        fRet = true;

    END:
        if ((hEvent) && (hEvent != INVALID_HANDLE_VALUE))  
            CloseHandle(hEvent);

        if (pdwOutputSize)
            UnmapViewOfFile(pdwOutputSize);

        if (pOutput)
            UnmapViewOfFile(pOutput);

        if (hFileOutputSize)
            CloseHandle(hFileOutputSize);

        if (hFile)
            CloseHandle(hFile);

        return fRet;
    }

    bool RunCmd(LPCWSTR lpszCmdLine, string& strUTF8Output, int nShowCmd, DWORD dwWaitTime)
    {
        bool fRet = false;
        wstring wstrOutput;
        char* pBuffer = NULL;
        size_t nLen = 0;

        strUTF8Output.clear();

        if (!(RunCmd(lpszCmdLine, wstrOutput, nShowCmd, dwWaitTime)))
            goto END;

        nLen = wstrOutput.length();
        if (!nLen)
            goto END;

        pBuffer = new char[nLen + 1];
        if (!pBuffer)
            goto END;

        memset(pBuffer, 0, sizeof(char) * (nLen + 1));
        WideCharToMultiByte(CP_UTF8, 0, wstrOutput.c_str(), nLen, pBuffer, nLen + 1, NULL, NULL);

        strUTF8Output = pBuffer;

        // Done
        fRet = true;

    END:
        if (pBuffer)
            delete[] pBuffer;

        return fRet;
    }            

}