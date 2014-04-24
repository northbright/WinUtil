#pragma once

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// How to use:
// 1. Include "Debug.h" in cpp.
// 2. #define new DEBUG_NEW after all include.
// Ex:
// // source.cpp
// #include "Debug.h"
// ...... // other includes
// #ifdef _DEBUG
// #define new DEBUG_NEW
// #endif

#ifdef _DEBUG
// -----------------------------------------------------------
void* operator new(size_t nSize, const char * lpszFileName, int nLine);
#define DEBUG_NEW new(__FILE__, __LINE__)

//#define MALLOC_DBG(x) _malloc_dbg(x, 1, __FILE__, __LINE__);
//#define malloc(x) MALLOC_DBG(x)
// -----------------------------------------------------------
#endif // _DEBUG

#include <Windows.h>
#include <tchar.h>

// For C++ struct / class, if it free members in the destructor, the destructor will be called after _CrtDumpMemoryLeaks() is called(even it's in the end of code block / function).
// And you will get the wrong memory leak detected.
// Ex:
//class Obj {
//public:
//    Obj() { m_p = new int; };
//    ~Obj() { delete m_p; };
//
//    int* m_p;
//};
//
//Obj obj;
//
//int main() {
//    _CrtDumpMemoryLeaks();  // Wront memory leak detected even m_p is freed in the destructor.
//    return 0;
//}

// The workaround is:
// Create a MEMCHECK variable as the first variable in the code block / function and it will get destroyed last. See jsmith's answer in http://www.cplusplus.com/forum/general/9614/
// Ex:
//class Obj {
//public:
//    Obj() { m_p = new int; };
//    ~Obj() { delete m_p; };
//
//    int* m_p;
//};
//
//typedef struct _MEMCHECK {
//    ~_MEMCHECK()
//    {
//        _CrtDumpMemoryLeaks();
//    }
//}MEMCHECK;
//
//MEMCHECK memCheck;  // Make the MEMCHECK variable the 1st variable in the code block, so it will be destroyed the last and _CrtDumpMemoryLeaks() will be called the last. Then you won't see the wrong memory leak detected.
//Obj obj;
//
//int main() {
//    return 0;  // _CrtDumpMemoryLeaks() will be called when memCheck is destroyed the last.
//}

namespace Util {

    typedef struct _MEMCHECK {
        _MEMCHECK()
        {
            // Detect CRT Memory Leak
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
        }

        ~_MEMCHECK()
        {
            _CrtDumpMemoryLeaks();
        }
    }MEMCHECK;

    // Output Debug Message.
    void DBG_MSG(LPCWSTR format, ...);
}
