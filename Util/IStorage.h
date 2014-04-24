#pragma once

#include "Thread.h"

// -------------- Begin Defination for IITStorage Interface --------------
typedef struct _ITS_Control_Data
{
    UINT cdwControlData;
    UINT adwControlData[1];
} ITS_Control_Data, *PITS_Control_Data;  

typedef enum ECompactionLev 
{ 
    COMPACT_DATA = 0, 
    COMPACT_DATA_AND_PATH
};

DECLARE_INTERFACE_(IITStorage, IUnknown)
{
    STDMETHOD(StgCreateDocfile) 
        (const WCHAR* pwcsName, DWORD grfMode, DWORD reserved, IStorage** ppstgOpen) PURE;
    STDMETHOD(StgCreateDocfileOnILockBytes) 
        (ILockBytes* plkbyt, DWORD grfMode, DWORD reserved, IStorage ** ppstgOpen) PURE;
    STDMETHOD(StgIsStorageFile) 
        (const WCHAR* pwcsName) PURE;
    STDMETHOD(StgIsStorageILockBytes) 
        (ILockBytes* plkbyt) PURE;
    STDMETHOD(StgOpenStorage) 
        (const WCHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage ** ppstgOpen) PURE;
    STDMETHOD(StgOpenStorageOnILockBytes)
        (ILockBytes* plkbyt, IStorage* pStgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage ** ppstgOpen ) PURE;
    STDMETHOD(StgSetTimes)
        (WCHAR const* lpszName, FILETIME const* pctime, FILETIME const* patime, FILETIME const* pmtime) PURE;
    STDMETHOD(SetControlData)
        (PITS_Control_Data pControlData) PURE;
    STDMETHOD(DefaultControlData)
        (PITS_Control_Data *ppControlData) PURE;
    STDMETHOD(Compact)
        (const WCHAR* pwcsName, ECompactionLev iLev) PURE;
};

// -------------- End Defination for IITStorage Interface --------------


namespace Util {

    UINT GetStreamCount(IStorage* pStg);
    UINT GetFileCountInStorageFile(LPCTSTR ptszFileName);

    // Output all streams in a storage file.
    // Params:
    //    ptszFile: Storage file to be output.
    //    ptszPath: Output folder to store streams(files).
    //    pfStopped: Used to stop function while call this function in a thread.
    //    pThread:  Used to update thread progress / notify host window while call this function in a thread.
    // Return:
    //    true if succeeded or false if failed.
    bool OutputStreamsInStorageFile(LPCTSTR ptszFile, LPCTSTR ptszPath, bool* pfStopped = NULL, Thread* pThread = NULL);

}