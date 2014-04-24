
#include <windows.h>
#include <tchar.h>
#include "Shlwapi.h"  // Path functions
#include "WinInet.h"  // INTERNET_MAX_URL_LENGTH

#include <vector>
#include <boost/algorithm/string.hpp>  // split()
#include <boost/algorithm/string/replace.hpp> // replace_all()
#include <boost/algorithm/string/predicate.hpp>  // starts_with()

#include "Debug.h"
#include "String.h"
#include "Path.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace boost;

#pragma comment(lib, "Shlwapi.lib")

namespace Util {

    bool GetCurrentModulePath(wstring& strCurrentModulePath)
    {
        bool fRet = true;
        TCHAR szPath[512] = {0};

        strCurrentModulePath.clear();

        GetModuleFileName(NULL, szPath, sizeof(szPath) / sizeof(TCHAR));
        fRet = PathRemoveFileSpec(szPath);

        strCurrentModulePath = szPath;

        return fRet; 
    }

    bool GetFullPathRelativeToCurrentModule(LPCWSTR lpszRelativePath, wstring& strFullPath)
    {
        bool fRet = false;
        wstring strCurrentModulePath;
        TCHAR szFullPath[512] = {0};

        strFullPath.clear();

        if ((!lpszRelativePath) || (!_tcslen(lpszRelativePath)) || (!PathIsRelative(lpszRelativePath)))
            goto END;

        fRet = GetCurrentModulePath(strCurrentModulePath);
        if (!fRet)
            goto END;

        PathCombine(szFullPath, strCurrentModulePath.c_str(), lpszRelativePath);
        strFullPath = szFullPath;

        // ------- Done. ---------
        fRet = true;

    END:
        return fRet;    
    }

    bool GetFileURLRelativeToCurrentModule(LPCWSTR lpszRelativePath, wstring& strURL)  // "html\index.html" -> "file:///c:/myApp/html/index.html"
    {
        bool fRet = false;
        HRESULT hr = S_OK;
        wstring strFullPath;
        TCHAR szURL[INTERNET_MAX_URL_LENGTH] = {0};
        DWORD dwSize = sizeof(szURL) / sizeof(szURL[0]);

        strURL.clear();

        fRet = GetFullPathRelativeToCurrentModule(lpszRelativePath, strFullPath);
        if (!fRet)
            goto END;

        hr = UrlCreateFromPath(strFullPath.c_str(), szURL, &dwSize, 0);
        if ((hr != S_OK) && (hr != S_FALSE))  // S_FALSE indicates path is already in URL format.
            goto END;

        strURL = szURL;
        // ------- Done. ---------
        fRet = true;

    END:
        return fRet;
    }


    bool GetFileNameFromURL(const wstring& wstrURL, wstring& wstrFileName)
    {
        bool fRet = false;
        wstring wstrTempURL = wstrURL;
        vector<wstring> strTempVector;
        wstring strTemp;
        size_t nSize = 0;
        LPCWSTR lpszDelimiters = _T(":?#[]@*+,;=");  // See 2.2 reserved chars = gen-delims / sub-delims

        wstrFileName.clear();

        if (wstrURL.empty())
            goto END;

        // http://northbright.com:8088/findurl/index?id=20333#bottom
        // scheme|     authority      |     path    | query  |fragment|
        // See URI's W3C Syntax:
        // http://labs.apache.org/webarch/uri/rfc/rfc3986.html

        // Remove '://' if exits
        replace_all(wstrTempURL, _T("://"), _T(""));

        split(strTempVector, wstrTempURL, is_any_of(_T("/")));
        nSize = strTempVector.size();
        if ((nSize <= 1) || ((nSize == 2) && (strTempVector[1].empty())))  // '/' not found or just 'xxx.com/' -> vector size = 2: [0]: "xxx", [1]: "" 
            goto END;

        strTemp = strTempVector[nSize - 1];
        strTempVector.clear();

        split(strTempVector, strTemp, is_any_of(lpszDelimiters));  // remove any delimiters
        nSize = strTempVector.size();
        if (nSize <= 1)  // '?' not found
        {
            wstrFileName = strTemp;
        }
        else
            wstrFileName = strTempVector[0];

        // Done
        fRet = true;    

    END:
        return fRet;
    }

    bool GetFileNameFromURL(const string& strUTF8URL, string& strUTF8FileName)
    {
        bool fRet = false;
        wstring wstrURL;
        wstring wstrFileName;

        strUTF8FileName.clear();

        if (strUTF8URL.empty())
            goto END;

        UTF8StringToWstring(strUTF8URL, wstrURL);
        if (!GetFileNameFromURL(wstrURL, wstrFileName))
            goto END;

        WstringToUTF8String(wstrFileName, strUTF8FileName);       

        // Done
        fRet = true;

    END:
        return fRet;
    }            

