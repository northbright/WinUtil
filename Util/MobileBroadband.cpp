#include <Windows.h>
#include <tchar.h>

#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp> // replace_all()

#include "Debug.h"
#include "String.h"
#include "Com.h"
#include "MobileBroadband.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Util;
using namespace boost;

#pragma comment(lib, "mbnapi_uuid.lib")

MobileBroadbandManager::MobileBroadbandManager()
{
    HRESULT hr = S_OK;

    m_pManager = NULL;

    hr = CoCreateInstance(CLSID_MbnInterfaceManager, NULL, CLSCTX_SERVER, IID_IMbnInterfaceManager, (LPVOID*)&m_pManager);
    if (FAILED(hr))
        goto END;

END:
    return;
}

MobileBroadbandManager::~MobileBroadbandManager()
{
    SafeRelease(&m_pManager);
}

int MobileBroadbandManager::GetDeviceCount()
{
    int nCount = 0;
    HRESULT hr = S_OK;
    SAFEARRAY *psa = NULL;
    LONG lLower = 0;
    LONG lUpper = 0;

    if (!m_pManager)
        goto END;

    hr = m_pManager->GetInterfaces(&psa);
    if (FAILED(hr))
        goto END;

    hr = SafeArrayGetLBound(psa, 1, &lLower);
    if (FAILED(hr))
        goto END;

    hr = SafeArrayGetUBound(psa, 1, &lUpper);
    if (FAILED(hr))
        goto END;

    nCount = (int)(lUpper - lLower + 1);

END:
    SafeArrayDestroy(psa);
    return nCount;
} 

bool MobileBroadbandManager::GetPowerState(int nDeviceId, bool& fOn)
{
    bool fRet = false;
    HRESULT hr = S_OK;
    IMbnInterface* pInterface = NULL;
    IMbnRadio* pRadio = NULL;
    SAFEARRAY *psa = NULL;
    LONG lLower = 0;
    LONG lUpper = 0;
    MBN_READY_STATE readyState;
    MBN_RADIO radioState;
    ULONG ulRequestId = 0;

    if (!m_pManager)
        goto END;

    hr = m_pManager->GetInterfaces(&psa);
    if (FAILED(hr))
        goto END;

    for (LONG l = lLower; l <= lUpper; l++)
    {
        if (l != (LONG)nDeviceId)
            continue;

        hr = SafeArrayGetElement(psa, &l, (void*)(&pInterface));
        if (FAILED(hr))
            goto END;

        hr = pInterface->GetReadyState(&readyState);
        if (FAILED(hr))
            goto END;

        hr = pInterface->QueryInterface(IID_IMbnRadio, (void**)&pRadio);
        if (FAILED(hr))
            goto END;

        hr = pRadio->get_SoftwareRadioState(&radioState);
        if (FAILED(hr))
            goto END;

        fOn = (radioState == MBN_RADIO::MBN_RADIO_ON) ? true : false;

        SafeRelease(&pRadio);
        SafeRelease(&pInterface);

        fRet = true;

        break;
    }

END:
    SafeArrayDestroy(psa);
    SafeRelease(&pRadio);
    SafeRelease(&pInterface);

    return fRet;   
}

bool MobileBroadbandManager::SetPowerState(int nDeviceId, bool fOn)
{
    bool fRet = false;
    HRESULT hr = S_OK;
    IMbnInterface* pInterface = NULL;
    IMbnRadio* pRadio = NULL;
    SAFEARRAY *psa = NULL;
    LONG lLower = 0;
    LONG lUpper = 0;
    MBN_READY_STATE readyState;
    MBN_RADIO radioState;
    ULONG ulRequestId = 0;

    if (!m_pManager)
        goto END;

    hr = m_pManager->GetInterfaces(&psa);
    if (FAILED(hr))
        goto END;

    for (LONG l = lLower; l <= lUpper; l++)
    {
        if (l != (LONG)nDeviceId)
            continue;

        hr = SafeArrayGetElement(psa, &l, (void*)(&pInterface));
        if (FAILED(hr))
            goto END;

        hr = pInterface->GetReadyState(&readyState);
        if (FAILED(hr))
            goto END;

        hr = pInterface->QueryInterface(IID_IMbnRadio, (void**)&pRadio);
        if (FAILED(hr))
            goto END;

        radioState = (fOn) ? MBN_RADIO::MBN_RADIO_ON : MBN_RADIO::MBN_RADIO_OFF;
        hr = pRadio->SetSoftwareRadioState(radioState, &ulRequestId);
        if (FAILED(hr))
            goto END;

        SafeRelease(&pRadio);
        SafeRelease(&pInterface);

        fRet = true;

        break;
    }
END:
    SafeArrayDestroy(psa);
    SafeRelease(&pRadio);
    SafeRelease(&pInterface);

    return fRet;
}

