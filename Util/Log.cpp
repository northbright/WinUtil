#include <Windows.h>
#include <tchar.h>

#include "Debug.h"
#include "Path.h"
#include "Log.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Util;

map<HWND, Logger*> Logger::m_loggerMap;  // Init static variables of Logger.

LogFile::LogFile() {
}

LogFile::LogFile(LPCWSTR lpszCompanyName, LPCWSTR lpszAppName) {
    Set(lpszCompanyName, lpszAppName);
}

LogFile::~LogFile() {
    Deinit();
}

void LogFile::Init() {
}

void LogFile::Deinit() {
    m_wstrAppDataFolder.clear();
    m_wstrLogFile.clear();
}

bool LogFile::Set(LPCWSTR lpszCompanyName, LPCWSTR lpszAppName) {
    bool fRet = false;

    m_cs.Enter();
    Deinit();

    Util::MakeMyAppDataFolder(lpszCompanyName, lpszAppName, m_wstrAppDataFolder);
    if (m_wstrAppDataFolder.empty())
        goto END;

    if (!Util::CreateDir(m_wstrAppDataFolder.c_str()))
        goto END;

    m_wstrLogFile = m_wstrAppDataFolder + L"\\";
    m_wstrLogFile += LOG_FILE;

END:
    m_cs.Leave();
    return true;
}

bool LogFile::Add(LPCWSTR lpszMsg) {
    bool fRet = false;
    DWORD dwOut = 0;
    wchar_t szBuffer[256] = {0};
    wstring wstrLog;
    SYSTEMTIME st = {0};
    size_t nCount = 0;
    HANDLE hFile = NULL;
    
    m_cs.Enter();

    if (!lpszMsg)
        goto END;

    hFile = CreateFile(m_wstrLogFile.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        goto END;

    GetLocalTime(&st);
    wsprintf(szBuffer, L"%d/%d %02d:%02d:%02d ", st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    wstrLog += szBuffer;
    wstrLog += lpszMsg;
    wstrLog += L"\r\n";

    SetFilePointer(hFile, 0, NULL, FILE_END);
    if (!WriteFile(hFile, wstrLog.data(), wstrLog.length() * sizeof(wchar_t), &dwOut, NULL))
        goto END;

    fRet = (bool)FlushFileBuffers(hFile);
END:
    if (hFile && hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    m_cs.Leave();
    return fRet;
}

bool LogFile::Launch() {
    bool fRet = false;

    m_cs.Enter();
    fRet = (UINT)ShellExecute(NULL, _T("open"), m_wstrLogFile.c_str(), NULL, NULL, SW_SHOW) > 32 ? true : false;
    m_cs.Leave();

    return fRet;
}

bool LogFile::Delete() {
    bool fRet = false;

    m_cs.Enter();
    fRet = (bool)DeleteFile(m_wstrLogFile.c_str());
    m_cs.Leave();

    return fRet;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Logger* p = Logger::GetLogger(hWnd);
    if ((!p) || (!p->m_wndProcOld))
        return 0;

    if ((message == WM_LOG) && (p->m_pFnOnLog) && ((LPCWSTR)wParam)) {
        p->AddMsgToLogFile((LPCWSTR)wParam);
        if (p->m_pFnOnLog(hWnd, (LPCWSTR)wParam))
            delete[] (wchar_t*)wParam;  // delete new [] wchar_t
        
        return 0;
    }else {
        return p->m_wndProcOld(hWnd, message, wParam, lParam);
    }
}


Logger::Logger() {
    Init();
}

Logger::Logger(HWND hWnd, LPCWSTR lpszCompanyName, LPCWSTR lpszAppName, PFN_ON_LOG pFnOnLog) {
    Init();
    Set(hWnd, lpszCompanyName, lpszAppName, pFnOnLog);
}

Logger::~Logger() {
    Deinit();
}

void Logger::Init() {
    m_hWnd = NULL;
    m_pFnOnLog = NULL;
    m_wndProcOld = NULL;
}

void Logger::Deinit() {
    if (IsWindow(m_hWnd) && (m_wndProcOld))
        SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_wndProcOld);

    m_hWnd = NULL;
    m_pFnOnLog = NULL;
    m_wndProcOld = NULL;
}

void Logger::Set(HWND hWnd, LPCWSTR lpszCompanyName, LPCWSTR lpszAppName, PFN_ON_LOG pFnOnLog) {

    Deinit();

    m_pFnOnLog = pFnOnLog;

    if (IsWindow(hWnd)) {
        m_hWnd = hWnd;
        m_wndProcOld = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
        
        if (m_wndProcOld)
            m_loggerMap[m_hWnd] = this;
    }

    m_logFile.Set(lpszCompanyName, lpszAppName);
}

bool Logger::Log(LPCWSTR format, ...) {
    bool fRet = false;
    va_list args;
    int nCount = 0;
    int nRet = 0;
    wchar_t* pBuffer = NULL;

    if ((!m_hWnd) || (!m_pFnOnLog))
        return false;

    va_start(args, format);
    nCount = _vscwprintf(format, args);
    if (!nCount)
        goto END;

    pBuffer = new wchar_t[nCount + 1];  // // WndProc should delete[] pBuffer after WM_LOG is processed.
    if (!pBuffer)
        goto END;

    memset(pBuffer, 0, sizeof(wchar_t) * (nCount + 1));
    nRet = vswprintf(pBuffer, nCount + 1, format, args);
    if (nRet <= 0)
        goto END;

    fRet =  (bool)PostMessage(m_hWnd, WM_LOG, (WPARAM)pBuffer, 0);
END:
    va_end(args);
    return fRet;
}

Logger* Logger::GetLogger(HWND hWnd) {
    if (m_loggerMap.find(hWnd) != m_loggerMap.end())
        return m_loggerMap[hWnd];
    else
        return NULL;
}

bool Logger::AddMsgToLogFile(LPCWSTR lpszLog) {
    return m_logFile.Add(lpszLog);
}

bool Logger::LaunchLogFile() {
    return m_logFile.Launch();
}

bool Logger::DeleteLogFile() {
    return m_logFile.Delete();
}
