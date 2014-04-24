#pragma once

#include <Winioctl.h>

#include <string>
#include <vector>

using namespace std;

namespace Util {
    // WM_DEVICECHANGE message will be send to all top windows automatically for portable devices. See MSDN.

    typedef struct _PORTABLE_DEVICE_CHANGE_INFO
    {
        DWORD dwEvent;  // DBT_DEVICEARRIVAL or DBT_DEVICEREMOVECOMPLETE defined by MSFT in Dbt.h.
        vector<wstring> changedVolumes;
    }PORTABLE_DEVICE_CHANGE_INFO;

    // Use this API to parse the WM_DEVICECHANGE for portable devices.
    // Ex:
    //case WM_DEVICECHANGE:
    //    {
    //        bool fRet = false;
    //        PORTABLE_DEVICE_CHANGE_INFO info;
    //        wstring wstr;
    //        
    //        // ----------- Device ------------
    //        fRet = ParsePortableDeviceChangeMsg(wParam,  lParam, info);
    //        if (fRet) {
    //            wstr = (info.dwEvent == DBT_DEVICEARRIVAL) ? L"Arrival: " : L"Removed: ";
    //            for (size_t i = 0; i < info.changedVolumes.size(); i++) {
    //                wstr += info.changedVolumes[i];
    //                wstr += L" ";
    //            }
    //            OutputDebugString(wstr.c_str());
    //        }
    //        break;
    //    }

    bool ParsePortableDeviceChangeMsg(WPARAM wParam, LPARAM lParam, PORTABLE_DEVICE_CHANGE_INFO& info);

    // Get removable storage drive paths.
    // paths: Output paths.
    // pBusTypes: Specified bus type. If NULL, all removable storage drive path will be returned.
    void GetRemovableStoragePaths(vector<wstring>& paths, vector<STORAGE_BUS_TYPE>* pBusTypes = NULL);

    // Get USB disk drive paths.
    void GetUSBDiskPaths(vector<wstring>& paths);
};
