#include <windows.h>
#include <tchar.h>
#include <Propvarutil.h>  // VariantToFileTime(), VariantToXXX()...

#include "Util.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Propsys.lib")

namespace Util {

    bool SafeArrayToWstringVector(SAFEARRAY* psa, vector<wstring>& strs)
    {
        bool fRet = false;
        HRESULT hr = S_OK;
        LONG lStart = 0;
        LONG lEnd = 0;    

        if (!psa)
            goto END;

        if (!(psa->fFeatures & FADF_BSTR))
            goto END;

        hr = SafeArrayGetLBound(psa, 1, &lStart);
        if (FAILED(hr))
            goto END;

        hr = SafeArrayGetUBound(psa, 1, &lEnd);
        if (FAILED(hr))
            goto END;

        strs.clear();
        for (LONG i = lStart; i <= lEnd; i++) {
            BSTR bstr = NULL;

            hr = SafeArrayGetElement(psa, &i, &bstr);
            if (FAILED(hr))
            {
                if (bstr)
                    SysFreeString(bstr);

                goto END;
            }
            else
            {
                strs.push_back(bstr);
                if (bstr)
                    SysFreeString(bstr);
            }
        }

        fRet = true;
    END:
        return fRet;
    }

    bool VariantToWstringVector(_variant_t& var, vector<wstring>& strs)
    {
        bool fRet = false;
        HRESULT hr = S_OK;

        if (!(var.vt & (VT_ARRAY | VT_BSTR)))
            goto END;

        if (!SafeArrayToWstringVector(var.parray, strs))
            goto END;

        fRet = true;
    END:
        return fRet;
    }

    // VT_ARRAY | VT_I1, VT_I2, VT_I4, V_I8
    bool VariantToIntVector(_variant_t& var, vector<__int64>& ints)
    {
        bool fRet = false;
        HRESULT hr = S_OK;
        LONG lStart = 0;
        LONG lEnd = 0;

        if ((var.vt != (VT_ARRAY | VT_I1)) && (var.vt != (VT_ARRAY | VT_I2)) && (var.vt != (VT_ARRAY | VT_I4)) && (var.vt != (VT_ARRAY | VT_I8)))
            goto END;

        hr = SafeArrayGetLBound(var.parray, 1, &lStart);
        if (FAILED(hr))
            goto END;

        hr = SafeArrayGetUBound(var.parray, 1, &lEnd);
        if (FAILED(hr))
            goto END;

        ints.clear();
        for (LONG i = lStart; i <= lEnd; i++) {
            if (var.vt == (VT_ARRAY | VT_I1))
            {
                INT8 n = 0;
                hr = SafeArrayGetElement(var.parray, &i, &n);
                if (FAILED(hr))
                    goto END;

                ints.push_back((__int64)n);
            }
            else if (var.vt == (VT_ARRAY | VT_I2))
            {
                INT16 n = 0;
                hr = SafeArrayGetElement(var.parray, &i, &n);
                if (FAILED(hr))
                    goto END;

                ints.push_back((__int64)n);
            }
            else if (var.vt == (VT_ARRAY | VT_I4))
            {
                INT32 n = 0;
                hr = SafeArrayGetElement(var.parray, &i, &n);
                if (FAILED(hr))
                    goto END;

                ints.push_back((__int64)n);
            }
            else if (var.vt == (VT_ARRAY | VT_I8))
            {
                __int64 n = 0;
                hr = SafeArrayGetElement(var.parray, &i, &n);
                if (FAILED(hr))
                    goto END;

                ints.push_back((__int64)n);
            }
            else
            {
                goto END;
            }
        }

        fRet = true;
    END:
        return fRet;
    }

    // VT_ARRAY | VT_UI1, VT_UI2, VT_UI4, VT_UI8
    bool VariantToUIntVector(_variant_t& var, vector<unsigned __int64>& uints)
    {
        bool fRet = false;
        HRESULT hr = S_OK;
        LONG lStart = 0;
        LONG lEnd = 0;

        if ((var.vt != (VT_ARRAY | VT_UI1)) && (var.vt != (VT_ARRAY | VT_UI2)) && (var.vt != (VT_ARRAY | VT_UI4)) && (var.vt != (VT_ARRAY | VT_UI8)))
            goto END;

        hr = SafeArrayGetLBound(var.parray, 1, &lStart);
        if (FAILED(hr))
            goto END;

        hr = SafeArrayGetUBound(var.parray, 1, &lEnd);
        if (FAILED(hr))
            goto END;

        uints.clear();
        for (LONG i = lStart; i <= lEnd; i++) {
            if (var.vt == (VT_ARRAY | VT_UI1))
            {
                UINT8 n = 0;
                hr = SafeArrayGetElement(var.parray, &i, &n);
                if (FAILED(hr))
                    goto END;

                uints.push_back((unsigned __int64)n);
            }
            else if (var.vt == (VT_ARRAY | VT_UI2))
            {
                UINT16 n = 0;
                hr = SafeArrayGetElement(var.parray, &i, &n);
                if (FAILED(hr))
                    goto END;

                uints.push_back((unsigned __int64)n);
            }
            else if (var.vt == (VT_ARRAY | VT_UI4))
            {
                UINT32 n = 0;
                hr = SafeArrayGetElement(var.parray, &i, &n);
                if (FAILED(hr))
                    goto END;

                uints.push_back((unsigned __int64)n);
            }
            else if (var.vt == (VT_ARRAY | VT_UI8))
            {
                unsigned __int64  n = 0;
                hr = SafeArrayGetElement(var.parray, &i, &n);
                if (FAILED(hr))
                    goto END;

                uints.push_back((unsigned __int64)n);
            }
            else
            {
                goto END;
            }
        }

        fRet = true;
    END:
        return fRet;
    }

