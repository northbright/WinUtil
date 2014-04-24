// UtilTestApp.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"

#include <Shlwapi.h>
#include <Dbt.h>

#include "UtilTestApp.h"

#include "../Util/Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Util.lib")

#define MAX_LOADSTRING 100

Util::MEMCHECK memCheck;  // 1st Variable in all code block and it will detect memory leak.
Util::COM_THREAD_INIT comThreadInit;  // Make sure that CoUninitialize() be called after all COM object destructor.
Util::MF_THREAD_INIT mfThreadInit;  // Make sure that MFShutdown() be called after all MF object released.

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

Util::Thread g_sdCardTestThread;
Util::Thread g_usbDriveTestThread;
Util::Thread g_emmcTestThread;

Util::ThreadGroup g_threadGroup;
Util::ChmDecompiler g_chmDecompiler;

int g_nThreadCount = 4;

int g_nCameraId = 0;
HWND g_hWndPreview = NULL;

Util::CPreview* g_pPreview = NULL;
Util::MobileBroadbandManager g_mobileBroadbandManager;
Util::Modem g_modem;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
bool				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

static LRESULT CALLBACK CameraPreviewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool FileWriteReadTest(LPCWSTR lpszTestName, LPCWSTR lpszFolder, int nPattern, DWORD dwBlockSize, DWORD dwFileSize, bool* pfStopped, Util::Thread* pThread);

