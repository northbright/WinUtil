#pragma once

#include <string>
#include <map>

#include "CriticalSection.h"

using namespace std;

// Internal use to control thread.
#define MSG_START_THREAD                   (WM_APP + 200)  // wParam: Thread Id. lParam: Ignored.
#define MSG_STOP_THREAD                    (WM_APP + 210)  // wParam: Thread Id. lParam: Ignored.

// WndProc should proceed these message.
#define WM_THREAD_PROGRESS_CHANGED         (WM_APP + 300)  // wParam: Thread Id. lParam: Progress.
#define WM_THREAD_SUCCEEDED                (WM_APP + 310)  // wParam: Thread Id. lParam: Ignored.
#define WM_THREAD_FAILED                   (WM_APP + 311)  // wParam: Thread Id. lParam: Ignored.
#define WM_THREAD_STOPPED                  (WM_APP + 312)  // wParam: Thread Id. lParam: Ignored.

namespace Util {

    class Thread;
    class ThreadGroup;

    typedef int (*PFN_COMPUTE_TOTAL_PROGRESS)(
        LPVOID lpParams,  // Params defined by user.
        int nSubProgress  // Sub progress, used to computer total progress if needed.
    );

    typedef bool (*PFN_THREAD_START_ROUTINE)(
        LPVOID lpParams,  // Params defined by user.
        bool* pfStopped,  // Used to inform the task function that user wants to stop task. The function should exit when this flag is set to true.
        Util::Thread* pThread  // Call Util::Thread::UpdateProgress() if need.
    );

    // PFN_THREAD_START_ROUTINE
    // Return: true: thread succeeded. false: thread failed.
    // Ex:
    //bool SDCardTestThreadStartRoutine(LPVOID lpParams, bool* pfStopped, Util::Thread* pThread)
    //{
    //    bool fRet = false;
    //    MyParam* pMyParam = (MyParam*)lpParams;
    //    Util::Thread* pThread = (Util::Thread*)pThread;
    //    int nOldProgress = 0;
    //    int nProgress = 0;
    //
    //    for (i = 0; i < XX; i++)
    //    {
    //        if (*pfStopped)      
    //            goto END;
    //
    //        DoSometing();
    //        PostMessage(pThread->m_hWndNotify, WM_MY_MESSAGE, 0, 0); // Thread::m_hWndNotify can be used to post messages.
    //        nProgress = CalculateProgress();
    //        if (nProgress != nOldProgress)
    //        {
    //            nOldProgress = nProgress;  // Update progress.
    //            pThread->UpdateProgress(nProgress);  // Notify UI thread that progress changed if need.
    //        }
    //    }
    //    fRet = true;
    //END:    
    //    return fRet; 
    //}

    DWORD WINAPI ThreadStartRoutine(LPVOID lpVoid);

    DWORD WINAPI MonitorThreadStartRoutine(LPVOID lpVoid);

    class Thread {
    public:
        Thread();
        // Params:
        //   pFnThreadStartRoutine: Thread Start Routine.
        //   lpVoid: Thread params.
        //   hWndNotify: Window to notify.
        //   pfStopped: Used to stop the thread. If NULL, it will point to the m_fStopped member.
        //   pFnComputeTotalProgress: User's customized func to computer total progress by sub progress, will be invoked by Util::Thread::UpdateProgress(). If NULL, total progress will be the same value as passed to UpdateProgress().);
        Thread(PFN_THREAD_START_ROUTINE pFnThreadStartRoutine, LPVOID lpVoid = NULL, HWND hWndNotify = NULL, bool* pfStopped = NULL, PFN_COMPUTE_TOTAL_PROGRESS pFnComputeTotalProgress = NULL);
        ~Thread();

        void Init();

        void Set(PFN_THREAD_START_ROUTINE pFnThreadStartRoutine, LPVOID lpVoid = NULL, HWND hWndNotify = NULL, bool* pfStopped = NULL, PFN_COMPUTE_TOTAL_PROGRESS pFnComputeTotalProgress = NULL);
        bool IsValid();
        void SetNotifyWindow(HWND hWndNotify);
        void SetGroup(ThreadGroup* pThreadGroup);
        void SetStoppedFlag(bool* pfStopped);
        void SetComputerTotalProgressFunc(PFN_COMPUTE_TOTAL_PROGRESS pFunc);
        void UpdateProgress(int nProgress);
        
        void OnExit(DWORD dwExitCode);
        bool IsRunning();
        bool Start(DWORD* pdwThreadId = NULL);  // Return Thread Id.
        void Stop();
        void StopAsync();  // non-block stop. Used for ThreadGroup.

        PFN_THREAD_START_ROUTINE m_pFnThreadStartRoutine;
        LPVOID m_lpVoid;
        HWND m_hWndNotify;
        bool m_fStopped;
        bool* m_pfStopped;

        bool m_fRunning;
        HANDLE m_hThread;
        HANDLE m_hMonitorThread;
        DWORD m_dwThreadId;
        int m_nProgress;
        HANDLE m_hEventExit;
        HANDLE m_hEventControl;

        Util::CriticalSection m_cs;
        ThreadGroup* m_pThreadGroup;
        PFN_COMPUTE_TOTAL_PROGRESS m_pFnComputeTotalProgress;
    };
}