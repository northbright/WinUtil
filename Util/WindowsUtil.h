#pragma once


#include <string>
#include <iostream>
#include <sstream>

using namespace std;

namespace Util {

    bool SetIcon(HWND hWnd, HICON hIcon, HICON hIconSmall);
    bool OpenSelectFolderDialog (HWND hwndOwner, LPCWSTR lpszTitle, wstring& wstrPath);
    void MyGetWindowText(HWND hWnd, wstring& wstr);
}