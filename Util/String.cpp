
#include <windows.h>
#include <tchar.h>

#include <boost/algorithm/string.hpp>  // split()
#include <boost/algorithm/string/replace.hpp> // replace_all()

#include "String.h"

#include "Debug.h"

using namespace boost;

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Util {

    bool StringToWstring(UINT nCodePage, const string& str, wstring& wstr)
    {
        bool fRet = false;
        WCHAR* pBuffer = NULL;
        size_t nSize = 0;

        if (str.empty())
            goto END;

        nSize = str.size();
        pBuffer = new WCHAR[nSize + 1];
        if (!pBuffer)
            goto END;

        memset(pBuffer, 0, sizeof(WCHAR) * (nSize + 1));

        if (!MultiByteToWideChar(nCodePage, 0, str.c_str(), nSize, pBuffer, nSize + 1))
            goto END;

        wstr = pBuffer;

        // Done
        fRet = true;      

    END:
        if (pBuffer)
            delete[] pBuffer;

        return fRet;
    }    

    bool WstringToString(UINT nCodePage, const wstring& wstr, string& str)
    {
        bool fRet = false;
        char* pBuffer = NULL;
        size_t nSize = 0;

        if (wstr.empty())
            goto END;

        nSize = wstr.size();
        pBuffer = new char[nSize + 1];
        if (!pBuffer)
            goto END;

        memset(pBuffer, 0, sizeof(char) * (nSize + 1));

        if (!WideCharToMultiByte(nCodePage, 0, wstr.c_str(), nSize, pBuffer, nSize + 1, NULL, NULL))
            goto END;

        str = pBuffer;

        // Done
        fRet = true;      

    END:
        if (pBuffer)
            delete[] pBuffer;

        return fRet;
    }    

    bool UTF8StringToWstring(const string& strUTF8, wstring& wstr)
    {
        return StringToWstring(CP_UTF8, strUTF8, wstr);
    }            

    bool WstringToUTF8String(const wstring& wstr, string& strUTF8)
    {
        return WstringToString(CP_UTF8, wstr, strUTF8);
    }


    void SizeToString(unsigned __int64 nSize, wstring& wstr)
    {
        bool fRet = true;
        unsigned __int64 nThresholdArray[5] = {SIZE_PB, SIZE_TB, SIZE_GB, SIZE_MB, SIZE_KB};
        WCHAR* lpszUnitArray[5] = {_T("PB"), _T("TB"), _T("GB"), _T("MB"), _T("KB")};
        WCHAR szBuffer[256] = {0};

        wstr.clear();

        for (size_t i = 0; i < 5; i++)
        {
            if (nSize >= nThresholdArray[i])
            {
                UINT m = nSize / nThresholdArray[i];
                unsigned __int64 n = nSize % nThresholdArray[i];

                if (n)
                {
                    UINT nPercent = (UINT)(n * 100 / nThresholdArray[i]);
                    UINT nTemp = (UINT)(n * 1000 / nThresholdArray[i]);
                    UINT k = nTemp % 10;
                    if (k >= 5)
                        nPercent += 1;

                    double v = m + (double)nPercent / 100;
                    swprintf(szBuffer, _T("%.2f%s"), v, lpszUnitArray[i]);
                }
                else
                {
                    swprintf(szBuffer, _T("%u%s"), m, lpszUnitArray[i]);
                }

                break;    
            }
            else
            {
                if (i == 4)  // nSize < 1024B
                    swprintf(szBuffer, _T("%d%B"), nSize);
                else
                    continue;    
            }
        }

        wstr = szBuffer;
    }

    void SizeToString(unsigned __int64 nSize, string& strUTF8)
    {
        wstring wstr;

        strUTF8.clear();
        SizeToString(nSize, wstr);

        WstringToUTF8String(wstr, strUTF8);
    }

    void IntVectorToWstring(vector<int>& ints, wstring& wstr)
    {
        string str;

        for (size_t i = 0; i < ints.size(); i++)
        {
            str.push_back((char)ints[i]);
        }

        StringToWstring(CP_ACP, str, wstr);
    }

    void UIntVectorToWstring(vector<unsigned int>& uints, wstring& wstr)
    {
        string str;

        for (size_t i = 0; i < uints.size(); i++)
        {
            str.push_back((char)uints[i]);
        }

        StringToWstring(CP_ACP, str, wstr);
    }

    bool FormatArgsToWstring(wstring& wstr, LPCWSTR format, va_list args)
    {
        bool fRet = false;
        int nCount = 0;
        int nRet = 0;
        WCHAR* pBuffer = NULL;

        wstr.clear();

        nCount = _vscwprintf(format, args);
        if (!nCount)
            goto END;

        pBuffer = new WCHAR[nCount + 1];
        if (!pBuffer)
            goto END;

        memset(pBuffer, 0, sizeof(WCHAR) * (nCount + 1));
        nRet = vswprintf(pBuffer, nCount + 1, format, args);
        if (nRet <= 0)
            goto END;

        wstr = pBuffer;
        fRet = true;
    END:
        if (pBuffer)
            delete[] pBuffer;

        return fRet;
    }

    bool FormatString(wstring& wstr, LPCWSTR format, ...)
    {
        bool fRet = false;
        va_list args;

        va_start(args, format);
        if (!FormatArgsToWstring(wstr, format, args))
            goto END;

        fRet = true;
    END:
        va_end(args);
        return fRet;
    }

    bool WstringToTime(const wstring& wstr, SYSTEMTIME& st)
    {
        bool fRet = false;
        vector<wstring> strVector, dateVector, timeVector;

        split(strVector, wstr, is_any_of(L" "));
        if (strVector.size() != 2)
            goto END;

        split(dateVector, strVector[0], is_any_of(L"/"));
        if (dateVector.size() != 3)
            goto END;

        st.wYear = _wtoi(dateVector[0].c_str());
        st.wMonth = _wtoi(dateVector[1].c_str());
        st.wDay = _wtoi(dateVector[2].c_str());

        split(timeVector, strVector[1], is_any_of(L":"));
        if ((timeVector.size() != 3) && (timeVector.size() != 2))  // size = 3: hh:mm:ss, size = 2: hh:mm and set ss = 0.
            goto END;

        st.wHour = _wtoi(timeVector[0].c_str());
        st.wMinute = _wtoi(timeVector[1].c_str());
        st.wSecond = (timeVector.size() == 3) ? _wtoi(timeVector[2].c_str()) : 0;

        fRet = true;
    END:
        return fRet;
    }

    bool StringToTime(const string& str, SYSTEMTIME& st)
    {
        bool fRet = false;
        wstring wstr;

        if (!UTF8StringToWstring(str, wstr))
            goto END;

        fRet = WstringToTime(wstr, st);
    END:
        return fRet;
    }
}