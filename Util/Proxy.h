#pragma once

/*
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <vector>
*/
#include <string>

using std::wstring;
//using namespace boost;

namespace Util {

    bool IsWindowsProxyEnabled();
    bool GetWindowsProxySetting(wstring& strProxySetting);  // Ex: "http://web-proxy.atl.hp.com:8088"
    //bool GetWindowsProxySetting(wstring& strProxyType, wstring& strHost, unsigned long& nPort);  // "http", "web-proxy.atl.hp.com", 8088

}