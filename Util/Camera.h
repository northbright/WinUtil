#pragma once

#include <string>
#include <vector>
#include <map>

#include "Preview.h"

using namespace std;

// Internal Use
#define WM_CAMERA_MSG            (WM_APP + 200)  // wParam: CameraId, lParam: CAMERA_EVENT enum.

namespace Util {

    typedef enum {
        CameraCaptureEngineError = 0,
        CameraInitialized,
        CameraPreviewStarted,
        CameraPreviewStopped,
        CameraRecordStarted,
        CameraRecordStopped,
        CameraPhotoTaken
    }CAMERA_EVENT;

    // Enumerate all camera devices. Caller should call FreeCameraDevices() to release the malloced IMFActivate* array by system.
    void GetCameraDevices(IMFActivate*** pppDevices, UINT32* pnCount);
    void FreeCameraDevices(IMFActivate** ppDevices, UINT32 nCount);

    // Enumerate all camera's name.
    void GetCameraInfos(vector<wstring>& cameraInfos);

   
}