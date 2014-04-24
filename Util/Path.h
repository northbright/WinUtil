
#pragma once

#include <shlobj.h>  // SHGetFolderPath
#include <shellapi.h>  // SHGetFileInfo 

#include <string>
using namespace std;

#pragma comment (lib, "shlwapi.lib")

namespace Util {

    bool GetCurrentModulePath(wstring& strCurrentModulePath);
    bool GetFullPathRelativeToCurrentModule(LPCWSTR lpszRelativePath, wstring& strFullPath);  // "html\index.html" -> "c:\myApp\html\index.html"
    bool GetFileURLRelativeToCurrentModule(LPCWSTR lpszRelativePath, wstring& strURL);  // "html\index.html" -> "file:///c:/myApp/html/index.html"

    bool GetFileNameFromURL(const wstring& wstrURL, wstring& wstrFileName);
    bool GetFileNameFromURL(const string& strUTF8URL, string& strUTF8FileName);

    wstring CorrectURL(LPCWSTR lpszURL);
    string CorrectURL(const char* lpszURL);

    bool GetSpecialFolderInfo(const int csidl, wstring& wstrDisplayName, wstring& wstrPath);
    bool GetAppDataPath(wstring& wstrAppDataPath);
    bool MakeMyAppDataFolder(LPCWSTR lpszCompanyName, LPCWSTR lpszAppName, wstring& wstrPath);

    // Create dir and parent dirs if does not exist.
    bool CreateDir(LPCWSTR lpszDir);

    // Add a back slash at the end of the path if the path does not end with a "\".
    void AddBackSlashToPath(LPCWSTR lpszPath, wstring& wstrPath);
}
