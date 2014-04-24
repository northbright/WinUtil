#define _WIN32_WINNT 0x0501  // InterlockedXXX will be override to InterlockedXXX64 if _WIN32_WINNT >= 0x0502

#include <windows.h>
#include <tchar.h>

#include "Debug.h"
#include "Thread.h"
#include "ThreadGroup.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Util;

namespace Util {

    DWORD WINAPI ThreadStartRoutine(LPVOID lpVoid)
    {
        Thread* pThread = (Thread*)lpVoid;

        return pThread->m_pFnThreadStartRoutine(pThread->m_lpVoid, pThread->m_pfStopped, pThread);     
    }

    DWORD WINAPI MonitorThreadStartRoutine(LPVOID lpVoid)
    {
        DWORD dwExitCode = 0;
        Thread* pThread = (Thread*)lpVoid;
        
        WaitForSingleObject(pThread->m_hThread, 0xFFFFFFFF);
        
        GetExitCodeThread(pThread->m_hThread, &dwExitCode);
        pThread->OnExit(dwExitCode);
        SetEvent(pThread->m_hEventExit);

        return dwExitCode;
    }

}

Thread::Thread()
{
    Init();
}

Thread::Thread(PFN_THREAD_START_ROUTINE pFnThreadStartRoutine, LPVOID lpVoid, HWND hWndNotify, bool* pfStopped, PFN_COMPUTE_TOTAL_PROGRESS pFnComputeTotalProgress)
{
    Init();
    Set(pFnThreadStartRoutine, lpVoid, hWndNotify, pfStopped, pFnComputeTotalProgress);
}

Thread::~Thread()
{
    Stop();

    WaitForSingleObject(m_hMonitorThread, 0xFFFFFFFF);

    if ((m_hEventExit) && (m_hEventExit != INVALID_HANDLE_VALUE))
        CloseHandle(m_hEventExit);
    
    if ((m_hEventControl) && (m_hEventControl != INVALID_HANDLE_VALUE))
        CloseHandle(m_hEventControl);

    if ((m_hThread) && (m_hThread != INVALID_HANDLE_VALUE))
        CloseHandle(m_hThread);

    if ((m_hMonitorThread) && (m_hMonitorThread != INVALID_HANDLE_VALUE))
        CloseHandle(m_hMonitorThread);
}

void Thread::Init()
{
    m_pFnThreadStartRoutine = NULL;
    m_lpVoid = NULL;
    m_hWndNotify = NULL;

    m_fRunning = false;
    m_fStopped = false;
    m_hThread = NULL;
    m_hMonitorThread = NULL;
    m_dwThreadId = 0;
    m_nProgress = 0;

    m_pThreadGroup = NULL;
    m_hEventExit = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hEventControl = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_pFnComputeTotalProgress = NULL;
    
}

void Thread::Set(PFN_THREAD_START_ROUTINE pFnThreadStartRoutine, LPVOID lpVoid, HWND hWndNotify, bool* pfStopped, PFN_COMPUTE_TOTAL_PROGRESS pFnComputeTotalProgress)
{
    m_pFnThreadStartRoutine = pFnThreadStartRoutine;
    m_lpVoid = lpVoid;
    m_hWndNotify = hWndNotify;
    if (!pfStopped)
        m_pfStopped = &m_fStopped;
    else
        m_pfStopped = pfStopped;

    m_pFnComputeTotalProgress = pFnComputeTotalProgress;
}

void Thread::SetNotifyWindow(HWND hWndNotify)
{
    m_hWndNotify = hWndNotify;
}

void Thread::SetGroup(ThreadGroup* pThreadGroup)
{
    m_pThreadGroup = pThreadGroup;
}

void Thread::SetStoppedFlag(bool* pfStopped)
{
    if (!pfStopped)
        m_pfStopped = pfStopped;
    else
        m_pfStopped = &m_fStopped;
}


bool Thread::IsValid()
{
    return (m_pFnThreadStartRoutine) ? true : false;
}

void Thread::SetComputerTotalProgressFunc(PFN_COMPUTE_TOTAL_PROGRESS pFunc)
{
    if (pFunc)
        m_pFnComputeTotalProgress = pFunc;
}