bool SDCardTest(LPVOID lpParams, bool* pfStopped, Util::Thread* pThread);
bool USBDriveTest(LPVOID lpParams, bool* pfStopped, Util::Thread* pThread);
bool EmmcTest(LPVOID lpParams, bool* pfStopped, Util::Thread* pThread);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_UTILTESTAPP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

    //CoInitialize(NULL);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return false;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UTILTESTAPP));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

    // In Win8 Win32 apps, _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF) may cause crash, try to use _CrtDumpMemoryLeaks() instead.
    //_CrtDumpMemoryLeaks();

    //CoUninitialize();

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UTILTESTAPP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_UTILTESTAPP);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);

    // Register CameraPreview class.
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= CameraPreviewWndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInst;
    wcex.hIcon			= NULL;
    wcex.hCursor		= NULL;
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= L"CameraPreview";
    wcex.hIconSm		= NULL;

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
bool InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return false;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return true;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;     

    case WM_CREATE:
        {
            HRESULT hr = S_OK;
            bool fRet = false;

            // Camera Preview
            // Create the object that manages video preview.
            g_nCameraId = 0;
            hr = Util::CPreview::CreateInstance(hWnd, hWnd, &g_pPreview);
            fRet = g_pPreview->Set(g_nCameraId);

            // Thread
            g_sdCardTestThread.Set(SDCardTest, NULL, hWnd);
            g_usbDriveTestThread.Set(USBDriveTest, NULL, hWnd);
            g_emmcTestThread.Set(EmmcTest, NULL, hWnd);
        }
        break;

    case WM_LBUTTONDOWN:
        /*{
            // Memory Leak
            class myClass{
            public:
                myClass() {m_nCount = 0;};
                int m_nCount;
            };

            int* p = new int[5];
            myClass* pClass = new myClass;
            Util::Thread* pThread = new Util::Thread;
        }*/

        /*{
            // Monitor
            Util::Monitor::SetPowerState(2);
        }*/

        /*{
            // Wlan
            bool fRet = true;
            bool fOn = false;
            fRet = Util::GetWlanPowerState(fOn);
            fRet = Util::SetWlanPowerOn(!fOn);

        }*/

        /*{
            // Power
            static bool fFlag = false;
            Util::EnableSleep(fFlag);
            fFlag = !fFlag;
        }*/

        /*{
            // Thread Test
            bool fRet = false;

        }*/
        
        /*{
            // Thread Group Test
            if (!g_threadGroup.IsRunning())
            {
                g_threadGroup.Clear();
                g_threadGroup.Add(SDCardTest);
                g_threadGroup.Add(USBDriveTest);
                g_threadGroup.Add(EmmcTest);

                g_threadGroup.Start();
            }
            else
            {
                g_threadGroup.Stop();
            }
        }*/
        /*{
            // Camera Preview
            bool fRet = false;

            if (g_nCameraId == 0)
                g_nCameraId = 1;
            else
                g_nCameraId = 0;

            fRet = g_pPreview->Set(g_nCameraId);
        }*/

        /*{
            // Path
            bool fRet = false;
            LPCWSTR szDirs[] = {
                _T("."),
                _T("/"),
                _T("\\"),
                _T("./"),
                _T("c:"),
                _T("c:\\"),
                _T("c:\\mydir"),
                _T("c:\\my\\temp\\folder")
            };

            for (size_t i = 0; i < sizeof(szDirs) / sizeof(szDirs[0]); i++)
            fRet = Util::CreateDir(szDirs[i]);
        }*/

        /*{
        // Mobile Broadband
        bool fRet = false;
        bool fOn = false;
        int nCount = 0;
        ULONG ulStrength = 0;
        static bool fStarted = false;
        wstring str;
        WCHAR szBuffer[256] = {0};

        nCount = g_mobileBroadbandManager.GetDeviceCount();
        if (!nCount)
        str = L"No Mobile Braodband devices detected.";
        else
        {
        wsprintf(szBuffer, L"%d Mobile Broadband devices detected. ", nCount);
        str = szBuffer;
        }

        for (int i = 0; i < nCount; i++)
        {
        fRet = g_mobileBroadbandManager.GetPowerState(i, fOn);
        fRet = g_mobileBroadbandManager.SetPowerState(i, !fOn);
        fRet = g_mobileBroadbandManager.GetPowerState(i, fOn);

        wsprintf(szBuffer, L"id: %d, radioOn: %d,  ", i, fOn);
        str += szBuffer;

        if (fOn)
        {
        fRet = g_mobileBroadbandManager.GetSignalStrength(i, ulStrength);
        if (fRet)
        wsprintf(szBuffer, L"strength: %d,  ", ulStrength);
        else
        wsprintf(szBuffer, L"GetSignalStrength() failed,  ", ulStrength);

        str += szBuffer;
        }
        }

        SetWindowText(hWnd, str.c_str());

        if (!fStarted)
        SetTimer(hWnd, 0, 1000, NULL);
        }*/

        /*{
            // Modem
            bool fRet = false;
            wstring str;
            vector<string> resultArray;
            Util::MODEM_INFO modemInfo;

            fRet = g_modem.Set(L"COM7:");
            //fRet = g_modem.SendATCommand("at", resultArray);
            //fRet = g_modem.SendATCommand("ati", resultArray);
            //fRet = g_modem.GetInfo(modemInfo);
            fRet = g_modem.GetIMSI(str);
            //MessageBox(hWnd, str.c_str(), L"IMSI", MB_OK);
        }*/

        /*{
        // Bluetooth
        bool fRet = false;
        bool fOn = false;
        wstring strDllPath;
        WCHAR szBuffer[256] = {0};

        fRet = Util::GetBluetoothSupportDllPath(strDllPath);
        fRet = Util::GetBluetoothPowerState(fOn);
        if (fRet)
        {
        wsprintf(szBuffer, L"GetBluetoothPowerState() succeeded. fOn = %d.", fOn);
        MessageBox(NULL, szBuffer, L"OK", MB_OK);
        }

        fRet = Util::SetBluetoothPowerOn(!fOn);
        if (fRet)
        {
        wsprintf(szBuffer, L"SetBluetoothPowerOn() succeeded. !fOn = %d.", !fOn);
        MessageBox(NULL, szBuffer, L"OK", MB_OK);
        }
        }*/

        /*{
            // Device
            bool fRet = false;
            HANDLE hDevice = NULL;
            //{6AC27878-A6FA-4155-BA85-F98F491D4F33}
            static const GUID GUID_PORTABLE_DEVICE = 
            { 0x6AC27878, 0xA6FA, 0x4155, { 0xBA, 0x85, 0xF9, 0x8F, 0x49, 0x1D, 0x4F, 0x33 } };

            // portable Device
            vector<wstring> paths;
            Util::GetRemovableStoragePaths(paths);
            Util::GetUSBDiskPaths(paths);

            // GetDeviceHandleByInterfaceGUID()
            wstring strPath;
            fRet = Util::GetDevicePathByInterfaceGUID(GUID_PORTABLE_DEVICE, 0, strPath);
            if (fRet)
                Util::DBG_MSG(L"GetDevicePathByInterfaceGUID(): %s\r\n", strPath.c_str());
        }*/

        /*{
            // Power
            //bool fRet = false;
        }*/

        /*{
            // String
            SYSTEMTIME st = {0};
            Util::WstringToTime(L"2012/07/12 17:31", st);
            Util::WstringToTime(L"2012/07/12 17:31:24", st);

            Util::StringToTime("2012/07/13 17:37", st);
            Util::StringToTime("2012/07/13 17:37:30", st);
        }*/

        /*{
            // WMI
            wstring str;
            bool fRet = false;
            vector<Util::WMI::Obj> objs;
            LPCWSTR lpszNames[] = {
                L"Win32_DiskDrive",
                L"Win32_BIOS",
                L"Win32_PhysicalMemory",
                L"Win32_Processor",
                L"Win32_NetworkAdapter",
                L"Win32_Battery",
                L"Win32_CurrentProbe",
                L"Win32_PortableBattery",
                L"Win32_VoltageProbe"
            };

            for (size_t i = 0; i < sizeof(lpszNames) / sizeof(lpszNames[0]); i++)
            {
                Util::WMI::Namespace ns(L"root\\CIMV2");
                fRet = ns.GetObjs(lpszNames[i], objs);
                if (fRet)
                {
                    for (size_t j = 0; j < objs.size(); j++)
                    {
                        wstring str;
                        objs[j].GetProps(str);
                        Util::DBG_MSG(str.c_str());
                        Util::DBG_MSG(L"\n");
                    }
                }
                else
                {
                    Util::DBG_MSG(L"GetObjs() of %s failed.", lpszNames[i]);
                    Util::DBG_MSG(L"\n");
                }
            }
        }*/

        /*{
            // File
            set<wstring> wstrFileSet;
            Util::SearchFilesByExtName(L"D:\\", L".mp3;.chm", wstrFileSet);
            size_t n = wstrFileSet.size();
        }*/
        
        {
            // Chm
            set<wstring> wstrChmFileSet;
            LPCWSTR lpszChmFiles[] = {
                L"D:\\Chm\\.NETFramework40BlendSDK.chm",
                L"D:\\Chm\\aclui.chm",
                L"D:\\Chm\\ADO210.chm",
                L"D:\\Chm\\applocker_help.chm",
                L"D:\\Chm\\appverif.chm",
                L"D:\\Chm\\AtlTraceTool8.chm",
                L"D:\\Chm\\authfw.chm",
                L"D:\\Chm\\authm.chm"
            };
            LPCWSTR lpszOutDir = L"D:\\DecompiledChm";

            for (size_t i = 0; i < sizeof(lpszChmFiles) / sizeof(lpszChmFiles[0]); i++)
            {
                wstrChmFileSet.insert(lpszChmFiles[i]);
            }
            //g_chmDecompiler.Set(wstrChmFileSet, lpszOutDir, hWnd);
            g_chmDecompiler.Set(L"D:\\", L"D:\\DecompiledChm", true, hWnd);
            g_chmDecompiler.Start();
        }

    break;

    case WM_TIMER:
        {
            {
                // Mobile Broadband
                bool fRet = false;
                bool fOn = false;
                int nCount = 0;
                ULONG ulStrength = 0;
                wstring str;
                WCHAR szBuffer[256] = {0};

                nCount = g_mobileBroadbandManager.GetDeviceCount();
                if (!nCount)
                    str = L"No Mobile Braodband devices detected.";
                else
                {
                    wsprintf(szBuffer, L"%d Mobile Broadband devices detected. ", nCount);
                    str = szBuffer;
                }

                for (int i = 0; i < nCount; i++)
                {
                    fRet = g_mobileBroadbandManager.GetPowerState(i, fOn);

                    wsprintf(szBuffer, L"id: %d, radioOn: %d,  ", i, fOn);
                    str += szBuffer;

                    if (fOn)
                    {
                        fRet = g_mobileBroadbandManager.GetSignalStrength(i, ulStrength);
                        if (fRet)
                            wsprintf(szBuffer, L"strength: %d,  ", ulStrength);
                        else
                            wsprintf(szBuffer, L"GetSignalStrength() failed,  ", ulStrength);

                        str += szBuffer;
                    }
                }
                SetWindowText(hWnd, str.c_str());
            }
        }
        break;

    case WM_THREAD_PROGRESS_CHANGED:
        {
            DWORD dwThreadId = (DWORD)wParam;
            int nProgress = (int)lParam;

            //Util::DBG_MSG(L"WM_THREAD_PROGRESS_CHANGED: Id: %d, progress: %d.\r\n", dwThreadId, nProgress);
        }
        break;

    case WM_THREAD_SUCCEEDED:
        {
            DWORD dwThreadId = (DWORD)wParam;
            int nProgress = (int)lParam;

            Util::DBG_MSG(L"WM_THREAD_SUCCEEDED: Id: %d.\r\n", dwThreadId);
        }
        break;

    case WM_THREAD_FAILED:
        {
            DWORD dwThreadId = (DWORD)wParam;
            int nProgress = (int)lParam;

            Util::DBG_MSG(L"WM_THREAD_FAILED: Id: %d.\r\n", dwThreadId);
        }
        break;

    case WM_THREAD_STOPPED:
        {
            DWORD dwThreadId = (DWORD)wParam;
            int nProgress = (int)lParam;

            Util::DBG_MSG(L"WM_THREAD_STOPPED: Id: %d.\r\n", dwThreadId);
        }
        break;

    case WM_THREAD_GROUP_PROGRESS_CHANGED:
        {
            DWORD dwGroupId = (DWORD)wParam;
            int nProgress = (int)lParam;

            Util::DBG_MSG(L"WM_THREAD_GROUP_PROGRESS_CHANGED: Id: %d, progress = %d.\r\n", dwGroupId, nProgress);
        }
        break;

    case WM_THREAD_GROUP_STOPPED:
        {
            DWORD dwGroupId = (DWORD)wParam;
            int nProgress = (int)lParam;

            Util::DBG_MSG(L"WM_THREAD_GROUP_STOPPED: Id: %d.\r\n", dwGroupId);
        }
        break;

    case WM_THREAD_GROUP_EXITED:
        {
            DWORD dwGroupId = (DWORD)wParam;
            int nProgress = (int)lParam;

            Util::DBG_MSG(L"WM_THREAD_GROUP_EXITED: Id: %d.\r\n", dwGroupId);
        }
        break;

    case WM_SIZE:
        {
            // Camera Preview
            WORD w = LOWORD(lParam);
            WORD h = HIWORD(lParam);
        }
        break;

    case WM_ACTIVATE:
        {
            int nActiveState = LOWORD(wParam);
            int nMinimizedState = HIWORD(wParam);
            
        }
        break;

    case WM_DEVICECHANGE:
        {
            PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
            HRESULT hr = S_OK;
        }
        break;

    case WM_DESTROY:
        KillTimer(hWnd, 0);
        PostQuitMessage(0);
        g_pPreview->CloseDevice();  // Must call CloseDevice() before Release() or will cause memory leak.
        SafeRelease(&g_pPreview);

        g_chmDecompiler.Stop();
        Sleep(500);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK CameraPreviewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)true;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)true;
		}
		break;
	}
	return (INT_PTR)false;
}

