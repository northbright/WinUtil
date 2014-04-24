#include <Windows.h>
#include <tchar.h>
#include <devguid.h> // GUID_DEVCLASS_BATTERY
#include <BatClass.h>  // IOCTL_BATTERY_QUERY_INFORMATION

#include "Debug.h"
#include "System.h"
#include "Device.h"
#include "Power.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace Util {

    void EnableSleep(bool fFlag)
    {
        EXECUTION_STATE state = 0;
        EXECUTION_STATE preState = 0;
        if (!fFlag)  // Disalbe Sleep.
        {
            if (IsVistaOrLater())  // Vista or Later support "ES_AWAYMODE_REQUIRED".
                state = ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED | ES_AWAYMODE_REQUIRED;
            else
                state = ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED;
        }
        else  // Enable Sleep.
        {
            state = ES_CONTINUOUS;  // Clear flag.
        }

        preState = SetThreadExecutionState(state);
    }
}