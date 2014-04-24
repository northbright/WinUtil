
//#include <atlbase.h>
//#include <atlutil.h>

#include <tchar.h>
#include <Shlwapi.h>
#include "Path.h"
#include "IStorage.h"

using namespace Util;

namespace Util {

    static const GUID CLSID_ITStorage = { 0x5d02926a, 0x212e, 0x11d0, { 0x9d, 0xf9, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec } };
    static const GUID IID_ITStorage = { 0x88cc31de, 0x27ab, 0x11d0, { 0x9d, 0xf9, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xec} };

    UINT GetStreamCount(IStorage* pStg)
    {
        bool fRet = false;
        HRESULT hr = 0;
        UINT nCount = 0;
        UINT n = 0;
        IEnumSTATSTG* pEnum = NULL;
        STATSTG statStg = {0};
        IStorage* pSubStg = NULL;

        if (!pStg)
            goto END;
  
        pStg->EnumElements(0, NULL, 0, &pEnum);

        while (pEnum->Next(1, &statStg, NULL) == NOERROR)
        {
            if (statStg.type == STGTY_STORAGE)
            {
                hr = pStg->OpenStorage(statStg.pwcsName, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, 0, &pSubStg);
                if (SUCCEEDED(hr))
                {
                    n = GetStreamCount(pSubStg);  // Get Stream Count in Sub Storage
                    nCount += n;
                }

                if (pSubStg)
                {
                    pSubStg->Release();
                }    
            }
            else if (statStg.type == STGTY_STREAM)
            {
                nCount++;
            }

            CoTaskMemFree(statStg.pwcsName);
        }

    END:
        if (pEnum)
            pEnum->Release();

        return nCount;
    }


    UINT GetFileCountInStorageFile(LPCTSTR ptszFileName)
    {
        bool fRet = false;
        HRESULT hr = 0;
        LRESULT lResult = 0;
        IITStorage* pITStorage = NULL;
        IStorage* pStg = NULL;
        UINT nCount = 0;

        if (!ptszFileName)
        {
            fRet = false;
            goto END;
        }    

        hr = CoInitialize(NULL);
        if (FAILED(hr))
        {
            fRet = false;
            goto END;
        }    

        hr = CoCreateInstance(CLSID_ITStorage, NULL, CLSCTX_INPROC_SERVER, IID_ITStorage,(void **)&pITStorage);
        if (hr != S_OK)
        {
            fRet = false;
            goto END;
        }

        hr = pITStorage->StgOpenStorage(ptszFileName, NULL, STGM_READ | STGM_SHARE_DENY_WRITE, NULL, 0, &pStg);
        if (hr != S_OK)
        {
            fRet = false;
            goto END;
        }

        nCount = GetStreamCount(pStg);

    END:
        if (pStg)
            pStg->Release();

        if (pITStorage)
            pITStorage->Release();

        CoUninitialize();
        return nCount;
    }  