bool FileWriteReadTest(LPCWSTR lpszTestName, LPCWSTR lpszFolder, int nPattern, DWORD dwBlockSize, DWORD dwFileSize, bool* pfStopped, Util::Thread* pThread)
{
    bool fRet = false;
    HANDLE hWriteFile = NULL;
    HANDLE hReadFile = NULL;
    WCHAR szFile[256] = {0};
    int nProgress = 0;
    UINT i = 0;
    wstring wstrFolder;
    DWORD dwAllWritten = 0;
    DWORD dwWritten = 0;
    DWORD dwAllRead = 0;
    DWORD dwRead = 0;
    DWORD dwToWrite = 0;
    BYTE* pWriteBlock = NULL;
    BYTE* pReadBlock = NULL;
    DWORD dwErr = 0;
    SYSTEMTIME st = {0};
    UINT nLoop = 10;

    if ((!lpszFolder) || (!dwBlockSize) || (!dwFileSize))
        goto END;

    if (dwBlockSize > dwFileSize)
        dwBlockSize = dwFileSize;

    pWriteBlock = new BYTE[dwBlockSize];
    if (!pWriteBlock)
        goto END;

    memset(pWriteBlock, nPattern, sizeof(BYTE) * dwBlockSize);

    pReadBlock = new BYTE[dwBlockSize];
    if (!pReadBlock)
        goto END;

    memset(pReadBlock, 0, sizeof(BYTE) * dwBlockSize);

    CreateDirectory(lpszFolder, NULL);

    while (i < nLoop)
    {
        if (*pfStopped)
            goto END;
        
        if (PathFileExists(szFile))
        {
            if (!DeleteFile(szFile))
                goto END;
        }

        GetLocalTime(&st);
        wsprintf(szFile, L"%s\\%04d-%02d-%02d_%02d_%02d_%02d.bin", lpszFolder, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

        hWriteFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
        if (hWriteFile == INVALID_HANDLE_VALUE)
            goto END;

        hReadFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hReadFile == INVALID_HANDLE_VALUE)
            goto END;

        dwAllWritten = 0;
        dwAllRead = 0;

        while (dwAllWritten < dwFileSize)
        {
            if (*pfStopped)
                goto END;

            if (dwFileSize - dwAllWritten > dwBlockSize)
                dwToWrite = dwBlockSize;
            else
                dwToWrite = dwFileSize - dwWritten;

            if (!WriteFile(hWriteFile, pWriteBlock, dwBlockSize, &dwWritten, NULL))
                goto END;

            FlushFileBuffers(hWriteFile);
            dwAllWritten += dwWritten;

            // Read File
            if (!ReadFile(hReadFile, pReadBlock, dwWritten, &dwRead, NULL))
                goto END;

            dwAllRead += dwRead;
        }
        
        CloseHandle(hWriteFile);
        hWriteFile = NULL;

        CloseHandle(hReadFile);
        hReadFile = NULL;

        Util::DBG_MSG(L"%s: write / read %s done. Loop = %d\n", lpszTestName, szFile, i);

        i++;

        nProgress = i * 100 / nLoop;
        pThread->UpdateProgress(nProgress);
    }

    fRet = true;
END:
    if (pWriteBlock)
        delete[] pWriteBlock;

    if (pReadBlock)
        delete[] pReadBlock;

    if (hWriteFile && hWriteFile != INVALID_HANDLE_VALUE)
        CloseHandle(hWriteFile);

    if (hReadFile && hReadFile != INVALID_HANDLE_VALUE)
        CloseHandle(hReadFile);

    if (PathFileExists(szFile))
        DeleteFile(szFile);

    if (*pfStopped)
        Util::DBG_MSG(L" ********** %s stopped. **********\n", lpszTestName);
    else if (fRet)
        Util::DBG_MSG(L" ********** %s succeeded. **********\n", lpszTestName);
    else
        Util::DBG_MSG(L" ********** %s failed. **********\n", lpszTestName);

    return fRet;
}

bool SDCardTest(LPVOID lpParams, bool* pfStopped, Util::Thread* pThread)
{
    int nPattern = 0;

    sscanf("0x5a", "%x", &nPattern);
    return FileWriteReadTest(L"SDCardTest", L"G:\\SDCardTest", nPattern, 65535, 554432, pfStopped, pThread);
}

bool USBDriveTest(LPVOID lpParams, bool* pfStopped, Util::Thread* pThread)
{
    int nPattern = 0;

    sscanf("0x5a", "%x", &nPattern);
    return FileWriteReadTest(L"USBDriveTest", L"H:\\USBDriveTest", nPattern, 65535, 554432, pfStopped, pThread);
}

bool EmmcTest(LPVOID lpParams, bool* pfStopped, Util::Thread* pThread)
{
    int nPattern = 0;

    sscanf("0x5a", "%x", &nPattern);
    return FileWriteReadTest(L"EmmcTest", L"C:\\EmmcTest", nPattern, 65535, 554432, pfStopped, pThread);
}