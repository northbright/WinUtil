#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>

#include <vector>
#include <boost/algorithm/string.hpp>  // split(), to_lower()
#include <boost/algorithm/string/replace.hpp> // replace_all()

#include "Debug.h"
#include "String.h"
#include "FileExtFilter.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace boost;
using namespace Util;

CFileExtFilter::CFileExtFilter()
{
}

CFileExtFilter::CFileExtFilter(LPCWSTR lpszFilter, LPCWSTR lpszSpliter)
{
    Set(lpszFilter, lpszSpliter);
}

CFileExtFilter::CFileExtFilter(const char* lpszUTF8Filter, const char* lpszUTF8Spliter)
{
    Set(lpszUTF8Filter, lpszUTF8Spliter);
}    

bool CFileExtFilter::Set(LPCWSTR lpszFilter, LPCWSTR lpszSpliter)
{
    bool fRet = false;
    vector<wstring> extVector;
    wstring strFilter = lpszFilter;
    size_t nSize = 0;

    if ((!lpszFilter) || (!lpszSpliter))
        goto END;

    // ------- Clear spaces and '*' in the filter string ---------
    boost::algorithm::to_lower(strFilter);
    replace_all(strFilter, _T("*"), _T(""));
    replace_all(strFilter, _T(" "), _T(""));

    split(extVector, strFilter, is_any_of(lpszSpliter));
    nSize = extVector.size();

    if (!nSize)
        goto END;

    m_extSet.clear();

    for (size_t i = 0; i < nSize; i++)
        m_extSet.insert(extVector[i]);

    // Done
    fRet = true;

END:
    return fRet;
}

bool CFileExtFilter::Set(const char* lpszUTF8Filter, const char* lpszUTF8Spliter)
{
    bool fRet = false;
    wstring wstrFilter;
    wstring wstrSpliter;

    if ((!lpszUTF8Filter) || (!lpszUTF8Spliter))
        goto END;

    if (!UTF8StringToWstring(lpszUTF8Filter, wstrFilter))
        goto END;

    if (!UTF8StringToWstring(lpszUTF8Spliter, wstrSpliter))
        goto END;

    if (!Set(wstrFilter.c_str(), wstrSpliter.c_str()))
        goto END;

    // Done
    fRet = true;

END:
    return fRet;              
}

bool CFileExtFilter::IsFileMatch(LPCWSTR lpszFileName)
{
    bool fRet = false;
    LPTSTR lpszExt = PathFindExtension(lpszFileName);
    wstring strExt = lpszExt;

    if (strExt.empty())
        goto END;

    boost::algorithm::to_lower(strExt);
    if (m_extSet.find(strExt) == m_extSet.end())
        goto END;

    // Done
    fRet = true;

END:
    return fRet;
}        

bool CFileExtFilter::IsFileMatch(const char* lpszUTF8FileName)
{
    bool fRet = false;
    wstring wstrFileName;

    if (!lpszUTF8FileName)
        goto END;

    if (!UTF8StringToWstring(lpszUTF8FileName, wstrFileName))
        goto END;

    if (!IsFileMatch(wstrFileName.c_str()))
        goto END;

    // Done
    fRet = true;

END:
    return fRet;
}
