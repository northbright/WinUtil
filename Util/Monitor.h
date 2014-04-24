#pragma once

#include "../jsoncpp/include/json/json.h"

#include <string>
#include <vector>

using namespace std;

#pragma comment(lib, "Propsys.lib")  // VariantToXX functions

namespace Util {

    namespace Monitor {

        bool SetBrightness(int nPercent);
        bool GetBrightness(int& nPercent);

        bool GetInfo(Json::Value& root);  // See "WmiMonitorID" class.

        bool SetPowerState(int nState);  // -1: On, 1: Low Power, 2: Off, See SC_MONITORPOWER on MSDN.

        //bool WmiCreateProcess(LPCWSTR lpszParams);
    }

}