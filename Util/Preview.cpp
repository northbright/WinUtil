//////////////////////////////////////////////////////////////////////////
//
// preview.cpp: Manages video preview.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <assert.h>
#include <ks.h>
#include <ksmedia.h>
#include <shlwapi.h>

#include "Debug.h"
#include "Com.h"
#include "D3DDevice.h"
#include "Camera.h"
#include "Preview.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Util;

#pragma comment(lib, "Mfreadwrite.lib")

//-------------------------------------------------------------------
//  CreateInstance
//
//  Static class method to create the CPreview object.
//  Must call CloseDevice() before SafeRelease(&pPreview) after use or will cause memory leak.
//-------------------------------------------------------------------

HRESULT CPreview::CreateInstance(
    HWND hVideo,        // Handle to the video window.
    HWND hEvent,        // Handle to the window to receive notifications.
    CPreview **ppPlayer // Receives a pointer to the CPreview object.
    )
{
    HRESULT hr = S_OK;
    assert(hVideo != NULL);
    assert(hEvent != NULL);

    if (ppPlayer == NULL)
    {
        return E_POINTER;
    }

    CPreview *pPlayer = new CPreview(hVideo, hEvent);

    // The CPlayer constructor sets the ref count to 1.

    if (pPlayer == NULL)
    {
        return E_OUTOFMEMORY;
    }

    //HRESULT hr = pPlayer->Initialize();

    //if (SUCCEEDED(hr))
    {
        *ppPlayer = pPlayer;
        (*ppPlayer)->AddRef();
    }

    SafeRelease(&pPlayer);
    return hr;
}


//-------------------------------------------------------------------
//  constructor
//-------------------------------------------------------------------

CPreview::CPreview(HWND hVideo, HWND hEvent) :
    m_pReader(NULL),
    m_hwndVideo(hVideo),
    m_hwndEvent(hEvent),
    m_nRefCount(1),
    m_pwszSymbolicLink(NULL),
    m_cchSymbolicLink(0),
    m_pOldCameraPreviewWndProc(NULL),
    m_nCameraId(0),
    m_hdevnotify(NULL)
{
    InitializeCriticalSection(&m_critsec);
}

//-------------------------------------------------------------------
//  destructor
//-------------------------------------------------------------------

CPreview::~CPreview()
{
    CloseDevice();

    m_draw.DestroyDevice();

    DeleteCriticalSection(&m_critsec);
}


//-------------------------------------------------------------------
//  Initialize
//
//  Initializes the object.
//-------------------------------------------------------------------

HRESULT CPreview::Initialize()
{
    HRESULT hr = S_OK;

    hr = m_draw.CreateDevice(m_hwndVideo);

    return hr;
}


//-------------------------------------------------------------------
//  CloseDevice
//
//  Releases all resources held by this object.
//-------------------------------------------------------------------

HRESULT CPreview::CloseDevice()
{
    EnterCriticalSection(&m_critsec);

    SafeRelease(&m_pReader);

    CoTaskMemFree(m_pwszSymbolicLink);
    m_pwszSymbolicLink = NULL;
    m_cchSymbolicLink = 0;

    m_draw.DestroyDevice();

    if (m_hdevnotify)
    {
        UnregisterDeviceNotification(m_hdevnotify);
        m_hdevnotify = NULL;
    }

    if (m_pOldCameraPreviewWndProc)
    {
        SetWindowLongPtr(m_hwndVideo, GWLP_WNDPROC, (LONG_PTR)m_pOldCameraPreviewWndProc);
        m_pOldCameraPreviewWndProc = NULL;
    }

    LeaveCriticalSection(&m_critsec);
    return S_OK;
}


/////////////// IUnknown methods ///////////////

//-------------------------------------------------------------------
//  AddRef
//-------------------------------------------------------------------

ULONG CPreview::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}


//-------------------------------------------------------------------
//  Release
//-------------------------------------------------------------------

ULONG CPreview::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}



//-------------------------------------------------------------------
//  QueryInterface
//-------------------------------------------------------------------

HRESULT CPreview::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CPreview, IMFSourceReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


/////////////// IMFSourceReaderCallback methods ///////////////

//-------------------------------------------------------------------
// OnReadSample
//
// Called when the IMFMediaSource::ReadSample method completes.
//-------------------------------------------------------------------

