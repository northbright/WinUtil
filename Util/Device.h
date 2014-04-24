#pragma once

#include <initguid.h>

#include <string>
#include <vector>

using namespace std;

namespace Util {

    // Get device paths by given interface guid.
    bool GetDevicePathsByInterfaceGUID(const GUID& interfaceGuid, vector<wstring>& pathVector);

    // Get device path by given interface guid and 0-based index.
    bool GetDevicePathByInterfaceGUID(const GUID& interfaceGuid, UINT nIndex, wstring& strPath);

    // Get device count by given interface guid.
    ULONG GetDeviceCountByInterfaceGUID(const GUID& interfaceGuid);
}