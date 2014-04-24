#pragma once

#include "ThreadGroup.h"
#include "CriticalSection.h"
#include "Log.h"

#include <string>
#include <set>
#include <vector>

using namespace std;

#define MAX_DECOMPILE_THREAD_COUNT          64

namespace Util {

    class ChmDecompiler;

    typedef struct _CHM_DECOMPILE_INFO {
        wstring wstrChmFile;
        wstring wstrOutputDir;
    }CHM_DECOMPILE_INFO;

    typedef struct _CHM_DECOMPILE_THREAD_DATA {
        vector<CHM_DECOMPILE_INFO*> chmDecomipleInfos;
        size_t nDone;  // count of decompiled files.
        ChmDecompiler* pChmDecompiler;
    }CHM_DECOMPILE_THREAD_DATA;

    // lpParams: pointer to CHM_DECOMPILE_THREAD_DATA struct.
    int DecomileChmThreadComputeTotalProgress(LPVOID lpParams, int nSubProgress);

    // lpParams: pointer to CHM_DECOMPILE_THREAD_DATA struct.
    bool DecompileChmThreadStartRoutine(LPVOID lpParams, bool* pfStopped, Util::Thread* pThread);

    class ChmDecompiler {
    public:
        ChmDecompiler();

        // Specify chm files
        ChmDecompiler(set<wstring>& wstrChmFileSet, LPCWSTR lpszOutputDir, HWND hWndNotify = NULL, DWORD dwThreadCount = 4, Logger* pLogger = NULL);

        // Specify an input dir contains chm files.
        ChmDecompiler(LPCWSTR lpszInputDir, LPCWSTR lpszOutputDir, bool fSearchSubDir = true, HWND hWndNotify = NULL, DWORD dwThreadCount = 4, Logger* pLogger = NULL); 
        ~ChmDecompiler();

        void Init();
        void Clear();
        void Set(set<wstring>& wstrChmFileSet, LPCWSTR lpszOutputDir, HWND hWndNotify = NULL, DWORD dwThreadCount = 4, Logger* pLogger = NULL);
        void Set(LPCWSTR lpszInputDir, LPCWSTR lpszOutputDir, bool fSearchSubDir = true, HWND hWndNotify = NULL, DWORD dwThreadCount = 4, Logger* pLogger = NULL);

        bool Start();
        void Stop();
        bool IsRunning();

        // Members
        DWORD m_dwThreadCount;
        ThreadGroup m_threadGroup;
        vector<CHM_DECOMPILE_THREAD_DATA*> m_chmDecompileThreadDatas;
        set<wstring> m_errList;
        CriticalSection m_csErrList;

        Logger* m_pLogger;
    };
}

