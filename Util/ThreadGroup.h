#pragma once

#include <vector>

using namespace std;

#include "Thread.h"

#define WM_THREAD_GROUP_PROGRESS_CHANGED   (WM_APP + 320)  // wParam: Thread Group Id. lParam: Progress.
#define WM_THREAD_GROUP_EXITED             (WM_APP + 321)  // wParam: Thread Group Id. lParam: Ingored.
#define WM_THREAD_GROUP_STOPPED            (WM_APP + 322)  // wParam: Thread Group Id. lParam: Ingored.

#define MAX_THREAD_COUNT_IN_GROUP          65536  // 2^16

namespace Util {
    class ThreadGroup {
    public:
        ThreadGroup();
        ThreadGroup(HWND hWndNotify);
        ~ThreadGroup();

        void Init();
        void Set(HWND hWndNotify);
        UINT GetId();
        bool Add(PFN_THREAD_START_ROUTINE pFnThreadStartRoutine, LPVOID lpVoid = NULL, PFN_COMPUTE_TOTAL_PROGRESS pFnComputeTotalProgress = NULL);
        bool Clear();

        bool Start();
        void Stop();
        bool IsRunning();

        static long m_nCount;
        UINT m_nId;
        HWND m_hWndNotify;
        bool m_fStopped;
        int m_nProgress;
        vector<Thread*> m_threads;
        long m_nRunningThreadCount;
        HANDLE m_hEventExit;
    };
}