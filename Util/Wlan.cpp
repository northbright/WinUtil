
#include <Windows.h>
#include <tchar.h>
#include <wlanapi.h>  // Native Wlan APIs

#include "Util.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Wlanapi.lib")  // Native Wlan APIs

namespace Util {

    bool SetWlanPowerOn(bool fOn)
    {
        bool fRet = false;
        HANDLE hWlan = NULL;
        DWORD dwRet = 0;
        DWORD dwVer = 0;
        PWLAN_INTERFACE_INFO_LIST pList = NULL;
        PWLAN_INTERFACE_INFO pInfo = NULL;
        WLAN_PHY_RADIO_STATE state = {0};

        dwRet = WlanOpenHandle(2, NULL, &dwVer, &hWlan);
        if (dwRet != ERROR_SUCCESS)
            goto END;

        dwRet = WlanEnumInterfaces(hWlan, NULL, &pList);
        if (dwRet != ERROR_SUCCESS)
            goto END;

        if (pList->dwNumberOfItems != 1)  // Support 1 Wlan interface only.
            goto END;

        pInfo = &(pList->InterfaceInfo[0]);

        if (fOn)
            state.dot11SoftwareRadioState = dot11_radio_state_on;  // dot11_radio_state_on = 1, dot11_radio_state_off = 2;
        else
            state.dot11SoftwareRadioState = dot11_radio_state_off;  // dot11_radio_state_on = 1, dot11_radio_state_off = 2;

        dwRet = WlanSetInterface(hWlan, &(pInfo->InterfaceGuid), wlan_intf_opcode_radio_state, sizeof(state), &state, NULL);
        if (dwRet != ERROR_SUCCESS)
            goto END;

        fRet = true;
    END:
        if (pList)
            WlanFreeMemory(pList);

        if (hWlan)
            WlanCloseHandle(hWlan, NULL);

        return fRet;
    }

    bool GetWlanPowerState(bool& fOn)
    {
        bool fRet = false;
        HANDLE hWlan = NULL;
        DWORD dwRet = 0;
        DWORD dwVer = 0;
        PWLAN_INTERFACE_INFO_LIST pList = NULL;
        PWLAN_INTERFACE_INFO pInfo = NULL;
        WLAN_PHY_RADIO_STATE* pState = NULL;
        DWORD dwSize = sizeof(WLAN_PHY_RADIO_STATE);

        dwRet = WlanOpenHandle(2, NULL, &dwVer, &hWlan);
        if (dwRet != ERROR_SUCCESS)
            goto END;

        dwRet = WlanEnumInterfaces(hWlan, NULL, &pList);
        if (dwRet != ERROR_SUCCESS)
            goto END;

        if (pList->dwNumberOfItems != 1)  // Support 1 Wlan interface only.
            goto END;

        pInfo = &(pList->InterfaceInfo[0]);

        dwRet = WlanQueryInterface(hWlan, &(pInfo->InterfaceGuid), wlan_intf_opcode_radio_state, NULL, &dwSize, (PVOID*)&pState, NULL);
        if (dwRet != ERROR_SUCCESS)
            goto END;

        if (pState->dot11HardwareRadioState == dot11_radio_state_on)
            fOn = true;
        else
            fOn = false;

        fRet = true;
    END:
        if (pState)
            WlanFreeMemory(pState);

        if (pList)
            WlanFreeMemory(pList);

        if (hWlan)
            WlanCloseHandle(hWlan, NULL);

        return fRet;
    }
}