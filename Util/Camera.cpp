#include <Windows.h>
#include <tchar.h>

#include "Debug.h"
#include "Com.h"
#include "MediaFoundation.h"
#include "Camera.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "Mf.lib")

using namespace Util;

namespace Util {

    void GetCameraDevices(IMFActivate*** pppDevices, UINT32* pnCount)
    {
        HRESULT hr = S_OK;
        IMFAttributes* pAttributes = NULL;
        WCHAR* pszFriendlyName = NULL;
        UINT32 cchName = 0;

        hr = MFCreateAttributes(&pAttributes, 1);
        if (FAILED(hr))
            goto END;

        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (FAILED(hr))
            goto END;

        hr = MFEnumDeviceSources(pAttributes, pppDevices, pnCount);
        if (FAILED(hr))
            goto END;

END:
        SafeRelease(&pAttributes);

        return;
    }

    void FreeCameraDevices(IMFActivate** ppDevices, UINT32 nCount)
    {
        if ((!ppDevices) || (!nCount))
            goto END;

        // Clear malloced IMFActivate* collection.
        for (size_t i = 0; i < nCount; i++)
        {
            SafeRelease(&(ppDevices[i]));
        }

END:
        if (ppDevices)
            CoTaskMemFree(ppDevices);
    }

    void GetCameraInfos(vector<wstring>& cameraInfos)
    {
        HRESULT hr = S_OK;
        IMFActivate** ppDevices = NULL;
        UINT32 nCount = 0;
        WCHAR* pszFriendlyName = NULL;
        UINT32 cchName = 0;

        GetCameraDevices(&ppDevices, &nCount);

        for (size_t i = 0; i < nCount; i++)
        {
            hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pszFriendlyName, &cchName);
            if (SUCCEEDED(hr))
            {
                cameraInfos.push_back(pszFriendlyName);
            }

            CoTaskMemFree(pszFriendlyName);
        }

        FreeCameraDevices(ppDevices, nCount);
    }
}