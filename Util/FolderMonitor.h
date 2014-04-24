#pragma once

#include <string>
#include "Thread.h"

using namespace std;

#define DEF_NOTIFY_FILTER          (FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE)
#define DEF_NOTIFY_WAIT_TIME       100

// Send to window when folder is changed.
#define WM_FOLDER_CHANGED          (WM_USER + 200)

namespace Util {

    // Work thread start routine for CFolderMonitor.
    static bool FolderMonitorThreadStartRoutine(LPVOID lpVoid, bool* pfStopped, Thread* pThread);

    // On Work thread exits.
    static void OnThreadExit(DWORD dwExitCode, LPVOID lpVoid);


    class CFolderMonitor
    {
    public:
        CFolderMonitor();
        CFolderMonitor(LPCWSTR lpszFolder, HWND hWndNotify, DWORD dwNotifyFilter = DEF_NOTIFY_FILTER, bool fWatchSubStree = false);
        ~CFolderMonitor();

        void Init();
        void Set(LPCWSTR lpszFolder, HWND hWndNotify, DWORD dwNotifyFilter = DEF_NOTIFY_FILTER, bool fWatchSubStree = false);
        bool Start();
        void Stop();

        wstring m_wstrFolder;
        HWND m_hWndNotify;
        DWORD m_dwNotifyFilter;
        bool m_fWatchSubTree;
        HANDLE m_hFolder;
        HANDLE m_hChange;
        Thread m_thread;
    };

}