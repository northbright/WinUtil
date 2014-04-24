#pragma once

// Release IXX Pointer.
template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

namespace Util {
    // For some user's global COM object destructor will call Release() after CoUninitialize() in the caller thread(Ex. in winmain), the application will crash.
    // How to avoid this:
    // 1. Place all global COM objects in ONLY one cpp file. Or you may use new / delete for COM objects.
    // 2. Place a COM_THREAD_INIT global var before users' COM object to avoid this.
    // Ex:
    // COM_THREAD_INIT g_comThreadInit;  // place it before COM object so that the CoUninitialize() will be called after user's COM object destructor(xxx.Release()).
    // MyComObject g_object;
    // int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
    // {
    //     // CoInitialize(NULL);  // Remove CoInitialize() and CoUninitialize() in winmain(), the global COM_THREAD_INIT will call CoUninitialize() automatically in the destructor.
    //     g_object.xxxx();
    //     ....
    //     // CoUninitialize();
    //     return 0;
    // }

    typedef struct _COM_THREAD_INIT {
        _COM_THREAD_INIT() { CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); }
        ~_COM_THREAD_INIT() { CoUninitialize(); }
    }COM_THREAD_INIT;

}