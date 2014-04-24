#pragma once

#include <string>
#include <set>
using namespace std;

namespace Util {

    // CFileExtFilter
    // Used to set a filter string and check if a file name matches.
    // File Extenstion Filter String: "*.mp3;*.wma;*.ogg"...

    class CFileExtFilter {
    public:
        CFileExtFilter();
        CFileExtFilter(LPCWSTR lpszFilter, LPCWSTR lpszSpliter = _T(";,"));
        CFileExtFilter(const char* lpszUTF8Filter, const char* lpszUTF8Spliter = ";,");

        bool Set(LPCWSTR lpszFilter, LPCWSTR lpszSpliter = _T(";,"));
        bool Set(const char* lpszUTF8Filter, const char* lpszUTF8Spliter = ";,");

        bool IsFileMatch(LPCWSTR lpszFileName);
        bool IsFileMatch(const char* lpszUTF8FileName);

        wstring m_wstrSpilter;
        set<wstring> m_extSet;
    };

}