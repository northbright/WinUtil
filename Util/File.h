#pragma once

#include <string>
#include <set>

#include "Thread.h"

using namespace std;

namespace Util {
    // Search specified files by ext names
    // params:
    //     lpszDir: Dir to be searched.
    //     lpszExtFilter: Ex: ".mp3;.wma". Multiple ext names are splitted by ';' or ','.
    //     wstrFileSet: Output matched files.
    //     fSearchSubDir: Search subdir or not.
    //     pfStopped: Used to stopped func.
    void SearchFilesByExtName(LPCWSTR lpszDir, LPCWSTR lpszExtFilter, set<wstring>& wstrFileSet, bool fSearchSubDir = false, bool* pfStopped = NULL);
}

