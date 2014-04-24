
#include <windows.h>
#include <tchar.h>

#include "Debug.h"
#include "Proxy.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Util {

    bool IsWindowsProxyEnabled()
    {
        bool fRet = false;
        DWORD dwRet = 0;
        DWORD dwSize = sizeof(DWORD);
        DWORD dwType = REG_DWORD;
        HRESULT hr = ERROR_SUCCESS;
        HKEY hKey = NULL;

        hr = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), 0, KEY_QUERY_VALUE, &hKey);
        if (hr != ERROR_SUCCESS)
            goto END;

        hr = RegQueryValueEx (hKey, _T("ProxyEnable"), NULL, &dwType, (LPBYTE)&dwRet, &dwSize);
        if (hr != ERROR_SUCCESS)
            goto END;

        fRet = (bool)dwRet;
    END:
        if (hKey)
            RegCloseKey(hKey);

        return fRet;
    }

    bool GetWindowsProxySetting(wstring& strProxySetting)
    {
        bool fRet = false;
        WCHAR szBuffer[512] = {0};
        DWORD dwSize = sizeof(WCHAR) * 512;
        DWORD dwType = REG_SZ;
        HRESULT hr = ERROR_SUCCESS;
        HKEY hKey = NULL;

        strProxySetting.clear();

        hr = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), 0, KEY_QUERY_VALUE, &hKey);
        if (hr != ERROR_SUCCESS)
            goto END;

        hr = RegQueryValueEx (hKey, _T("ProxyServer"), NULL, &dwType, (LPBYTE)szBuffer, &dwSize);
        if (hr != ERROR_SUCCESS)
            goto END;

        strProxySetting = szBuffer;
        if (!strProxySetting.length())
            goto END;

        // -------- Done. ----------    
        fRet = true;    

    END:
        if (hKey)
            RegCloseKey(hKey);

        return fRet;
    }

    /*
    bool GetWindowsProxySetting(wstring& strProxyType, wstring& strHost, unsigned long& nPort)
    {
    bool fRet = false;
    wstring strProxySetting;
    wstring strTemp;
    vector<wstring> stringArray;

    fRet = GetWindowsProxySetting(strProxySetting);
    if (!fRet)
    goto END;

    // http://web-proxy.atl.hp.com:8088 -> "http", "//web-proxy.atl.hp.com", "8088"

    // ------- Check protocal string "://" ----------
    if (find_first(strProxySetting, _T("://")))
    {
    split_regex(stringArray, strProxySetting, regex("://"));
    strProxyType = stringArray[0];
    strTemp = stringArray[1];
    }
    else
    {
    strProxyType = _T("http");  // default
    strTemp = strProxySetting;
    }    

    // ------- Check port string ":" ----------
    if (find_first(strTemp, _T(":")))
    {
    stringArray.clear();
    split(stringArray, strTemp, is_any_of(_T(":")));
    strHost = stringArray[0];
    nPort = (unsigned long)_wtoi(stringArray[1].c_str());
    }
    else
    {
    strHost = strTemp;
    nPort = 80;  // default
    }

    // --------- Done. ------------
    fRet = true;

    END:    
    return fRet;
    }
    */

}