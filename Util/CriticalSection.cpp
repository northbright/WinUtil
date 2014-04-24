#include <windows.h>
#include <tchar.h>

#include "Debug.h"
#include "CriticalSection.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Util;

CriticalSection::CriticalSection()
{
    InitializeCriticalSection(&m_cs);
}

CriticalSection::~CriticalSection()
{
    DeleteCriticalSection(&m_cs);
}

void CriticalSection::Enter()
{
    EnterCriticalSection(&m_cs);
}

void CriticalSection::Leave()
{
    LeaveCriticalSection(&m_cs);
}