void Thread::UpdateProgress(int nProgress)
{
    int p = 0;
    if (!m_pFnComputeTotalProgress)
        p = nProgress;
    else
        p = m_pFnComputeTotalProgress(m_lpVoid, nProgress);  // computer total progress if user register a customized func.

    if (m_nProgress != p)
    {
        m_nProgress = p;

        if (m_hWndNotify)
        {
            PostMessage(m_hWndNotify, WM_THREAD_PROGRESS_CHANGED, (WPARAM)m_dwThreadId, (LPARAM)m_nProgress);

            // Compute thread group progress and notify.
            if (m_pThreadGroup)
            {
                size_t nGroupProgress = 0;
                size_t nCount = m_pThreadGroup->m_threads.size();
                
                if (nCount)
                {
                    size_t nSize = m_pThreadGroup->m_threads.size();
                    for (size_t i = 0; i < nSize; i++) {
                        nGroupProgress += m_pThreadGroup->m_threads[i]->m_nProgress;
                    }

                    nGroupProgress /= nCount;

                    if ((int)nGroupProgress != m_pThreadGroup->m_nProgress)
                    {
                        m_pThreadGroup->m_nProgress = (int)nGroupProgress;
                        PostMessage(m_hWndNotify, WM_THREAD_GROUP_PROGRESS_CHANGED, (WPARAM)m_pThreadGroup->m_nId, (LPARAM)((int)nGroupProgress));
                    }
                }
            }
        }
    }
}

void Thread::OnExit(DWORD dwExitCode)
{
    // Notify UI thread exits.
    if (*m_pfStopped)
    {
        if (m_hWndNotify)
        {
            PostMessage(m_hWndNotify, WM_THREAD_STOPPED, (WPARAM)m_dwThreadId, (LPARAM)0);

            // If thread is in group, notify if all threads in group stopped.
            if (m_pThreadGroup)
            {
                InterlockedDecrement(&(m_pThreadGroup->m_nRunningThreadCount));
                if (!(m_pThreadGroup->IsRunning())) {
                    PostMessage(m_hWndNotify, WM_THREAD_GROUP_STOPPED, (WPARAM)m_pThreadGroup->m_nId, (LPARAM)0);
                    SetEvent(m_pThreadGroup->m_hEventExit);
                }
            }
        }
    }
    else
    {
        if (m_hWndNotify)
        {
            PostMessage(m_hWndNotify, dwExitCode ? WM_THREAD_SUCCEEDED : WM_THREAD_FAILED, (WPARAM)m_dwThreadId, (LPARAM)0);

            // If thread is in group, notify if all threads in group exited.
            if (m_pThreadGroup)
            {
                InterlockedDecrement(&(m_pThreadGroup->m_nRunningThreadCount));
                if (!(m_pThreadGroup->IsRunning())) {
                    PostMessage(m_hWndNotify, WM_THREAD_GROUP_EXITED, (WPARAM)m_pThreadGroup->m_nId, (LPARAM)0);
                    SetEvent(m_pThreadGroup->m_hEventExit);
                }
            }
        }
    }
}

bool Thread::IsRunning()
{
    DWORD dwExitCode = 0;
    
    if (m_hThread)
    {
        if (!GetExitCodeThread(m_hThread, &dwExitCode))
        {
            if (GetLastError() == STILL_ACTIVE)
            {
                return true;
            }
        }
    }

    return false;
}

bool Thread::Start(DWORD* pdwThreadId)
{
    bool fRet = false;
    DWORD dwExitCode = 0;

    if (!IsValid())
        goto END;

    if (IsRunning()) {
        DBG_MSG(L"Thread::Start(): already running.\r\n");
        goto END;
    }

    if ((m_hThread) && (m_hThread != INVALID_HANDLE_VALUE)) {

        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    m_fRunning = true;
    *m_pfStopped = false;
    m_nProgress = 0;
    ResetEvent(m_hEventExit);  // set m_hEvent to nosignal state.

    m_hThread = CreateThread(NULL, 0, ThreadStartRoutine, this, 0, &m_dwThreadId);
    if (!m_hThread)
        goto END;

    m_hMonitorThread = CreateThread(NULL, 0, MonitorThreadStartRoutine, this, 0, &m_dwThreadId);
    if (!m_hMonitorThread)
        goto END;

    if (pdwThreadId)
        *pdwThreadId = m_dwThreadId;

    fRet = true;
END:
    return fRet;
}

void Thread::Stop()  // in ms
{
    if (!IsRunning())
        goto END;

    *m_pfStopped = true;

    WaitForSingleObject(m_hEventExit, 0xFFFFFFFF);

END:
    return;
}

void Thread::StopAsync()
{
    if (IsRunning())
        *m_pfStopped = true;
}