
#include <windows.h>
#include <tchar.h>

#include "Util.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Util {

    bool SetIcon(HWND hWnd, HICON hIcon, HICON hIconSmall)
    {
        bool fRet = false;

        if (!hWnd)
            goto END;

        if (hIcon)
            SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

        if (hIconSmall)
            SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

        // ------ Done. --------
        fRet = true;    

    END:
        return fRet;
    }

    bool OpenSelectFolderDialog (HWND hwndOwner, LPCWSTR lpszTitle, wstring& wstrPath)
    {
        bool fRet = false;
        HRESULT hr = 0;
        BROWSEINFO bi = {0};
        WCHAR szPath[MAX_PATH + 1] = {0};
        LPITEMIDLIST pidl = NULL;
        IMalloc * pIMalloc = NULL;

        bi.hwndOwner = hwndOwner;
        bi.lpszTitle = lpszTitle;

        pidl = SHBrowseForFolder(&bi);
        if (pidl == NULL)
            goto END;

        fRet = SHGetPathFromIDList(pidl, szPath);
        if (!fRet)
            goto END;    

        wstrPath = szPath;

        hr = SHGetMalloc(&pIMalloc);
        if (FAILED (hr))
            goto END;

        fRet = true;
    END:
        if (pIMalloc)
        {    
            pIMalloc->Free(pidl);
            pidl = NULL;
            pIMalloc->Release();
        }

        return fRet;
    }

    void MyGetWindowText(HWND hWnd, wstring& wstr) {
        int nLen = GetWindowTextLength(hWnd);
        wchar_t* p = NULL;

        if (!nLen)
            return;

        p = new wchar_t[nLen + 1];
        memset(p, 0, sizeof(wchar_t) * (nLen + 1));
        GetWindowText(hWnd, p, nLen + 1);
        wstr = p;

        if (p)
            delete p;
    }

}