bool MobileBroadbandManager::SetAllPowerState(bool fOn)
{
    bool fRet = false;
    HRESULT hr = S_OK;
    IMbnInterface* pInterface = NULL;
    IMbnRadio* pRadio = NULL;
    SAFEARRAY *psa = NULL;
    LONG lLower = 0;
    LONG lUpper = 0;
    MBN_READY_STATE readyState;
    MBN_RADIO radioState;
    ULONG ulRequestId = 0;

    if (!m_pManager)
        goto END;

    hr = m_pManager->GetInterfaces(&psa);
    if (FAILED(hr))
        goto END;

    for (LONG l = lLower; l <= lUpper; l++)
    {   
        hr = SafeArrayGetElement(psa, &l, (void*)(&pInterface));
        if (FAILED(hr))
            goto END;

        hr = pInterface->GetReadyState(&readyState);
        if (FAILED(hr))
            goto END;

        hr = pInterface->QueryInterface(IID_IMbnRadio, (void**)&pRadio);
        if (FAILED(hr))
            goto END;

        radioState = (fOn) ? MBN_RADIO::MBN_RADIO_ON : MBN_RADIO::MBN_RADIO_OFF;
        hr = pRadio->SetSoftwareRadioState(radioState, &ulRequestId);
        if (FAILED(hr))
            goto END;

        SafeRelease(&pRadio);
        SafeRelease(&pInterface);
    }

    fRet = true;

END:
    SafeArrayDestroy(psa);
    SafeRelease(&pRadio);
    SafeRelease(&pInterface);

    return fRet;
}

bool MobileBroadbandManager::GetSignalStrength(int nDeviceId, ULONG& ulStrength)
{
    bool fRet = false;
    HRESULT hr = S_OK;
    IMbnInterface* pInterface = NULL;
    IMbnSignal* pSignal = NULL;
    SAFEARRAY *psa = NULL;
    LONG lLower = 0;
    LONG lUpper = 0;
    MBN_READY_STATE readyState;

    if (!m_pManager)
        goto END;

    hr = m_pManager->GetInterfaces(&psa);
    if (FAILED(hr))
        goto END;

    for (LONG l = lLower; l <= lUpper; l++)
    {
        if (l != (LONG)nDeviceId)
            continue;

        hr = SafeArrayGetElement(psa, &l, (void*)(&pInterface));
        if (FAILED(hr))
            goto END;

        hr = pInterface->GetReadyState(&readyState);
        if (FAILED(hr))
            goto END;

        hr = pInterface->QueryInterface(IID_IMbnSignal, (void**)&pSignal);
        if (FAILED(hr))
            goto END;

        hr = pSignal->GetSignalStrength(&ulStrength);
        if (FAILED(hr))
            goto END;

        SafeRelease(&pSignal);
        SafeRelease(&pInterface);

        fRet = true;

        break;
    }
END:
    SafeArrayDestroy(psa);
    SafeRelease(&pSignal);
    SafeRelease(&pInterface);

    return fRet;
}


Modem::Modem()
{
    m_hCom = NULL;
}

Modem::Modem(LPCWSTR lpszCom, DWORD dwBaudrate)
{
    m_hCom = NULL;

    Set(lpszCom, dwBaudrate);
}

Modem::~Modem()
{
    if (m_hCom && m_hCom != INVALID_HANDLE_VALUE)
        CloseHandle(m_hCom);
}

