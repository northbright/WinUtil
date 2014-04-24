#pragma once

#include <string>
#include <vector>
using namespace std;

#define SIZE_PB           1125899906842624  // (1024 * 1024 * 1024 * 1024 * 1024)
#define SIZE_TB           1099511627776  // (1024 * 1024 * 1024 * 1024)
#define SIZE_GB           1073741824  // (1024 * 1024 * 1024)
#define SIZE_MB           1048576  // (1024 * 1024)
#define SIZE_KB           1024

namespace Util {

    bool StringToWstring(UINT nCodePage, const string& str, wstring& wstr);
    bool WstringToString(UINT nCodePage, const wstring& wstr, string& str);

    bool UTF8StringToWstring(const string& strUTF8, wstring& wstr);
    bool WstringToUTF8String(const wstring& wstr, string& strUTF8);

    // Size in byte to string. Unit: from B to PB. Ex: 2.4PB, 1.2TB, 4GB, 3.02MB, 234KB, 45B...
    void SizeToString(unsigned __int64 nSize, wstring& wstr);
    void SizeToString(unsigned __int64 nSize, string& strUTF8);

    void IntVectorToWstring(vector<int>& ints, wstring& wstr);
    void UIntVectorToWstring(vector<unsigned int>& uints, wstring& wstr);

    bool FormatArgsToWstring(wstring& wstr, LPCWSTR format, va_list args);
    bool FormatString(wstring& wstr, LPCWSTR format, ...);

    bool WstringToTime(const wstring& wstr, SYSTEMTIME& st);  // Format: YYYY/MM/DD HH:MM:SS or YYYY/MM/DD HH:MM. Ex: "2012/07/12 17:25:03", "2012/07/12 07:25"
    bool StringToTime(const string& str, SYSTEMTIME& st);  // Format: YYYY/MM/DD HH:MM:SS or YYYY/MM/DD HH:MM. Ex: "2012/07/12 17:25:03", "2012/07/12 07:25"
}