#pragma once

#include <string>

using namespace std;

namespace Util {

    class CriticalSection {
    public:
        CriticalSection();
        ~CriticalSection();

        void Enter();
        void Leave();

        CRITICAL_SECTION m_cs;
    };
}