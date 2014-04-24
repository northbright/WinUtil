#include <windows.h>
#include <tchar.h>
#include <objbase.h>                                
#include <stdio.h>
#include <wbemidl.h> 
#include <comdef.h>
#include <Propvarutil.h>  // Variant functions

#include "Debug.h"
#include "Wmi.h"
#include "NetworkAdapter.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Util {

    namespace NetworkAdapter {

        bool GetInfos(vector<Json::Value>& infos)
        {
            bool fRet = false;
            WMI::Namespace ns(L"root\\CIMV2");
            vector<WMI::Obj> objs;
            Json::Value inParams;

            infos.clear();
            ns.GetObjs(L"CIM_NetworkAdapter", objs);
            for (size_t i = 0; i < objs.size(); i++)
            {
                Json::Value root;

                fRet = objs[i].GetProps(root);
                if (root["PhysicalAdapter"].asBool())
                    infos.push_back(root);
            }

            return fRet;
        }
    }  // namespace NetworkAdapter
}  // namespace Util