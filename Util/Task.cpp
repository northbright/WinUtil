#include <Windows.h>
#include <tchar.h>

#include "Util.h"

namespace Util {

    // Tasks::GetId() will return Util::nTaskObjCount as task Id.
    UINT nTasksObjCount;

    DWORD TaskThreadStartRoutine(LPVOID lpVoid, BOOL* pfStopped)
    {
        DWORD dwRet = 0;
        TASK_INFO* pTaskInfo = (TASK_INFO*)lpVoid;
        int nMsg = 0;
        
        if (pTaskInfo->pfnTask)
        {
            dwRet = pTaskInfo->pfnTask(pTaskInfo->lpParams, pfStopped, pTaskInfo->hWndNotify, pTaskInfo->hEventProgressChanged, &(pTaskInfo->nProgress));
            nMsg = dwRet ? WM_TASK_SUCCEEDED : WM_TASK_FAILED;
            PostMessage(pTaskInfo->hWndNotify, nMsg, (WPARAM)(MAKELONG(pTaskInfo->dwTaskId, pTaskInfo->dwTasksObjId)), 0);
        }

        return dwRet;
    }

    DWORD MainThreadStartRoutine(LPVOID lpVoid, BOOL* pfStopped)
    {
        DWORD dwRet = 0;
        Tasks* pTasks = (Tasks*)lpVoid;
        Threads threads;
        size_t nCount = pTasks->m_taskInfos.size();
        HANDLE* pEventProgressChangedArray = NULL;
        DWORD dwWait = 0;
        DWORD dwTaskId = 0;
        int nTotalProgress = 0;

        if (!nCount)
            goto END;

        pEventProgressChangedArray = new HANDLE[nCount];

        threads.Clear();
        for (size_t i = 0; i < nCount; i++)
        {
            threads.Add(TaskThreadStartRoutine, pTasks->m_taskInfos[i], pTasks->m_taskInfos[i]->strName.c_str());
            pEventProgressChangedArray[i] = pTasks->m_taskInfos[i]->hEventProgressChanged;  // Copy progress changed event.
        }

        threads.StartAll();

        while (TRUE)
        {
            if (*pfStopped)
            {
                threads.StopAll(1000);
                PostMessage(pTasks->m_hWndNotify, WM_TASKS_STOPPED, (WPARAM)(MAKELONG(0, pTasks->m_dwTasksObjId)), 0);
                break;
            }
            else
            {
                if (threads.WaitAll(100))
                {
                    map<DWORD, ThreadInfo*>::iterator it;
                    DWORD dwExitCodeAll = 1;

                    for (it = threads.m_threadInfos.begin(); it != threads.m_threadInfos.end(); it++)
                    {
                        dwExitCodeAll &= it->second->m_dwExitCode;
                    }

                    if (dwExitCodeAll)
                        PostMessage(pTasks->m_hWndNotify, WM_TASKS_SUCCEEDED, (WPARAM)(MAKELONG(0, pTasks->m_dwTasksObjId)), 0);
                    else
                        PostMessage(pTasks->m_hWndNotify, WM_TASKS_FAILED, (WPARAM)(MAKELONG(0, pTasks->m_dwTasksObjId)), 0);

                    break;
                }
                else
                {
                    dwWait = WaitForMultipleObjects(nCount, pEventProgressChangedArray, FALSE, 100);
                    if ((dwWait >= WAIT_OBJECT_0 + 0) && (dwWait <= WAIT_OBJECT_0 + nCount - 1))
                    {
                        dwTaskId = dwWait - WAIT_OBJECT_0;
                        nTotalProgress = 0;

                        for (size_t i = 0; i < nCount; i++)
                        {
                            nTotalProgress += pTasks->m_taskInfos[i]->nProgress;
                        }

                        nTotalProgress /= nCount;
                        PostMessage(pTasks->m_hWndNotify, WM_TASK_PROGRESS_CHANGED, (WPARAM)(MAKELONG(dwTaskId, pTasks->m_dwTasksObjId)), (LPARAM)(MAKELONG(pTasks->m_taskInfos[dwTaskId]->nProgress, nTotalProgress)));
                    }
                }
            }
        }
    
    END:
        if (pEventProgressChangedArray)
        {
            delete[] pEventProgressChangedArray;
            pEventProgressChangedArray = NULL;
        }

        return dwRet;
    }

    // Tasks
    Tasks::Tasks()
    {
        Init();
    }

    Tasks::Tasks(HWND hWndNotify, LPCWSTR lpszName)
    {
        Init();
        Set(hWndNotify, lpszName);
    }

    Tasks::~Tasks()
    {
        Clear();
        Deinit();
    }

    void Tasks::Init()
    {
        m_dwTasksObjId = Util::nTasksObjCount;  // Set tasks Object Id with global tasks object count.
        InterlockedIncrement(&(Util::nTasksObjCount)); // Increase global tasks object count.
        m_hWndNotify = NULL;
        m_mainThread.Set(MainThreadStartRoutine, this, L"TaskMainThread");
    }

    void Tasks::Deinit()
    {
        m_mainThread.Clear();
    }

    void Tasks::Set(HWND hWndNotify, LPCWSTR lpszName)
    {
        m_hWndNotify = hWndNotify;

        if (lpszName)
            m_strName = lpszName;
    }

    DWORD Tasks::Add(PFN_TASK pfnTask, LPVOID lpParams, LPCWSTR lpszName)
    {
        TASK_INFO* pTaskInfo = new TASK_INFO();

        if (lpszName)
            pTaskInfo->strName = lpszName;

        pTaskInfo->dwTaskId = m_taskInfos.size();        
        pTaskInfo->dwTasksObjId = m_dwTasksObjId;
        pTaskInfo->pfnTask = pfnTask;
        pTaskInfo->lpParams = lpParams;
        pTaskInfo->hWndNotify = m_hWndNotify;
        pTaskInfo->hEventProgressChanged = CreateEvent(NULL, FALSE, FALSE, NULL);
        pTaskInfo->nProgress = 0;

        m_taskInfos.push_back(pTaskInfo);

        return pTaskInfo->dwTaskId;
    }

    void Tasks::Clear()
    {
        if (m_mainThread.IsRunning())
            m_mainThread.Stop();

        for (size_t i = 0; i < m_taskInfos.size(); i++)
        {
            if (m_taskInfos[i])
            {
                if (m_taskInfos[i]->hEventProgressChanged)
                    CloseHandle(m_taskInfos[i]->hEventProgressChanged);

                delete m_taskInfos[i];
            }
        }

        m_taskInfos.clear();
    }

    DWORD Tasks::GetId()
    {
        return m_dwTasksObjId;
    }

    BOOL Tasks::IsRunning()
    {
        return m_mainThread.IsRunning();
    }

    BOOL Tasks::Start()
    {
        BOOL fRet = FALSE;

        if (!m_taskInfos.size())
            goto END;

        fRet = m_mainThread.Start();
        DBG_MSG(L"Task: %s Start()\r\n", m_strName.c_str());
    END:
        return fRet; 
    }
    
    BOOL Tasks::Wait(DWORD dwTimeout)
    {
        DBG_MSG(L"Task: %s Wait()\r\n", m_strName.c_str());
        return m_mainThread.Wait(dwTimeout);
    }
    
    BOOL Tasks::Stop(DWORD dwTimeout)
    {
        DBG_MSG(L"Task: %s Stop()\r\n", m_strName.c_str());
        return m_mainThread.Stop(dwTimeout);
    }
}
