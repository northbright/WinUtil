
#pragma once

#include <string>
using namespace std;

#define MY_CMD_EXE                            _T("MyCmd.exe")

#define MY_CMD_OUTPUT_EVENT                   _T("MyCmdOutputEvent")  // Used to communicate with MyCmd.exe
#define MY_CMD_OUTPUT_SIZE_FILE_MAPPING       _T("MyCmdOutputSizeFileMapping")  // Read / Write Output String Size(in Bytes)
#define MY_CMD_OUTPUT_FILE_MAPPING            _T("MyCmdOutputFileMapping")  // Read / Write Output String of cmd.exe

namespace Util {
    // Run MyCmd.exe with params(Command Line) and get output string.
    // Make sure MyCmd.exe stays in the same folder.
    bool RunCmd(LPCWSTR lpszCmdLine, wstring& strOutput, int nShowCmd = SW_HIDE, DWORD dwWaitTime = 0xFFFFFFFF);

    bool RunCmd(LPCWSTR lpszCmdLine, string& strUTF8Output, int nShowCmd = SW_HIDE, DWORD dwWaitTime = 0xFFFFFFFF);

}