HRESULT CPreview::OnReadSample(
    HRESULT hrStatus,
    DWORD /* dwStreamIndex */,
    DWORD /* dwStreamFlags */,
    LONGLONG /* llTimestamp */,
    IMFSample *pSample      // Can be NULL
    )
{
    bool fRet = false;
    HRESULT hr = S_OK;
    IMFMediaBuffer *pBuffer = NULL;

    EnterCriticalSection(&m_critsec);

    if (!m_pReader)
    {
        fRet = false;
        Util::DBG_MSG(L"CPreview::OnReadSample() m_pReader = NULL.\r\n");
        goto END;
    }

    if (FAILED(hrStatus))
    {
        hr = hrStatus;
    }

    if (SUCCEEDED(hr))
    {
        if (pSample)
        {
            // Get the video frame buffer from the sample.

            hr = pSample->GetBufferByIndex(0, &pBuffer);

            // Draw the frame.

            if (SUCCEEDED(hr))
            {
                hr = m_draw.DrawFrame(pBuffer);
                if (FAILED(hr))
                {
                    fRet = false;
                    goto END;
                }
            }
            else
            {
                fRet = false;
                goto END;
            }
        }
    }
    else
    {
        fRet = false;
        goto END;
    }

    // Request the next frame.
    if (SUCCEEDED(hr))
    {
        hr = m_pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,
            NULL,   // actual
            NULL,   // flags
            NULL,   // timestamp
            NULL    // sample
            );

        if (FAILED(hr))
        {
            fRet = false;
            goto END;
        }
    }
    else
    {
        fRet = false;
        goto END;
    }

END:
    if (FAILED(hr))
    {
        NotifyError(hr);
    }
    SafeRelease(&pBuffer);

    LeaveCriticalSection(&m_critsec);
    return hr;
}


//-------------------------------------------------------------------
// TryMediaType
//
// Test a proposed video format.
//-------------------------------------------------------------------

HRESULT CPreview::TryMediaType(IMFMediaType *pType)
{
    HRESULT hr = S_OK;

    bool bFound = false;
    GUID subtype = { 0 };

    hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);

    if (FAILED(hr))
    {
        return hr;
    }

    // Do we support this type directly?
    if (m_draw.IsFormatSupported(subtype))
    {
        bFound = true;
    }
    else
    {
        // Can we decode this media type to one of our supported
        // output formats?

        for (DWORD i = 0;  ; i++)
        {
            // Get the i'th format.
            m_draw.GetFormat(i, &subtype);

            hr = pType->SetGUID(MF_MT_SUBTYPE, subtype);

            if (FAILED(hr)) { break; }

            // Try to set this type on the source reader.
            hr = m_pReader->SetCurrentMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                NULL,
                pType
                );

            if (SUCCEEDED(hr))
            {
                bFound = true;
                break;
            }
        }
    }

    if (bFound)
    {
        hr = m_draw.SetVideoType(pType);
    }

    return hr;
}



//-------------------------------------------------------------------
// SetDevice
//
// Set up preview for a specified video capture device.
//-------------------------------------------------------------------

HRESULT CPreview::SetDevice(IMFActivate *pActivate)
{
    HRESULT hr = S_OK;

    IMFMediaSource  *pSource = NULL;
    IMFAttributes   *pAttributes = NULL;
    IMFMediaType    *pType = NULL;

    EnterCriticalSection(&m_critsec);

    // Release the current device, if any.

    hr = CloseDevice();

    Initialize();

    // Create the media source for the device.
    if (SUCCEEDED(hr))
    {
        hr = pActivate->ActivateObject(
            __uuidof(IMFMediaSource),
            (void**)&pSource
            );
    }

    // Get the symbolic link.
    if (SUCCEEDED(hr))
    {
        hr = pActivate->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
            &m_pwszSymbolicLink,
            &m_cchSymbolicLink
            );
    }


    //
    // Create the source reader.
    //

    // Create an attribute store to hold initialization settings.

    if (SUCCEEDED(hr))
    {
        hr = MFCreateAttributes(&pAttributes, 2);
    }
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, true);
    }

    // Set the callback pointer.
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetUnknown(
            MF_SOURCE_READER_ASYNC_CALLBACK,
            this
            );
    }

    if (SUCCEEDED(hr))
    {
        hr = MFCreateSourceReaderFromMediaSource(
            pSource,
            pAttributes,
            &m_pReader
            );
    }

    // Try to find a suitable output type.
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; ; i++)
        {
            hr = m_pReader->GetNativeMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                i,
                &pType
                );

            if (FAILED(hr)) { break; }

            hr = TryMediaType(pType);

            SafeRelease(&pType);

            if (SUCCEEDED(hr))
            {
                // Found an output type.
                break;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        // Ask for the first sample.
        hr = m_pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,
            NULL,
            NULL,
            NULL,
            NULL
            );
    }

    if (FAILED(hr))
    {
        if (pSource)
        {
            pSource->Shutdown();

            // NOTE: The source reader shuts down the media source
            // by default, but we might not have gotten that far.
        }
        CloseDevice();
    }

    SafeRelease(&pSource);
    SafeRelease(&pAttributes);
    SafeRelease(&pType);

    LeaveCriticalSection(&m_critsec);
    return hr;
}

