#include <windows.h>
#include <tchar.h>
#include "Shlwapi.h"

#include "Debug.h"
#include "File.h"
#include "Path.h"
#include "IStorage.h"
#include "Chm.h"

using namespace Util;

namespace Util {

    int DecomileChmThreadComputeTotalProgress(LPVOID lpParams, int nSubProgress)
    {
        int nTotalProgress = 0;
        CHM_DECOMPILE_THREAD_DATA* p = (CHM_DECOMPILE_THREAD_DATA*)lpParams;
        size_t n = p->chmDecomipleInfos.size();

        if (!n)
            return 0;

        return (p->nDone * 100 + nSubProgress) / n;
    }

    bool DecompileChmThreadStartRoutine(LPVOID lpParams, bool* pfStopped, Util::Thread* pThread)
    {
        bool fRet = false;
        CHM_DECOMPILE_THREAD_DATA* p = (CHM_DECOMPILE_THREAD_DATA*)lpParams;
        wstring wstrOutputDir;
        size_t n = 0;

        if (!p)
            goto END;

        p->nDone = 0;
        n = p->chmDecomipleInfos.size();
        for (size_t i = 0; i < n; i++) {
            
            if ((p->chmDecomipleInfos[i]->wstrChmFile.empty()) || (p->chmDecomipleInfos[i]->wstrOutputDir.empty()))
                goto END;

            wstrOutputDir = p->chmDecomipleInfos[i]->wstrOutputDir + L"\\";
            wstrOutputDir += PathFindFileName(p->chmDecomipleInfos[i]->wstrChmFile.c_str());  // Output dir = parent output dir + chm file name.

            if (!CreateDir(wstrOutputDir.c_str()))
                goto END;

            fRet = OutputStreamsInStorageFile(p->chmDecomipleInfos[i]->wstrChmFile.c_str(), wstrOutputDir.c_str(), pfStopped, pThread);
            if (!fRet) {
                if (p->pChmDecompiler->m_pLogger)
                    p->pChmDecompiler->m_pLogger->Log(L"%s ...Failed.", p->chmDecomipleInfos[i]->wstrChmFile.c_str());

                p->pChmDecompiler->m_csErrList.Enter();
                p->pChmDecompiler->m_errList.insert(p->chmDecomipleInfos[i]->wstrChmFile);
                p->pChmDecompiler->m_csErrList.Leave();
            }else {
                if (p->pChmDecompiler->m_pLogger)
                    p->pChmDecompiler->m_pLogger->Log(L"%s ...OK.", p->chmDecomipleInfos[i]->wstrChmFile.c_str());

                p->nDone++;
            }
        }

        fRet = true;
    END:
        return fRet;
    }
}

ChmDecompiler::ChmDecompiler()
{
    Init();
}

ChmDecompiler::ChmDecompiler(set<wstring>& wstrChmFileSet, LPCWSTR lpszOutputDir, HWND hWndNotify, DWORD dwThreadCount, Logger* pLogger)
{
    Init();
    Set(wstrChmFileSet, lpszOutputDir, hWndNotify, dwThreadCount, pLogger);
}

ChmDecompiler::ChmDecompiler(LPCWSTR lpszInputDir, LPCWSTR lpszOutputDir, bool fSearchSubDir, HWND hWndNotify, DWORD dwThreadCount, Logger* pLogger)
{
    Init();
    Set(lpszInputDir, lpszOutputDir, fSearchSubDir, hWndNotify, dwThreadCount, pLogger);
}

ChmDecompiler::~ChmDecompiler()
{
    Clear();
}

void ChmDecompiler::Init()
{
    m_pLogger = NULL;
}

void ChmDecompiler::Clear()
{
    size_t n = m_chmDecompileThreadDatas.size();
    m_threadGroup.Clear();

    for (size_t i = 0; i < n; i++) {
        if (m_chmDecompileThreadDatas[i]) {
            for (size_t j = 0; j < m_chmDecompileThreadDatas[i]->chmDecomipleInfos.size(); j++) {
                if (m_chmDecompileThreadDatas[i]->chmDecomipleInfos[j]) {
                    delete m_chmDecompileThreadDatas[i]->chmDecomipleInfos[j];
                    m_chmDecompileThreadDatas[i]->chmDecomipleInfos[j] = NULL;
                }
            }
            m_chmDecompileThreadDatas[i]->chmDecomipleInfos.clear();
            delete m_chmDecompileThreadDatas[i];
            m_chmDecompileThreadDatas[i] = NULL;
        }
    }
    m_chmDecompileThreadDatas.clear();
    m_errList.clear();
    m_pLogger = NULL;
}

void ChmDecompiler::Set(set<wstring>& wstrChmFileSet, LPCWSTR lpszOutputDir, HWND hWndNotify, DWORD dwThreadCount, Logger* pLogger)
{
    set<wstring>::iterator it;
    size_t i = 0;
    size_t n = wstrChmFileSet.size();
    
    if ((!n) || (!lpszOutputDir) || (!wcslen(lpszOutputDir)))
        return;

    if ((dwThreadCount <=0) || (dwThreadCount > MAX_DECOMPILE_THREAD_COUNT))
        m_dwThreadCount = 1;
    else
        m_dwThreadCount = min(n, dwThreadCount);

    Clear();

    m_threadGroup.Set(hWndNotify);

    for (size_t i = 0; i < m_dwThreadCount; i++) {
        CHM_DECOMPILE_THREAD_DATA* pData = new CHM_DECOMPILE_THREAD_DATA;
        pData->pChmDecompiler = this;
        m_chmDecompileThreadDatas.push_back(pData);
        m_threadGroup.Add(DecompileChmThreadStartRoutine, (LPVOID)pData, DecomileChmThreadComputeTotalProgress);
    }

    for (it = wstrChmFileSet.begin(); it != wstrChmFileSet.end(); it++)
    { 
        CHM_DECOMPILE_INFO* pInfo = new CHM_DECOMPILE_INFO();
        pInfo->wstrChmFile = it->c_str();
        pInfo->wstrOutputDir = lpszOutputDir;

        m_chmDecompileThreadDatas[i % m_dwThreadCount]->chmDecomipleInfos.push_back(pInfo);
        i++;
    }

    m_pLogger = pLogger;
}

void ChmDecompiler::Set(LPCWSTR lpszInputDir, LPCWSTR lpszOutputDir, bool fSearchSubDir, HWND hWndNotify, DWORD dwThreadCount, Logger* pLogger)
{
    set<wstring>::iterator it;
    size_t i = 0;
    set<wstring> wstrChmFileSet;
    
    SearchFilesByExtName(lpszInputDir, L"*.chm", wstrChmFileSet, fSearchSubDir);
    if (wstrChmFileSet.empty())
        return;

    Set(wstrChmFileSet, lpszOutputDir, hWndNotify, dwThreadCount, pLogger);
}

bool ChmDecompiler::Start()
{
    m_errList.clear();
    return m_threadGroup.Start();
}

void ChmDecompiler::Stop()
{
    m_threadGroup.Stop();
}

bool ChmDecompiler::IsRunning()
{
    return m_threadGroup.IsRunning();
}