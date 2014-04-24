#pragma once

#include <string>
#include <map>

#include "CriticalSection.h"

#define LOG_FILE       L"log.txt"
#define WM_LOG         (WM_APP + 100)  // wParam: malloced LPCWSTR string. Main thread should free the string after use. lParam: ignored.

using namespace std;

// Hooked WndProc
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace Util {

    class LogFile {
    public:
        LogFile();
        LogFile(LPCWSTR lpszCompanyName, LPCWSTR lpszAppName);
        ~LogFile();

        void Init();
        void Deinit();
        bool Set(LPCWSTR lpszCompanyName, LPCWSTR lpszAppName);
        bool Add(LPCWSTR lpszMsg);  // Auto add time and '\r\n' with each message.
        bool Launch();  // Open log file in shell.
        bool Delete();

        wstring m_wstrAppDataFolder;
        wstring m_wstrLogFile;
        CriticalSection m_cs;
    };

    // Callback Func on WM_LOG for WndProc.
    // Params:
    //   hWnd: HWND of current window.
    //   lpszLog: Log string. This string is malloced by Log().
    // Return:
    //   true: malloced log string can be free. false: malloced log string is still in use and caller will free it by itself later.
    typedef bool (*PFN_ON_LOG)(HWND hWnd, LPCWSTR lpszLog);

    // Call Logger.Log() to post WM_LOG message with log message to Window and do some thing in the WndProc, will not block working thread.
    class Logger {
    public:
        Logger();
        Logger(HWND hWnd, LPCWSTR lpszCompanyName, LPCWSTR lpszAppName, PFN_ON_LOG pFnOnLog = NULL);
        ~Logger();

        void Init();
        void Deinit();
        void Set(HWND hWnd, LPCWSTR lpszCompanyName, LPCWSTR lpszAppName, PFN_ON_LOG pFnOnLog = NULL);

        // Call Log() in a thread, and window will receive WM_LOG message, process the message on WndProc will not block working thread.
        bool Log(LPCWSTR format, ...);

        // Get Logger by HWND.
        static Logger* GetLogger(HWND hWnd);
        bool AddMsgToLogFile(LPCWSTR lpszLog);  // Auto add time and '\r\n' with each message.
        bool LaunchLogFile();  // Launch log file in shell.
        bool DeleteLogFile();

        HWND m_hWnd;
        LogFile m_logFile;
        PFN_ON_LOG m_pFnOnLog;
        WNDPROC m_wndProcOld;  // Hook old WndProc

        static map<HWND, Logger*> m_loggerMap;  // Bind Logger with HWND.
    };

}