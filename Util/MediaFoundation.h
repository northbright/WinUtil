#pragma once

#include <Mfapi.h>

namespace Util {

    typedef struct _MF_THREAD_INIT {
        _MF_THREAD_INIT() { HRESULT hr = MFStartup(MF_VERSION); }
        ~_MF_THREAD_INIT() { HRESULT hr = MFShutdown(); }
    }MF_THREAD_INIT;
}