    // VT_ARRAY | VT_R4, VT_R8
    bool VariantToDoubleVector(_variant_t& var, vector<double>& doubles)
    {
        bool fRet = false;
        HRESULT hr = S_OK;
        LONG lStart = 0;
        LONG lEnd = 0;

        if ((var.vt != (VT_ARRAY | VT_R4)) && (var.vt != (VT_ARRAY | VT_R8)))
            goto END;

        hr = SafeArrayGetLBound(var.parray, 1, &lStart);
        if (FAILED(hr))
            goto END;

        hr = SafeArrayGetUBound(var.parray, 1, &lEnd);
        if (FAILED(hr))
            goto END;

        doubles.clear();
        for (LONG i = lStart; i <= lEnd; i++) {
            if (var.vt == (VT_ARRAY | VT_R4))
            {
                float f = 0;
                hr = SafeArrayGetElement(var.parray, &i, &f);
                if (FAILED(hr))
                    goto END;

                doubles.push_back((double)f);
            }
            else if (var.vt == (VT_ARRAY | VT_R8))
            {
                double d = 0;
                hr = SafeArrayGetElement(var.parray, &i, &d);
                if (FAILED(hr))
                    goto END;

                doubles.push_back(d);
            }
            else
            {
                goto END;
            }
        }

        fRet = true;
    END:
        return fRet;
    }


    bool VT_I4SafeArrayToWstring(SAFEARRAY* psa, wstring& str)
    {
        bool fRet = false;
        HRESULT hr = S_OK;
        LONG lStart = 0;
        LONG lEnd = 0;
        string s;

        hr = SafeArrayGetLBound(psa, 1, &lStart);
        if (FAILED(hr))
            goto END;

        hr = SafeArrayGetUBound(psa, 1, &lEnd);
        if (FAILED(hr))
            goto END;

        str.clear();
        for (LONG i = lStart; i <= lEnd; i++) {
            INT32 n = 0;

            hr = SafeArrayGetElement(psa, &i, &n);
            if (FAILED(hr))
                goto END;

            s.push_back((char)n);
        }

        if (!StringToWstring(CP_ACP, s, str))
            goto END;

        fRet = true;
    END:
        return fRet;
    }

    bool VariantToJsonValue(_variant_t& var, Json::Value& value)
    {
        bool fRet = false;

        value.clear();

        // VT_ARRAY
        if (var.vt & VT_ARRAY)
        {
            // BSTR ARRAY
            if (var.vt == (VT_ARRAY | VT_BSTR))
            {
                vector<wstring> strs;
                wstring wstr;
                string str;

                if (!SafeArrayToWstringVector(var.parray, strs))
                    goto END;

                for (size_t i = 0; i < strs.size(); i++)
                {
                    string strUTF8;

                    WstringToUTF8String(strs[i], strUTF8);
                    value[(Json::ArrayIndex)i] = strUTF8;
                }
            }
            else if ((var.vt == (VT_ARRAY | VT_I1)) || (var.vt == (VT_ARRAY | VT_I2)) || (var.vt == (VT_ARRAY | VT_I4)) || (var.vt == (VT_ARRAY | VT_I8)))
            {
                vector<__int64> ints;
                if (!VariantToIntVector(var, ints))
                    goto END;

                for (size_t i = 0; i < ints.size(); i++)
                    value[(Json::Value::ArrayIndex)i] =  ints[i];
            }
            else if ((var.vt == (VT_ARRAY | VT_UI1)) || (var.vt == (VT_ARRAY | VT_UI2)) || (var.vt == (VT_ARRAY | VT_UI4)) || (var.vt == (VT_ARRAY | VT_UI8)))
            {
                vector<unsigned __int64> uints;
                if (!VariantToUIntVector(var, uints))
                    goto END;

                for (size_t i = 0; i < uints.size(); i++)
                    value[(Json::ArrayIndex)i] =  uints[i];
            }
            else
            {
                goto END;  // Not supported array type.
            }
        }
        else if (var.vt == VT_BSTR)
        {
            _bstr_t bstr = (_bstr_t)var;
            value = (char*)bstr;
        }
        else if (var.vt == VT_BOOL)
        {
            if (var.boolVal == VARIANT_TRUE)
                value = true;
            else
                value = false;
        }
        else if (var.vt == VT_I1 || var.vt == VT_I2 || var.vt == VT_I4 || var.vt == VT_I8)
        {
            __int64 n = (__int64)var;  // __int64 extrator of _variant_t
            value = n;
        }
        else if (var.vt == VT_UI1 || var.vt == VT_UI2 || var.vt == VT_UI4 || var.vt == VT_UI8)
        {
            unsigned __int64 n = (unsigned __int64)var;  // unsigned __int64 extrator of _variant_t
            value = n;
        }
        else if (var.vt == VT_R4 || var.vt == VT_R8)
        {
            double d = (double)var;
            value = d;
        }
        else if (var.vt == VT_DATE)
        {
            SYSTEMTIME st = {0};
            char szBuffer[256] = {0};

            if (!VariantTimeToSystemTime(var.date, &st))
                goto END;

            sprintf(szBuffer, "%4d/%d/%d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
            value = szBuffer;
        }
        else if (var.vt == VT_NULL)
        {
            value = Json::nullValue;
        }
        else
        {
            // not supported VARIANT VT type.
            value = Json::nullValue;
            goto END;
        }

        fRet = true;

    END:
        return fRet;
    }

}