    bool _OutputStreamsInStorage(IStorage* pStg, LPCWSTR szPath, UINT nTotalStream, UINT & nOutputedStream, bool* pfStopped, Thread* pThread)
    {
        bool fRet = false;
        HRESULT hr = 0;
        IEnumSTATSTG* pEnum = NULL;
        STATSTG statStg = {0};
        IStorage* pSubStg = NULL;
        IStream* pIStream = NULL;
        wstring wstrCurrentPath;
        wstring wstrStreamFullName;
        BYTE* pBuffer = NULL;
        ULONG ulReadBytes = 0;
        HANDLE hFile = NULL;
        ULONG ulSize = 0;
        DWORD dwWriteBytes = 0;
        UINT nPercent = 0;
        UINT nOldPercent = 0;

        if (!pStg)
        {
            fRet = false;
            goto END;
        }    

        hr = pStg->EnumElements(0, NULL, 0, &pEnum);
        if (FAILED(hr))
        {
            fRet = false;
            goto END;
        }    

        while (pEnum->Next(1, &statStg, NULL) == NOERROR)
        {
            if (pfStopped)
            {
                if (*pfStopped)
                {
                    fRet = false;
                    goto END;
                }
            }        

			wstrCurrentPath = szPath;
			wstrCurrentPath += _T("\\");
			wstrStreamFullName.empty();

            if (statStg.type == STGTY_STORAGE)
            {
                hr = pStg->OpenStorage(statStg.pwcsName, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, 0, &pSubStg);
                if (FAILED(hr))
                {
                    fRet = false;
                    goto END;
                }    

                wstrCurrentPath += statStg.pwcsName;

                if (!PathFileExists(wstrCurrentPath.c_str()))
                {
                    if (!Util::CreateDir(wstrCurrentPath.c_str()))
                    {
                        fRet = false;
                        goto END;
                    }  
                }

                fRet = _OutputStreamsInStorage(pSubStg, wstrCurrentPath.c_str(), nTotalStream, nOutputedStream, pfStopped, pThread);
                if (!fRet)
                {
                    fRet = false;
                    goto END;
                }    

                if (pSubStg)
                {
                    pSubStg->Release();
                    pSubStg = NULL;
                }    
            }
            else if (statStg.type == STGTY_STREAM)
            {
                wstrStreamFullName = wstrCurrentPath;
                wstrStreamFullName += statStg.pwcsName;

                hr = pStg->OpenStream(statStg.pwcsName, NULL, 0, 0, &pIStream);
                if (FAILED(hr))
                {
                    fRet = false;
                    goto END;
                }    

                ulSize = statStg.cbSize.LowPart;
                pBuffer =(BYTE *)malloc(ulSize);
                if (!pBuffer)
                {
                    fRet = false;
                    goto END;
                }    

                hr = pIStream->Read(pBuffer, ulSize, &ulReadBytes);
                if (FAILED(hr))
                {
                    fRet = false;
                    goto END;
                }        

                hFile = CreateFile(wstrStreamFullName.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    fRet = false;
                    goto END;
                }    

                fRet = WriteFile(hFile, pBuffer, ulSize, &dwWriteBytes, NULL);
                if (!fRet)
                {
                    fRet = false;
                    goto END;
                }    

                CloseHandle(hFile);
                hFile = NULL;

                if (pBuffer)
                {    
                    free(pBuffer);
                    pBuffer = NULL;
                }

                if (pIStream)
                {
                    pIStream->Release();
                    pIStream = NULL;
                }

                nOutputedStream++;

                // -------- Update Progress -----------
                if (pThread)
                {
                    if (nTotalStream != 0)
                    {
                        nOldPercent = nPercent;
                        nPercent =(int)((nOutputedStream)* 100 / nTotalStream);
                    }    

                    if (nPercent != nOldPercent)
                    {
                        pThread->UpdateProgress(nPercent);
                    }
                }    
            }

            CoTaskMemFree(statStg.pwcsName);
        }

    END:
        if (pEnum)
        {
            pEnum->Release();
            pEnum = NULL;
        }

        if ((hFile) &&(hFile != INVALID_HANDLE_VALUE))
            CloseHandle(hFile);

        return fRet;
    }


    bool OutputStreamsInStorageFile(LPCTSTR ptszFile, LPCTSTR ptszPath, bool* pfStopped, Thread* pThread)
    {
        bool fRet = false;
        HRESULT hr = 0;
        LRESULT lResult = 0;
        IITStorage* pITStorage = NULL;
        IStorage* pStg = NULL;
        UINT nTotalStream = 0;
        UINT nOutputedStream = 0;

        if (!ptszFile)
        {
            fRet = false;
            goto END;
        }    

        hr = CoInitialize(NULL);
        if (FAILED(hr))
        {
            fRet = false;
            goto END;
        }    

        hr = CoCreateInstance(CLSID_ITStorage, NULL, CLSCTX_INPROC_SERVER, IID_ITStorage,(void **)&pITStorage);
        if (hr != S_OK)
        {
            fRet = false;
            goto END;
        }

        hr = pITStorage->StgOpenStorage(ptszFile, NULL, STGM_READ | STGM_SHARE_DENY_WRITE, NULL, 0, &pStg);
        if (hr != S_OK)
        {
            fRet = false;
            goto END;
        }    

        nTotalStream = GetStreamCount(pStg);
        nOutputedStream = 0;

        fRet = _OutputStreamsInStorage(pStg, ptszPath, nTotalStream, nOutputedStream, pfStopped, pThread);

    END:
        if (pStg)
            pStg->Release();

        if (pITStorage)
            pITStorage->Release();

        CoUninitialize();
        return fRet;
    }

}