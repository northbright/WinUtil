#include <windows.h>
#include <tchar.h>
#include <objbase.h>                                
#include <stdio.h>
#include <wbemidl.h> 
#include <comdef.h>
#include <Propvarutil.h>  // Variant functions

#include "Debug.h"
#include "Wmi.h"
#include "Monitor.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "libjson.lib")

namespace Util {

    namespace Monitor {

        bool SetBrightness(int nPercent)
        {
            bool fRet = false;
            char szBuffer[256] = {0};
            Json::Value inParams;
            Json::Reader reader;
            WMI::Namespace ns(L"root\\wmi");
            vector<WMI::Obj> objs;

            if (!ns.GetObjs(L"WmiMonitorBrightnessMethods", objs))
                goto END;

            sprintf(szBuffer, "[ {\"name\": \"Timeout\", \"value\": \"100\", \"type\": %d}, {\"name\": \"Brightness\", \"value\": \"%d\", \"type\": %d} ]", CIM_UINT32, nPercent, CIM_UINT8);
            if (!reader.parse(szBuffer, inParams))
                goto END;

            if (!ns.ExecMethod(objs[0], L"WmiMonitorBrightnessMethods", L"WmiSetBrightness", &inParams, NULL))
                goto END;

            fRet = true;
        END:

            return fRet;
        }

        bool GetBrightness(int& nPercent)
        {
            bool fRet = false;
            WMI::Namespace ns(L"root\\wmi");
            vector<WMI::Obj> objs;
            Json::Value props;

            if (!ns.GetObjs(L"WmiMonitorBrightness", objs))
                goto END;

            if (!objs[0].GetProps(props))
                goto END;

            nPercent = props["CurrentBrightness"].asUInt();

            fRet = true;
        END:

            return fRet;
        }

        bool GetInfo(Json::Value& root)
        {
            bool fRet = false;
            WMI::Namespace ns(L"root\\wmi");
            vector<WMI::Obj> objs;

            if (!ns.GetObjs(L"WmiMonitorID", objs))
                goto END;

            if (!objs[0].GetProps(root))
                goto END;

        END:
            return fRet;
        }

        bool SetPowerState(int nState)
        {
            return PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, nState);
        }

    }  // namespace Monitor
}  // namespace Util