#include <windows.h>
#include <tchar.h>
#include <Setupapi.h>

#include "Debug.h"
#include "String.h"
#include "Device.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Setupapi.lib")

// Copy from HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses
// GUID_DEVINTERFACE_USB_DEVICE
//{ 0xA5DCBF10, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },

// GUID_DEVINTERFACE_DISK
//{ 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },

namespace Util {

    bool GetDevicePathsByInterfaceGUID(const GUID& interfaceGuid, vector<wstring>& pathVector)
    {
        bool fRet = false;
        HDEVINFO hDeviceInfoSet = NULL;
        SP_DEVINFO_DATA devInfo = {0};
        SP_DEVICE_INTERFACE_DATA deviceInterfaceData = {0};
        PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;
        DWORD dwErr = 0;
        DWORD dwLen = 0;

        pathVector.clear();

        hDeviceInfoSet = SetupDiGetClassDevs(&interfaceGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if (hDeviceInfoSet == INVALID_HANDLE_VALUE) 
            goto END;

        devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

        for (UINT i = 0; SetupDiEnumDeviceInfo(hDeviceInfoSet, i, &devInfo); i++)
        {
            deviceInterfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

            //Get information about the device interface.
            fRet = SetupDiEnumDeviceInterfaces(hDeviceInfoSet, &devInfo, &interfaceGuid, 0, &deviceInterfaceData);
            if (!fRet)
            {
                if (GetLastError() == ERROR_NO_MORE_ITEMS)
                    break;
                else
                    Util::DBG_MSG(L"GetDeviceHandleByInterfaceGUID(): SetupDiEnumDeviceInterfaces() failed.\r\n");
            }

            // Get the buffer length of interface detail.
            fRet = SetupDiGetDeviceInterfaceDetail(hDeviceInfoSet, &deviceInterfaceData, NULL, 0, &dwLen, NULL);
            if (!fRet) 
            {
                if ((GetLastError() == ERROR_INSUFFICIENT_BUFFER) && (dwLen > 0))
                {
                    //we got the size, allocate buffer
                    pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)new BYTE[dwLen];
                    if (!pInterfaceDetailData) 
                        goto END;
                }
                else
                {
                    Util::DBG_MSG(L"GetDeviceHandleByInterfaceGUID(): SetupDiGetDeviceInterfaceDetail() failed. Err = %d.\r\n", GetLastError());
                    goto END;
                }
            }

            //get the interface detailed data
            pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            fRet = SetupDiGetDeviceInterfaceDetail(hDeviceInfoSet, &deviceInterfaceData, pInterfaceDetailData, dwLen, NULL, NULL);
            if (!fRet) 
            {
                Util::DBG_MSG(L"GetDeviceHandleByInterfaceGUID(): SetupDiGetDeviceInterfaceDetail() failed. Err = %d.\r\n", GetLastError());
                goto END;
            }

            pathVector.push_back(pInterfaceDetailData->DevicePath);

            if (pInterfaceDetailData)
            {
                delete[] pInterfaceDetailData;
                pInterfaceDetailData = NULL;
            }
        }

        fRet = true;

    END:
        if (pInterfaceDetailData)
        {
            delete[] pInterfaceDetailData;
            pInterfaceDetailData = NULL;
        }

        if (hDeviceInfoSet)
        {
            SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
        }

        return fRet;
    }

    bool GetDevicePathByInterfaceGUID(const GUID& interfaceGuid, UINT nIndex, wstring& strPath)
    {
        bool fRet = false;
        vector<wstring> paths;

        strPath.clear();

        if (!GetDevicePathsByInterfaceGUID(interfaceGuid, paths))
            goto END;

        if (!paths.size())
            goto END;

        if (nIndex >= paths.size())
            goto END;

        strPath = paths[nIndex];
        fRet = true;
    END:
        return fRet;
    }

    ULONG GetDeviceCountByInterfaceGUID(const GUID& interfaceGuid)
    {
        ULONG nCount = 0;
        vector<wstring> paths;

        if (!GetDevicePathsByInterfaceGUID(interfaceGuid, paths))
            goto END;
        
        nCount = paths.size();
    END:
        return nCount;
    }
}