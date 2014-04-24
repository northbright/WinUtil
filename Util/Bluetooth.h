#pragma once

// Bluetooth Functions are supported since Windows 8 and later.

#define BTH_RADIO_SUPPORT          L"SYSTEM\\CurrentControlSet\\Services\\BTHPORT\\Parameters\\Radio Support\\"
#define BTH_SUPPORT_DLL            L"SupportDLL"

typedef DWORD (WINAPI *PFN_BLUETOOTH_ENABLE_RADIO)(bool fEnable);  // BluetoothEnableRadio() prototype.
typedef DWORD (WINAPI *PFN_IS_BLUETOOTH_RADIO_ENABLED)(bool* pfEnabled);  // IsBluetoothRadioEnabled() prototype.

#include <string>
using namespace std;

namespace Util {
    bool GetBluetoothSupportDllPath(wstring& strDllPath);
    bool SetBluetoothPowerOn(bool fOn);
    bool GetBluetoothPowerState(bool& fOn);
}