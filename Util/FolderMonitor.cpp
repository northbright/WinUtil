
#include <windows.h>
#include <tchar.h>

#include "Debug.h"
#include "Thread.h"
#include "FolderMonitor.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Util;

namespace Util {

    static bool FolderMonitorThreadStartRoutine(LPVOID lpVoid, bool* pfStopped, Thread* pThread)
    {
        DWORD dwRet = 0;
        CFolderMonitor* pFolderMon = (CFolderMonitor*)lpVoid;
        DWORD dwWait = 0;
        bool fRet = false;

        while (!(*pfStopped))
        {
            dwWait = WaitForSingleObject(pFolderMon->m_hChange, DEF_NOTIFY_WAIT_TIME);
            if (dwWait != WAIT_OBJECT_0)
                continue;

            fRet = FindNextChangeNotification(pFolderMon->m_hChange);
            PostMessage(pFolderMon->m_hWndNotify, WM_FOLDER_CHANGED, 0, 0);
        }

        return dwRet;
    }
}

CFolderMonitor::CFolderMonitor()
{
    Init();
}

CFolderMonitor::CFolderMonitor(LPCWSTR lpszFolder, HWND hWndNotify, DWORD dwNotifyFilter, bool fWatchSubStree)
{
    Init();
    Set(lpszFolder, hWndNotify, dwNotifyFilter, fWatchSubStree);  
}

CFolderMonitor::~CFolderMonitor()
{
    Stop();
}

void CFolderMonitor::Init()
{
    m_hWndNotify = NULL;
    m_dwNotifyFilter = 0;
    m_fWatchSubTree = false;
    m_hFolder = NULL;
    m_hChange = NULL;
    m_thread.Set(FolderMonitorThreadStartRoutine, this);
}

void CFolderMonitor::Set(LPCWSTR lpszFolder, HWND hWndNotify, DWORD dwNotifyFilter, bool fWatchSubStree)
{
    m_wstrFolder = lpszFolder;
    m_hWndNotify = hWndNotify;
    m_dwNotifyFilter = dwNotifyFilter;
    m_fWatchSubTree = fWatchSubStree;
}

bool CFolderMonitor::Start()
{
    bool fRet = false;

    if (!m_hWndNotify)
        goto END;

    m_hFolder = CreateFile(m_wstrFolder.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (m_hFolder == INVALID_HANDLE_VALUE)
        goto END;

    m_hChange = FindFirstChangeNotification(m_wstrFolder.c_str(), m_fWatchSubTree, m_dwNotifyFilter);
    if (m_hChange == INVALID_HANDLE_VALUE)
        goto END;

    fRet = m_thread.Start();

END:
    return fRet;
}

void CFolderMonitor::Stop()
{
    m_thread.Stop();

    if (m_hChange)
    {
        FindCloseChangeNotification(m_hChange);
        m_hChange = NULL;
    }

    if ((m_hFolder) || (m_hFolder != INVALID_HANDLE_VALUE))
    {
        CloseHandle(m_hFolder);
        m_hFolder = NULL;
    }
}
