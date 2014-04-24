#include <Windows.h>
#include <tchar.h>
#include <Shlwapi.h>  // PathFindFileName

#include "Debug.h"
#include "Bluetooth.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Advapi32.lib")  // RegOpenKeyEx()...

namespace Util {

    bool GetBluetoothSupportDllPath(wstring& strDllPath)
    {
        bool fRet = false;
        HKEY hKey = NULL;
        LONG lRet = 0;
        DWORD dwSize = 0;
        WCHAR* pBuffer = NULL;
        LPWSTR pDllPath = NULL;

        strDllPath.clear();

        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, BTH_RADIO_SUPPORT, 0, KEY_READ, &hKey);
        if (lRet != ERROR_SUCCESS)
            goto END;

        lRet = RegQueryValueEx(hKey, BTH_SUPPORT_DLL, NULL, NULL, (BYTE*)pBuffer, &dwSize);
        if (!dwSize)
            goto END;

        pBuffer = new WCHAR[dwSize / 2];
        memset(pBuffer, 0, sizeof(WCHAR) * (dwSize / 2));

        lRet = RegQueryValueEx(hKey, BTH_SUPPORT_DLL, NULL, NULL, (BYTE*)pBuffer, &dwSize);
        if (lRet != ERROR_SUCCESS)
            goto END;

        pDllPath = PathFindFileName(pBuffer);
        if (!pDllPath)
            goto END;

        strDllPath = pDllPath;
        fRet = true;

    END:
        if (pBuffer)
            delete[] pBuffer;

        if (hKey)
            RegCloseKey(hKey);

        return fRet;
    }

    bool SetBluetoothPowerOn(bool fOn)
    {
        bool fRet = false;
        HMODULE hDll = NULL;
        wstring strDllPath;
        PFN_BLUETOOTH_ENABLE_RADIO BluetoothEnableRadio = NULL;
        DWORD dwRet = 0;

        if (!GetBluetoothSupportDllPath(strDllPath))
            goto END;

        hDll = LoadLibrary(strDllPath.c_str());
        if (!hDll)
            goto END;

        BluetoothEnableRadio = (PFN_BLUETOOTH_ENABLE_RADIO)GetProcAddress(hDll, "BluetoothEnableRadio");
        if (!BluetoothEnableRadio)
            goto END;

        dwRet = BluetoothEnableRadio(fOn);
        if (dwRet != ERROR_SUCCESS)
            goto END;

        fRet = true;

    END:
        if (hDll)
            FreeLibrary(hDll);

        return fRet;
    }

    bool GetBluetoothPowerState(bool& fOn)
    {
        bool fRet = false;
        HMODULE hDll = NULL;
        wstring strDllPath;
        PFN_IS_BLUETOOTH_RADIO_ENABLED IsBluetoothRadioEnabled = NULL;
        DWORD dwRet = 0;

        if (!GetBluetoothSupportDllPath(strDllPath))
            goto END;

        hDll = LoadLibrary(strDllPath.c_str());
        if (!hDll)
            goto END;

        IsBluetoothRadioEnabled = (PFN_IS_BLUETOOTH_RADIO_ENABLED)GetProcAddress(hDll, "IsBluetoothRadioEnabled");
        if (!IsBluetoothRadioEnabled)
            goto END;

        dwRet = IsBluetoothRadioEnabled(&fOn);
        if (dwRet != ERROR_SUCCESS)
            goto END;

        fRet = true;
    END:
        if (hDll)
            FreeLibrary(hDll);

        return fRet;
    }
}