#pragma once

#include <comutil.h>
#include <OleAuto.h>

#include "../jsoncpp/include/json/json.h"

#include <string>
#include <vector>

using namespace std;

#pragma comment(lib, "OleAut32.lib")

namespace Util {

    // Convert BSTR array to vector<wstring>.
    // VT_ARRAY | VT_BSTR, fFeatures: FADF_BSTR
    bool SafeArrayToWstringVector(SAFEARRAY* psa, vector<wstring>& strs);

    // Convert VT_I4 array stored with ANSI chars to wstring.
    bool VT_I4SafeArrayToWstring(SAFEARRAY* psa, wstring& str);

    // VT_ARRAY | VT_BSTR
    bool VariantToWstringVector(_variant_t& var, vector<wstring>& strs);

    // VT_ARRAY | VT_I1, VT_I2, VT_I4, VT_I8
    bool VariantToIntVector(_variant_t& var, vector<__int64>& ints);

    // VT_ARRAY | VT_UI1, VT_UI2, VT_UI4, VT_UI8
    bool VariantToUIntVector(_variant_t& var, vector<unsigned __int64>& uints);

    // VT_ARRAY | VT_R4, VT_R8
    bool VariantToDoubleVector(_variant_t& var, vector<double>& doubles);


    // Try to convert VARIANT to Json::Value.
    bool VariantToJsonValue(_variant_t& var, Json::Value& value);

}