// By Frank
bool CPreview::Set(UINT nCameraId)
{
    bool fRet = false;
    HRESULT hr = S_OK;
    IMFActivate** pCameraDeviceArray = NULL;
    UINT32 nCount = 0;
    DEV_BROADCAST_DEVICEINTERFACE di = {0};

    Util::GetCameraDevices(&pCameraDeviceArray, &nCount);
    if (!nCount)
        goto END;

    if ((nCameraId < 0) || (nCameraId >= nCount))
        goto END;

    hr = SetDevice(pCameraDeviceArray[nCameraId]);
    if (FAILED(hr))
        goto END;

    di.dbcc_size = sizeof(di);
    di.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    di.dbcc_classguid = KSCATEGORY_CAPTURE;

    m_hdevnotify = RegisterDeviceNotification(m_hwndVideo, &di, DEVICE_NOTIFY_WINDOW_HANDLE);
    SetWindowLongPtr(m_hwndVideo, GWLP_USERDATA, (LONG_PTR)this);
    m_pOldCameraPreviewWndProc = (WNDPROC)SetWindowLongPtr(m_hwndVideo, GWLP_WNDPROC, (LONG_PTR)HookedCameraPreviewWndProc);

    m_nCameraId = nCameraId;
    fRet = true;
END:
    Util::FreeCameraDevices(pCameraDeviceArray, nCount);
    return fRet;
}


//-------------------------------------------------------------------
//  ResizeVideo
//  Resizes the video rectangle.
//
//  The application should call this method if the size of the video
//  window changes; e.g., when the application receives WM_SIZE.
//-------------------------------------------------------------------

HRESULT CPreview::ResizeVideo(WORD /*width*/, WORD /*height*/)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critsec);

    hr = m_draw.ResetDevice();

    if (FAILED(hr))
    {
        //MessageBox(NULL, L"ResetDevice failed!", NULL, MB_OK);
        Util::DBG_MSG(L"CPreview::ResizeVideo(): m_draw.ResetDevice() failed.\r\n");
    }

    LeaveCriticalSection(&m_critsec);

    return hr;
}


//-------------------------------------------------------------------
//  CheckDeviceLost
//  Checks whether the current device has been lost.
//
//  The application should call this method in response to a
//  WM_DEVICECHANGE message. (The application must register for
//  device notification to receive this message.)
//-------------------------------------------------------------------

HRESULT CPreview::CheckDeviceLost(DEV_BROADCAST_HDR *pHdr, bool *pbDeviceLost)
{
    DEV_BROADCAST_DEVICEINTERFACE *pDi = NULL;

    if (pbDeviceLost == NULL)
    {
        return E_POINTER;
    }

    *pbDeviceLost = false;

    if (pHdr == NULL)
    {
        return S_OK;
    }

    if (pHdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
    {
        return S_OK;
    }

    pDi = (DEV_BROADCAST_DEVICEINTERFACE*)pHdr;


    EnterCriticalSection(&m_critsec);

    if (m_pwszSymbolicLink)
    {
        if (_wcsicmp(m_pwszSymbolicLink, pDi->dbcc_name) == 0)
        {
            *pbDeviceLost = true;
        }
    }

    LeaveCriticalSection(&m_critsec);

    return S_OK;
}

// Hook Camera Preview WndProc
LRESULT CALLBACK HookedCameraPreviewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Util::CPreview* pPreview = (Util::CPreview*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (!pPreview)
        goto END;

    switch(message)
    {
        case WM_SIZE:
            {
                WORD w = LOWORD(lParam);
                WORD h = HIWORD(lParam);

                if (!pPreview->ResizeVideo(w, h))
                    Util::DBG_MSG(L"HookedCameraPreviewWndProc: WM_SIZE: pPreview->ResizeVideo() failed.");
                else
                    InvalidateRect(hWnd, NULL, false);
            }
            break;

        case WM_ACTIVATE:
            {
                int nActiveState = LOWORD(wParam);
                int nMinimizedState = HIWORD(wParam);

                if (nActiveState == WA_ACTIVE)
                {
                    if (!pPreview->Set(pPreview->m_nCameraId))
                        Util::DBG_MSG(L"HookedCameraPreviewWndProc: WM_ACTIVATE: pPreview->Set() failed.");
                }
            }
            break;

        case WM_DEVICECHANGE:
            {
                PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
                HRESULT hr = S_OK;
                bool bDeviceLost = false;

                // Check if the current device was lost.
                hr = pPreview->CheckDeviceLost(pHdr, &bDeviceLost);

                if (FAILED(hr) || bDeviceLost)
                {
                    Util::DBG_MSG(L"HookedCameraPreviewWndProc: WM_DEVICECHANGE: pPreview->CheckDeviceLost() failed or bDeviceLost = true.\r\n");
                    pPreview->CloseDevice();
                }
            }
            break;

        default:
            break;
    }
    
END:
    if ((pPreview) && (pPreview->m_pOldCameraPreviewWndProc))
        return ::CallWindowProc(pPreview->m_pOldCameraPreviewWndProc, hWnd, message, wParam, lParam);
    else
        return DefWindowProc(hWnd, message, wParam, lParam);
}