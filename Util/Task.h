#pragma once

#include "Thread.h"

#include <string>
#include <vector>
#include <map>

using namespace std;


#define WM_TASK_SUCCEEDED                 (WM_USER + 250)  // Task succeeded. LOWORD(wParam): current task ID. HIWORD(wParam): all tasks object ID. lParam: ignored.
#define WM_TASK_FAILED                    (WM_USER + 251)  // Task failed. LOWORD(wParam): current task ID. HIWORD(wParam): all tasks object ID. lParam: ignored.
#define WM_TASK_PROGRESS_CHANGED          (WM_USER + 252)  // Task progress changed. LOWORD(wParam): current task ID. HIWORD(wParam): all tasks object ID. LOWORD(lParam) = current task progress; HIWORD(lParam) = total tasks progress.
#define WM_TASKS_SUCCEEDED                (WM_USER + 261)  // All Tasks succeeded. LOWORD(wParam): ignored. HIWORD(wParam): all tasks object ID. lParam: ignored.
#define WM_TASKS_FAILED                   (WM_USER + 262)  // All Tasks failed. LOWORD(wParam): ignored. HIWORD(wParam): all tasks object ID. lParam: ignored.
#define WM_TASKS_STOPPED                  (WM_USER + 263)  // Tasks stopped by user. LOWORD(wParam): ignored. HIWORD(wParam): all tasks object ID. lParam: ignord.


typedef BOOL (*PFN_TASK)(
    LPVOID lpParams,  // Params defined by user.
    BOOL* pfStopped,  // Used to inform the task function that user wants to stop task. The function should exit when this flag is set to TRUE.
    HWND hWndNotify,  // HWND used to communicate between task function and UI defined by user.
    HANDLE hEventProgressChanged,  // Set pnProgress and then set this event to notify UI. This will post a WM_TASKS_PROGRESS_CHANGED message to hWndNotify. LOWORD(wParam): current task ID. HIWORD(wParam): all tasks object ID. LOWORD(lParam) = current task progress; HIWORD(lParam) = total tasks progress.
    int* pnProgress  // Set this progress value then set hEventProgressChanged event to notify UI. This will post a WM_TASKS_PROGRESS_CHANGED message to hWndNotify. LOWORD(wParam): current task ID. HIWORD(wParam): all tasks object ID. LOWORD(lParam) = current task progress; HIWORD(lParam) = total tasks progress.
    );

namespace Util {

    // PFN_TASK: task callback function.
    // Return: TRUE: task succeeded. FALSE: task failed.
    // Ex:
    //BOOL SDCardTest(LPVOID lpParams, BOOL* pfStopped, HWND hWndNotify, HANDLE hEventProgressChanged, int* pnProgress)
    //{
    //    BOOL fRet = FALSE;
    //    double dProgress = 0;
    //
    //    for (i = 0; i < XX; i++)
    //    {
    //        if (*pfStopped)      
    //            goto END;
    //
    //        DoSometing();
    //        nProgress = CalculateProgress();
    //        if (nProgress != (*pnProgress))
    //        {
    //            *pnProgress = nProgress;  // Update progress.
    //            SetEvent(hEventProgressChanged);  // Notify UI. This will post a WM_TASKS_PROGRESS_CHANGED message to hWndNotify. LOWORD(wParam): current task ID. HIWORD(wParam): all tasks object ID. LOWORD(lParam) = current task progress; HIWORD(lParam) = total tasks progress.
    //        }
    //    }
    //    fRet = TRUE;
    //END:    
    //    return fRet; 
    //}

    DWORD TaskThreadStartRoutine(LPVOID lpVoid, BOOL* pfStopped);

    typedef struct _TASK_INFO {
        wstring strName;
        DWORD dwTaskId;
        DWORD dwTasksObjId;
        PFN_TASK pfnTask;
        LPVOID lpParams;
        HWND hWndNotify;
        HANDLE hEventProgressChanged;
        int nProgress;
    }TASK_INFO;

    class Tasks {
    public:
        Tasks();
        Tasks(HWND hWndNotify, LPCWSTR lpszName = NULL);
        ~Tasks();

        void Init();
        void Deinit();
        void Set(HWND hWndNotify, LPCWSTR lpszName = NULL);
        DWORD Add(PFN_TASK pfnTask, LPVOID lpParams = NULL, LPCWSTR lpszName = NULL);  // Add task. Return task's Id.
        void Clear();  // Clear tasks.
        DWORD GetId();  // Return Util::nTasksObjCount as tasks object Id.

        BOOL IsRunning();
        BOOL Start();
        BOOL Wait(DWORD dwTimeout = DEFAULT_STOP_TIMEOUT);
        BOOL Stop(DWORD dwTimeout = DEFAULT_STOP_TIMEOUT);

        DWORD m_dwTasksObjId;  // Tasks obj ID. Set this value with last Util::nTasksObjCount;
        HWND m_hWndNotify;
        wstring m_strName;
        vector<TASK_INFO*> m_taskInfos;  // Key = Task Thread Id, Value = TaskInfo*.
        Thread m_mainThread;
    };
}