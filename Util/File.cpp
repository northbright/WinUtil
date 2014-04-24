#include <windows.h>
#include <tchar.h>

#include "Debug.h"
#include "Path.h"
#include "FileExtFilter.h"
#include "File.h"

using namespace Util;

namespace Util {

    void SearchFilesByExtName(LPCWSTR lpszDir, LPCWSTR lpszExtFilter, set<wstring>& wstrFileSet, bool fSearchSubDir,  bool* pfStopped)
    {
        WIN32_FIND_DATA fd = {0};
        HANDLE hFind = INVALID_HANDLE_VALUE;
        CFileExtFilter filter(lpszExtFilter);
        wstring wstrDir;
        wstring wstrFile;
        wstring wstrFileToFind;
        size_t nPos = string::npos;

        if ((!lpszDir) || (!wcslen(lpszDir)))
            goto END;

        wstrFileSet.clear();
        
        AddBackSlashToPath(lpszDir, wstrDir);
        wstrFileToFind = wstrDir + L"*.*";

        hFind = FindFirstFile(wstrFileToFind.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE)
            goto END;

        do
        {
            if ((pfStopped) && (*pfStopped))
                goto END;

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                wstring wstrSubDir;
                set<wstring> wstrSubDirFileSet;

                if (!fSearchSubDir)
                    continue;

                wstrSubDir = fd.cFileName;
                if ((wstrSubDir == L".") || (wstrSubDir == L".."))
                    continue;

                wstrSubDir = wstrDir + fd.cFileName;
                // Recusive
                SearchFilesByExtName(wstrSubDir.c_str(), lpszExtFilter, wstrSubDirFileSet, fSearchSubDir, pfStopped);
                for (set<wstring>::iterator it = wstrSubDirFileSet.begin(); it != wstrSubDirFileSet.end(); it++)
                {
                    wstrFileSet.insert(*it);
                }
            }
            else
            {
                if (filter.IsFileMatch(fd.cFileName))
                {
                    wstrFile = wstrDir + fd.cFileName;
                    wstrFileSet.insert(wstrFile);
                }
            }
        }while (FindNextFile(hFind, &fd));

    END:
        if ((hFind) && (hFind != INVALID_HANDLE_VALUE))
            FindClose(hFind);
    }

}