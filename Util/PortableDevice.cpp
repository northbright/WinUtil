#include <Windows.h>
#include <tchar.h>
#include <Dbt.h>  // DEV_BROADCAST_DEVICEINTERFACE

#include "Debug.h"
#include "String.h"
#include "PortableDevice.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Util {
   
    bool ParsePortableDeviceChangeMsg(WPARAM wParam, LPARAM lParam, PORTABLE_DEVICE_CHANGE_INFO& info)
    {
        bool fRet = false;
        PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
        PDEV_BROADCAST_VOLUME pVolume = (PDEV_BROADCAST_VOLUME)pHdr;

        info.changedVolumes.clear();

        // We process DBT_DEVICEARRIVAL / DBT_DEVICEREMOVECOMPLETE for volumes change only.
        if (wParam != DBT_DEVICEARRIVAL && wParam != DBT_DEVICEREMOVECOMPLETE)
            goto END;

        info.dwEvent = (DWORD)wParam;

        switch (pHdr->dbch_devicetype)
        {
        case DBT_DEVTYP_DEVICEINTERFACE:
            break;

        case DBT_DEVTYP_VOLUME:
            for (int i = 0; i < 25; i++) {
                if (pVolume->dbcv_unitmask &  (1 << i)) {
                    char letter[2] = {0};
                    letter[0] = 65 + i;
                    string strPath(letter);
                    wstring wstrPath;
                    strPath += ":\\";
                    StringToWstring(CP_ACP, strPath, wstrPath);
                    info.changedVolumes.push_back(wstrPath);
                }
            }
            fRet = true;
            break;

        default:
            break;
        }

    END:
        return fRet; 
    }
    
    void GetRemovableStoragePaths(vector<wstring>& paths, vector<STORAGE_BUS_TYPE>* pBusTypes)
    {
        HANDLE hFile = NULL;
        DWORD dwErr = 0;

        paths.clear();

        for (int i = 0; i < 26; i++)
        {
            bool fRet = false;
            DWORD dwOut = 0;
            char buffer[1024] = {0};
            PSTORAGE_DEVICE_DESCRIPTOR pDevDesc = (PSTORAGE_DEVICE_DESCRIPTOR)buffer;
            STORAGE_PROPERTY_QUERY query;
            char letter = 'A' + i;
            string strPath = "\\\\.\\";
            wstring wstrPath;

            strPath += letter;
            strPath += ":";
            StringToWstring(CP_ACP, strPath, wstrPath);

            hFile = CreateFile(wstrPath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                query.PropertyId = StorageDeviceProperty;
                query.QueryType = PropertyStandardQuery;
                pDevDesc->Size = sizeof(buffer);

                fRet = DeviceIoControl(hFile, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY),  pDevDesc, sizeof(buffer), &dwOut, NULL);
                if (fRet)
                {
                    if ((pDevDesc->RemovableMedia))
                    {
                        bool fMatch = false;
                        if (pBusTypes)
                        {
                            for (size_t j = 0; j < pBusTypes->size(); j++)
                            {
                                if (pDevDesc->BusType == (*pBusTypes)[j])
                                {
                                    fMatch = true;
                                    break;
                                }
                            }
                        }
                        else  // all bus types
                            fMatch = true;

                        if (fMatch)
                        {
                            string strDrivePath;
                            wstring wstrDrivePath;

                            strDrivePath = letter;
                            strDrivePath += ":\\";
                            StringToWstring(CP_ACP, strDrivePath, wstrDrivePath);

                            paths.push_back(wstrDrivePath);
                            Util::DBG_MSG(L"path = %s\r\n", wstrDrivePath.c_str());
                        }
                    }
                }

                CloseHandle(hFile);
                hFile = NULL;
            }
        }
        return;
    }

    void GetUSBDiskPaths(vector<wstring>& paths)
    {
        vector<STORAGE_BUS_TYPE> busTypes;

        busTypes.push_back(BusTypeUsb);
        GetRemovableStoragePaths(paths, &busTypes);
    }
};