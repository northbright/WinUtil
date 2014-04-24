
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include <string>
using namespace std;

#define MY_CMD_EXE                            _T("MyCmd.exe")

#define MY_CMD_OUTPUT_EVENT                   _T("MyCmdOutputEvent")  // Used to communicate with MyCmd.exe
#define MY_CMD_OUTPUT_SIZE_FILE_MAPPING       _T("MyCmdOutputSizeFileMapping")  // Read / Write Output String Size(in Bytes)
#define MY_CMD_OUTPUT_FILE_MAPPING            _T("MyCmdOutputFileMapping")  // Read / Write Output String of cmd.exe

// argv[1]: Command Line of cmd.exe
// Ex: dir

int _tmain(int argc, TCHAR* argv[])
{
    FILE* pPipe = NULL;
    TCHAR* lpszCmdLine = NULL;
    wstring strCmdLine;
    TCHAR buffer[256 + 1] = {0};
    HANDLE hEvent = INVALID_HANDLE_VALUE;
    HANDLE hFileOutputSize = NULL;
    HANDLE hFile = NULL;
    DWORD* pdwOutputSize = NULL;
    DWORD dwOutputSize = 0;
    wstring strOutput;
    BYTE* pOutput = NULL;
       
    if (argc <= 1)
        goto END;
    
    for (int i = 1; i < argc; i++)
    {
        wstring strTmp = argv[i];
        if (strTmp.find(_T(" ")) != -1)
        {
            strCmdLine += _T("\"");
            strCmdLine += argv[i];
            strCmdLine += _T("\"");
        }
        else    
            strCmdLine += argv[i];  
                
        if (i != argc - 1)
            strCmdLine += _T(" "); 
    }

    pPipe = _wpopen(strCmdLine.c_str(), _T("r"));
    if (!pPipe)
        goto END;
    
    //if (!ftell(pPipe))
    //    goto END;
        
    while (fgetws(buffer, 256, pPipe))
    {
        strOutput += buffer;
        _tprintf(buffer);

        OutputDebugString(buffer);
    }

    hEvent = CreateEvent(NULL, FALSE, FALSE, MY_CMD_OUTPUT_EVENT);
    if (hEvent == INVALID_HANDLE_VALUE)
        goto END;
    
    hFileOutputSize = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DWORD), MY_CMD_OUTPUT_SIZE_FILE_MAPPING);
    if (!hFileOutputSize)
        goto END;
    
    pdwOutputSize = (DWORD*)MapViewOfFile(hFileOutputSize, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD));
    if (!pdwOutputSize)
        goto END;
    
    dwOutputSize = (strOutput.length() + 1) * sizeof (TCHAR);
    *pdwOutputSize = dwOutputSize;
        
    hFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwOutputSize, MY_CMD_OUTPUT_FILE_MAPPING);
    if (!hFile)
        goto END;
    
    pOutput = (BYTE*)MapViewOfFile(hFile, FILE_MAP_ALL_ACCESS, 0, 0, dwOutputSize);
    memset(pOutput, 0, dwOutputSize);
    if (strOutput.length())
        _tcsncpy((TCHAR*)pOutput, strOutput.c_str(), strOutput.length());
        
    // ------- Done -----------
    SetEvent(hEvent);
    OutputDebugString(_T("Done.\n"));
    
END:
    if (pPipe)
        _pclose(pPipe);
           
    if ((hEvent) && (hEvent != INVALID_HANDLE_VALUE))  
        CloseHandle(hEvent);

    if (pdwOutputSize)
        UnmapViewOfFile(pdwOutputSize);
    
    if (pOutput)
        UnmapViewOfFile(pOutput);

    if (hFileOutputSize)
        CloseHandle(hFileOutputSize);
        
    if (hFile)
        CloseHandle(hFile);    
   
    return 0;
}    