bool Modem::Set(LPCWSTR lpszCom, DWORD dwBaudrate)
{
    bool fRet = false;
    string strResult;
    DCB dcb = {0};

    if (m_hCom && m_hCom != INVALID_HANDLE_VALUE)
        CloseHandle(m_hCom);

    m_hCom = CreateFile(lpszCom, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (m_hCom == INVALID_HANDLE_VALUE)
        goto END;

    if (!GetCommState(m_hCom, &dcb))
        goto END;

    dcb.BaudRate = dwBaudrate;

    if (!SetCommState(m_hCom, &dcb))
        goto END;

    fRet = true;
END:
    return fRet;
}

bool Modem::IsValid()
{
    return (m_hCom && m_hCom != INVALID_HANDLE_VALUE) ? true : false; 
}

bool Modem::ParseATCommandResult(LPCSTR lpszATCommand, string& strResult, vector<string>& resultArray)
{
    bool fRet = false;

    resultArray.clear();

    if (strResult.find("\r\nOK\r\n") == string::npos)
        goto END;

    replace_all(strResult, "\r\nOK\r\n", "");
    replace_all(strResult, "\r\n", ";");
    replace_all(strResult, "\r", "");

    split(resultArray, strResult, is_any_of(";"));

    if (stricmp(resultArray[0].c_str(), lpszATCommand) != 0)
        goto END;

    fRet = true;
END:
    return fRet;
}

bool Modem::SendATCommand(LPCSTR lpszATCommand, string& strOutput, DWORD dwWaitTime)
{
    bool fRet = false;
    DWORD dwOut = 0;
    string strATCommand = lpszATCommand;
    char* pBuffer = NULL;
    OVERLAPPED ov = {0};
    DWORD dwErrors = 0;
    COMSTAT comStat = {0};
    DWORD dwEvtMask = 0;

    if (!IsValid())
        goto END;

    if (!strATCommand.size())
        goto END;

    strATCommand += "\r\n";

    if (!SetCommMask(m_hCom, EV_TXEMPTY))
        goto END;

    if (!WriteFile(m_hCom, strATCommand.data(), strATCommand.length(), &dwOut, NULL))
        goto END;

    fRet = WaitCommEvent(m_hCom, &dwEvtMask, NULL);  // Wait tx operation done.
    Sleep(dwWaitTime);  // Wait input buffer to be filled.

    if (!ClearCommError(m_hCom, &dwErrors, &comStat))
        goto END;

    if (!comStat.cbInQue)
        goto END;

    pBuffer = new char[comStat.cbInQue + 1];
    if (!pBuffer)
        goto END;

    memset(pBuffer, 0, comStat.cbInQue + 1);

    if (!ReadFile(m_hCom, pBuffer, comStat.cbInQue, &dwOut, NULL))
        goto END;

    strOutput = pBuffer;
    fRet = true;
END:
    if (pBuffer)
        delete[] pBuffer;

    return fRet;
}

bool Modem::SendATCommand(LPCSTR lpszATCommand, vector<string>& resultArray, DWORD dwWaitTime)
{
    bool fRet = false;
    string strOutput;

    if (!SendATCommand(lpszATCommand, strOutput, dwWaitTime))
        goto END;

    fRet = ParseATCommandResult(lpszATCommand, strOutput, resultArray);
END:
    return fRet;
}

bool Modem::GetInfo(MODEM_INFO& modemInfo, DWORD dwWaitTime)
{
    bool fRet = false;
    vector<string> lines;

    if (!IsValid())
        goto END;

    if (!SendATCommand("ati", lines, dwWaitTime))
        goto END;

    for (size_t i = 1; i < lines.size(); i++)
    {        
        if (lines[i].find("Manufacturer: ") != string::npos)
            Util::StringToWstring(CP_ACP, lines[i].substr(strlen("Manufacturer: ")), modemInfo.strManufacturer);
        else if (lines[i].find("Model: ") != string::npos)
            Util::StringToWstring(CP_ACP, lines[i].substr(strlen("Model: ")), modemInfo.strModel);
        else if (lines[i].find("Revision: ") != string::npos)
            Util::StringToWstring(CP_ACP, lines[i].substr(strlen("Revision: ")), modemInfo.strRevision);
        else if (lines[i].find("SVN: ") != string::npos)
            Util::StringToWstring(CP_ACP, lines[i].substr(strlen("SVN: ")), modemInfo.strSVN);
        else if (lines[i].find("IMEI: ") != string::npos)
            Util::StringToWstring(CP_ACP, lines[i].substr(strlen("IMEI: ")), modemInfo.strIMEI);
    }

    fRet = true;
END:
    return fRet;
}

bool Modem::GetIMSI(wstring& strIMSI, DWORD dwWaitTime)
{
    bool fRet = false;
    vector<string> lines;

    if (!IsValid())
        goto END;

    if (!SendATCommand("at+cimi", lines, dwWaitTime))
        goto END;

    fRet = Util::StringToWstring(CP_ACP, lines[1], strIMSI);
END:
    return fRet;
}