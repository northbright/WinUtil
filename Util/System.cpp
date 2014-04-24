#include <Windows.h>
#include <tchar.h>

#include "Debug.h"
#include "System.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Util {

    bool IsVistaOrLater()
    {
        bool fRet = false;
        OSVERSIONINFO osVer = {0};
        osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (!GetVersionEx(&osVer))
            goto END;

        if (osVer.dwMajorVersion >= 6)
            fRet = true;
        else
            fRet = false;
    END:
        return fRet;
    }

}