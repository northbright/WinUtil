#define _WIN32_WINNT 0x0501  // InterlockedXXX will be override to InterlockedXXX64 if _WIN32_WINNT >= 0x0502

#include <windows.h>
#include <tchar.h>

#include "Debug.h"
#include "ThreadGroup.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Util;

long ThreadGroup::m_nCount = 0;

ThreadGroup::ThreadGroup()
{
    Init();
}

ThreadGroup::ThreadGroup(HWND hWndNotify)
{
    Init();
    Set(hWndNotify);
}

ThreadGroup::~ThreadGroup()
{
    Clear();
    if ((m_hEventExit) && (m_hEventExit != INVALID_HANDLE_VALUE))
        CloseHandle(m_hEventExit);
}

void ThreadGroup::Init()
{
    InterlockedIncrement(&m_nCount);
    m_nId = m_nCount - 1;
    m_fStopped = false;
    m_nProgress = 0;
    m_nRunningThreadCount = 0;
    m_hEventExit = CreateEvent(NULL, FALSE, FALSE, NULL);
}

void ThreadGroup::Set(HWND hWndNotify)
{
    m_hWndNotify = hWndNotify;
}

UINT ThreadGroup::GetId()
{
    return m_nId;
}

bool ThreadGroup::Add(PFN_THREAD_START_ROUTINE pFnThreadStartRoutine, LPVOID lpVoid, PFN_COMPUTE_TOTAL_PROGRESS pFnComputeTotalProgress)
{
    bool fRet = false;

    if (!pFnThreadStartRoutine)
        goto END;

    if (IsRunning()) {
        DBG_MSG(L"ThreadGroup::Add() failed: some thread is running.\r\n");
        goto END;
    }

    if (m_threads.size() >= MAX_THREAD_COUNT_IN_GROUP) {
        DBG_MSG(L"ThreadGroup::Add() failed: max thread count reached: %d.\r\n", m_threads.size());
        goto END;
    }

    Thread* pThread = new Thread(pFnThreadStartRoutine, lpVoid, m_hWndNotify, &m_fStopped, pFnComputeTotalProgress);
    pThread->SetGroup(this);

    m_threads.push_back(pThread);

    fRet = true;
END:
    return fRet;
}

bool ThreadGroup::Clear()
{
    bool fRet = false;
    size_t nSize = m_threads.size();

    Stop();

    for (size_t i = 0; i < nSize; i++) {
        if (m_threads[i]) {
            delete m_threads[i];
            m_threads[i] =  NULL;
        }    
    }
    m_threads.clear();
    fRet = true;
//END:
    return fRet;
}

bool ThreadGroup::Start()
{
    bool fRetAll = true;
    bool fRet = false;
    size_t nCount = m_threads.size();

    if (IsRunning())
        goto END;

    if (!nCount)
        goto END;

    m_nProgress = 0;
    m_nRunningThreadCount = 0;
    ResetEvent(m_hEventExit);
    m_fStopped = false;

    for (size_t i = 0; i < m_threads.size(); i++)
    {
        fRetAll &= m_threads[i]->Start();
        if (!fRetAll)
            goto END;

        InterlockedIncrement(&m_nRunningThreadCount);
    }

END:
    if (!fRetAll)
        Stop();

    return fRetAll;
}

void ThreadGroup::Stop()
{
    if (!IsRunning())
        return;

    m_fStopped = true;
    WaitForSingleObject(m_hEventExit, 0xFFFFFFFF);
}

bool ThreadGroup::IsRunning()
{
    return m_nRunningThreadCount == 0 ? false : true;
}