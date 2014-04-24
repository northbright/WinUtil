#pragma once

#include <mbnapi.h>

#include <vector>

namespace Util {

    // Need call CoInitialize() / CoUninitialize() for the process.
    class MobileBroadbandManager {
    public:
        MobileBroadbandManager();
        ~MobileBroadbandManager();

        int GetDeviceCount();

        bool GetPowerState(int nDeviceId, bool& fOn);
        bool SetPowerState(int nDeviceId, bool fOn);
        bool SetAllPowerState(bool fOn);

        bool GetSignalStrength(int nDeviceId, ULONG& ulStrength);

        IMbnInterfaceManager* m_pManager;
    };

    typedef struct _MODEM_INFO {
        wstring strManufacturer;
        wstring strModel;
        wstring strRevision;
        wstring strSVN;
        wstring strIMEI;

    }MODEM_INFO;

    class Modem {
    public:
        Modem();
        Modem(LPCWSTR lpszCom, DWORD dwBaudrate = 115200);
        ~Modem();

        bool Set(LPCWSTR lpszCom, DWORD dwBaudrate = 115200);
        bool IsValid();

        // resultArray[0]: lpszATCommand
        // resultArray[1] - [n - 1]: output lines.
        bool ParseATCommandResult(LPCSTR lpszATCommand, string& strOutput, vector<string>& resultArray);
        bool SendATCommand(LPCSTR lpszATCommand, string& strOutput, DWORD dwWaitTime = 100);
        bool SendATCommand(LPCSTR lpszATCommand, vector<string>& resultArray, DWORD dwWaitTime = 100);

        bool GetInfo(MODEM_INFO& modemInfo, DWORD dwWaitTime = 100);
        bool GetIMSI(wstring& strIMSI, DWORD dwWaitTime = 100);

        HANDLE m_hCom;
    };
}