    wstring CorrectURL(LPCWSTR lpszURL)
    {
        wstring wstrCorrectedURL = lpszURL;

        if (starts_with(wstrCorrectedURL, _T("http://www.")))
            goto END;

        if (starts_with(wstrCorrectedURL, _T("www.")))
        {
            wstrCorrectedURL = _T("http://") + wstrCorrectedURL;
            goto END;
        }

        if (!contains(wstrCorrectedURL, _T("://")))
        {
            wstrCorrectedURL = _T("http://") + wstrCorrectedURL;
            goto END;
        }    

    END:
        return wstrCorrectedURL;        
    }

    string CorrectURL(const char* lpszURL)
    {
        string strCorrectedURL = lpszURL;
        wstring wstrURL;
        wstring wstrCorrectedURL;

        if (!UTF8StringToWstring(lpszURL, wstrURL))
            goto END;

        wstrCorrectedURL = CorrectURL(wstrURL.c_str());
        if (!WstringToUTF8String(wstrCorrectedURL, strCorrectedURL))
            goto END;

    END:    
        return strCorrectedURL;
    }

    bool GetSpecialFolderInfo(const int csidl, wstring& wstrDisplayName, wstring& wstrPath)
    {
        bool fRet = false;
        WCHAR szBuffer[512] = {0};
        ITEMIDLIST* pIdl = NULL;
        SHFILEINFO sfi = {0};
        HRESULT hr = NULL;
        DWORD_PTR dwPtr = 0;

        wstrDisplayName.clear();
        wstrPath.clear();

        hr = SHGetFolderLocation(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, &pIdl);
        if (FAILED(hr))
            goto END;

        dwPtr = SHGetFileInfo ((LPCWSTR)pIdl, -1, &sfi, sizeof (sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME);
        if (!dwPtr)
            goto END;

        hr = SHGetFolderPath (NULL, csidl, NULL, SHGFP_TYPE_CURRENT, szBuffer);
        if (FAILED(hr))
            goto END;

        wstrDisplayName = sfi.szDisplayName;
        wstrPath = szBuffer;

        // Done
        fRet = true;

    END:
        return fRet;
    }

    bool GetAppDataPath(wstring& wstrAppDataPath)
    {
        wstring wstrDisplayName;

        wstrAppDataPath.clear();
        return GetSpecialFolderInfo(CSIDL_APPDATA, wstrDisplayName, wstrAppDataPath);
    }    

    bool MakeMyAppDataFolder(LPCWSTR lpszCompanyName, LPCWSTR lpszAppName, wstring& wstrPath)
    {
        bool fRet = false;
        wstring wstrDisplayName;
        wstring wstrAppDataPath;
        wstring wstrCompanyName = lpszCompanyName;
        wstring wstrAppName = lpszAppName;

        if ((!wstrCompanyName.length()) || (!wstrAppName.length()))
            goto END;

        if (!GetSpecialFolderInfo(CSIDL_APPDATA, wstrDisplayName, wstrAppDataPath))
            goto END;

        wstrPath = wstrAppDataPath + _T("\\");
        wstrPath += wstrCompanyName;

        if (!CreateDirectory(wstrPath.c_str(), NULL))
        {
            if (GetLastError() != ERROR_ALREADY_EXISTS)
                goto END;
        }

        wstrPath += _T("\\");
        wstrPath += wstrAppName;

        if (!CreateDirectory(wstrPath.c_str(), NULL))
        {
            if (GetLastError() != ERROR_ALREADY_EXISTS)
                goto END;
        }

        // Done
        fRet = true;

    END:
        return fRet;
    }

    bool CreateDir(LPCWSTR lpszDir)
    {
        bool fRet = false;
        wstring wstrDirExists = lpszDir;
        wstring wstrSubDir;
        vector<wstring> dirToBeCreatedVector;

        if ((!lpszDir) || (!wcslen(lpszDir)))
            goto END;

        // Reverse find the exist folder path.
        // "/", "\", ".", "./", "C:", "C:\" will return TRUE by PathFileExists().
        while (!PathFileExists(wstrDirExists.c_str()))
        {
            size_t nFound = wstrDirExists.find_last_of(_T("/\\"));
            // not found any '/' or '\'
            if (nFound == wstring::npos)
                goto END;
            
            dirToBeCreatedVector.push_back(wstrDirExists.substr(nFound + 1));
            wstrDirExists = wstrDirExists.substr(0, nFound);
        }

        for (size_t i = 0; i < dirToBeCreatedVector.size(); i++)
        {
            wstrSubDir = wstrDirExists + _T("\\");
            wstrSubDir += dirToBeCreatedVector[dirToBeCreatedVector.size() - 1 - i];

            if (!CreateDirectory(wstrSubDir.c_str(), NULL))
                goto END;

            wstrDirExists = wstrSubDir;
        }

        if (PathFileExists(lpszDir))
            fRet = true;
    
    END:
        return fRet;
    }

    void AddBackSlashToPath(LPCWSTR lpszPath, wstring& wstrPath)
    {
        wstring wstrTemp = lpszPath;
        size_t nPos = string::npos;
        
        nPos = wstrTemp.find_last_of(L"\\");
        if ((nPos == string::npos) || (nPos != wstrTemp.length() - 1))
            wstrPath = wstrTemp + L"\\";
        else
            wstrPath = lpszPath;
    }
}