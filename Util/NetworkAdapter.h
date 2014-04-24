#pragma once

#include "../jsoncpp/include/json/json.h"

#include <string>
#include <vector>

using namespace std;

#pragma comment(lib, "Propsys.lib")  // VariantToXX functions

namespace Util {

    namespace NetworkAdapter {

        bool GetInfos(vector<Json::Value>& infos);
    }
}