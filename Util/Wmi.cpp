#include <windows.h>
#include <tchar.h>
#include <Propvarutil.h>  // Variant functions

#include "Util.h"

// Enable memory leak check for malloc and new
// Add this after all #include.
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "libjson.lib")

namespace Util {

    namespace WMI {

        bool Initialize()
        {
            bool fRet = false;
            HRESULT hr = S_OK;

            hr = CoInitializeSecurity(
                NULL, 
                -1,                          // COM negotiates service
                NULL,                        // Authentication services
                NULL,                        // Reserved
                RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
                RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
                NULL,                        // Authentication info
                EOAC_NONE,                   // Additional capabilities 
                NULL                         // Reserved
                );

            if (hr == RPC_E_TOO_LATE)  // CoInitializeSecurity() has been already called.
            {
                hr = S_OK;
                goto END;
            }else if (FAILED(hr))
                goto END;

            fRet = true;

        END:
            return fRet;
        }

        void UnInitialize()
        {
        }

        bool GetNamespace(LPCWSTR lpszNamespace, IWbemServices** ppNamespace)
        {
            bool fRet = false;
            HRESULT hr = S_OK;
            IWbemLocator* pLocator = NULL;

            hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);
            if (FAILED(hr))
                goto END;

            hr = pLocator->ConnectServer(_bstr_t(lpszNamespace), NULL, NULL, NULL, 0, NULL, NULL, ppNamespace);
            if (FAILED(hr))
                goto END;

            // Set security levels for the proxy automatically
            hr = CoSetProxyBlanket(
                *ppNamespace,                        // Indicates the proxy to set
                RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
                RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
                NULL,                        // Server principal name 
                RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
                RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                NULL,                        // client identity
                EOAC_NONE                    // proxy capabilities 
                );

            if (FAILED(hr))
                goto END;

            fRet = true;
        END:
            if (pLocator)
                pLocator->Release();

            return fRet;
        }

        bool GetClass(IWbemServices* pNamespace, LPCWSTR lpszClassName, IWbemClassObject** ppClass)
        {
            bool fRet = false;
            HRESULT hr = pNamespace->GetObject(_bstr_t(lpszClassName), 0, NULL, ppClass, NULL);
            if (FAILED(hr))
                goto END;

            fRet = true;
        END:
            return fRet;
        }

        bool GetObjects(IWbemServices* pNamespace, LPCWSTR lpszClassName, vector<IWbemClassObject*>& objs)
        {
            bool fRet = false;
            HRESULT hr = S_OK;
            IEnumWbemClassObject* pEnum = NULL;

            objs.clear();

            _bstr_t bstrQuery("SELECT * from ");
            bstrQuery += _bstr_t(lpszClassName);

            hr = pNamespace->ExecQuery(_bstr_t(L"WQL"), //Query Language
                bstrQuery, //Query to Execute
                WBEM_FLAG_RETURN_IMMEDIATELY, //Make a semi-synchronous call
                NULL, //Context
                &pEnum //Enumeration Interface
                );

            if (FAILED(hr))
                goto END;

            while(hr == WBEM_S_NO_ERROR) {
                IWbemClassObject* pObj = NULL;
                ULONG ulRet = 0;

                hr = pEnum->Next(WBEM_INFINITE, //Timeout
                    1, //No of objects requested
                    &pObj, //Returned Object 
                    &ulRet //No of object returned
                    );

                if (hr == WBEM_S_NO_ERROR)
                    objs.push_back(pObj);  // Caller should call pObj->Release() for each elements in the vector after all done.
            }

            if (!objs.size())
                goto END;

            fRet = true;
        END:
            if (pEnum)
                pEnum->Release();

            return fRet;
        }

        bool GetObjectProperty(IWbemClassObject* pObj, LPCWSTR lpszPropName, _variant_t& var)
        {
            bool fRet = false;
            HRESULT hr = S_OK;
            VARIANT v;

            VariantInit(&v);
            hr = pObj->Get(lpszPropName, 0, &v, NULL, NULL);
            if (FAILED(hr))
                goto END;

            var = v;

            fRet = true;
        END:
            VariantClear(&v);
            return fRet;
        }

        bool GetObjectPath(IWbemClassObject* pObj, wstring& strPath)
        {
            bool fRet = false;
            _variant_t var;

            if (!GetObjectProperty(pObj, L"__PATH", var))
                goto END;

            strPath = (_bstr_t)var;
            fRet = true;
        END:
            return fRet;
        }

        bool SetInParams(Json::Value* pInParams, IWbemClassObject* pInInstance)
        {
            bool fRet = false;
            HRESULT hr = S_OK;

            if ((!pInParams) || (!pInInstance))
                goto END;

            if (!pInParams->isArray())
                goto END;

            for (size_t i = 0; i < pInParams->size(); i++) {
                Json::Value name;
                Json::Value value;
                Json::Value type;
                VARIANT var;
                wstring strName;
                wstring strValue;

                name = (*pInParams)[(Json::ArrayIndex)i].get("name", NULL);
                value = (*pInParams)[(Json::ArrayIndex)i].get("value", NULL);
                type = (*pInParams)[(Json::ArrayIndex)i].get("type", NULL);

                StringToWstring(CP_ACP, name.asString(), strName);
                StringToWstring(CP_ACP, value.asString(), strValue);

                VariantInit(&var);
                var.vt = VT_BSTR;
                var.bstrVal = SysAllocString(strValue.c_str());

                hr = pInInstance->Put(strName.c_str(), 0, &var, type.asUInt());

                VariantClear(&var);

                if (FAILED(hr))
                    goto END;
            }

            fRet = true;
        END:
            return fRet;
        }

        bool ExecMethod(IWbemServices* pNamespace, IWbemClassObject* pClass, BSTR bstrPath, LPCWSTR lpszMethodName, Json::Value* pInParams, IWbemClassObject** ppOutParamsObj)
        {
            bool fRet = false;
            HRESULT hr = S_OK;
            IWbemClassObject* pInParamsObj = NULL;
            IWbemClassObject* pInInstance = NULL;
            IWbemCallResult* pResult = NULL;

            hr = pClass->GetMethod(lpszMethodName, 0, &pInParamsObj, NULL);
            if (FAILED(hr))
                goto END;

            hr = pInParamsObj->SpawnInstance(0, &pInInstance);
            if (FAILED(hr))
                goto END;

            if (pInParams) {
                hr = SetInParams(pInParams, pInInstance);
                if (FAILED(hr))
                    goto END;
            }

            hr = pNamespace->ExecMethod(bstrPath, _bstr_t(lpszMethodName), 0, NULL, pInInstance, ppOutParamsObj, NULL);
            if (FAILED(hr))
                goto END;

            fRet = true;
        END:
            if (pInParamsObj)
                pInParamsObj->Release();

            if (pInInstance)
                pInInstance->Release();

            return fRet;
        }

        // WMI::Obj
        Obj::Obj()
        {
            m_pObj = NULL;
        }

        Obj::Obj(IWbemClassObject* pObj)
        {
            m_pObj = NULL;
            Set(pObj);
        }

        Obj::Obj(const Obj& obj)
        {
            m_pObj = obj.m_pObj;
            obj.m_pObj->AddRef();
        }

        Obj::~Obj()
        {
            if (m_pObj)
                m_pObj->Release();
        }

        void Obj::Set(IWbemClassObject* pObj)
        {
            m_pObj = pObj;
        }

        bool Obj::isValid()
        {
            return m_pObj ? true : false;
        }

        bool Obj::GetProperty(LPCWSTR lpszName, _variant_t& var)
        {
            bool fRet = false;
            HRESULT hr = S_OK;

            if (!m_pObj)
                goto END;

            if (!lpszName)
                goto END;

            fRet = GetObjectProperty(m_pObj, lpszName, var);

        END:
            return fRet;
        }

        bool Obj::GetPropNames(vector<wstring>& propNames)
        {
            bool fRet = false;
            HRESULT hr = S_OK;
            SAFEARRAY* pArray = NULL;

            if (!m_pObj)
                goto END;

            hr = m_pObj->GetNames(NULL, 0, NULL, &pArray);
            if (FAILED(hr))
                goto END;

            if (!SafeArrayToWstringVector(pArray, propNames))
                goto END;

            fRet = true;
        END:
            return fRet;        
        }

        bool Obj::GetProps(map<wstring, _variant_t>& props, bool fIgnoreWMISystemProps)
        {
            bool fRet = false;
            vector<wstring> names;
            const WCHAR* lpszWMISystemPropNames[] = {
                L"__Class",
                L"__Derivation",
                L"__Dynasty",
                L"__Genus",
                L"__Namespace",
                L"__Path",
                L"__Property_Count",
                L"__Relpath",
                L"__Server",
                L"__Superclass"
            };
            vector<wstring> wmiSystemPropNames;

            if (!GetPropNames(names))
                goto END;

            props.clear();
            for (size_t i = 0; i < names.size(); i++)
            {
                _variant_t var;
                if (fIgnoreWMISystemProps)
                {
                    bool fFound = false;
                    for (size_t j = 0; j < sizeof(lpszWMISystemPropNames) / sizeof(lpszWMISystemPropNames[0]); j++)
                    {
                        if (wcsicmp(names[i].c_str(), lpszWMISystemPropNames[j]) == 0)
                        {
                            fFound = true;
                            break;
                        }
                    }

                    if (fFound)
                        continue;
                }

                if (!GetProperty(names[i].c_str(), var))
                    goto END;

                props[names[i]] = var; 
            }

            fRet = true;
        END:
            return fRet;
        }

        bool Obj::GetProps(Json::Value& root, bool fIgnoreWMISystemProps)
        {
            bool fRet = false;
            map<wstring, _variant_t> props;
            map<wstring, _variant_t>::iterator it;

            if (!GetProps(props, fIgnoreWMISystemProps))
                goto END;

            for (it = props.begin(); it != props.end(); it++)
            {
                string strUTF8Name;
                Json::Value value = Json::nullValue;

                if (!VariantToJsonValue(it->second, value))
                    goto END;

                WstringToUTF8String(it->first, strUTF8Name);
                root[strUTF8Name] = value;
            }

            fRet = true;
        END:
            return fRet;
        }

        bool Obj::GetProps(wstring& strProps, bool fIgnoreWMISystemProps)
        {
            bool fRet = false;
            Json::Value root;
            Json::FastWriter writer;
            string str;

            strProps.clear();

            if (!GetProps(root, fIgnoreWMISystemProps))
                goto END;

            str = writer.write(root);
            fRet = Util::StringToWstring(CP_UTF8, str, strProps);
        END:
            return fRet;
        }

        bool Obj::GetPath(wstring& strPath)
        {
            return GetObjectPath(m_pObj, strPath);
        }

        // WMUtil::Namespace
        Namespace::Namespace()
        {
            m_pNamespace = NULL;
            WMI::Initialize();
        }

        Namespace::Namespace(LPCWSTR lpszName)
        {
            m_pNamespace = NULL;

            WMI::Initialize();
            Set(lpszName);
        }

        Namespace::Namespace(const Namespace& ns)
        {
            m_pNamespace = ns.m_pNamespace;
            ns.m_pNamespace->AddRef();
        }

        Namespace::~Namespace()
        {
            if (m_pNamespace)
                m_pNamespace->Release();

            WMI::UnInitialize();
        }

        void Namespace::Set(LPCWSTR lpszName)
        {
            GetNamespace(lpszName, &m_pNamespace);
        }

        bool Namespace::IsValid()
        {
            return m_pNamespace ? true : false;
        }

        bool Namespace::GetClass(LPCWSTR lpszClassName, IWbemClassObject** ppClass)
        {
            if (!IsValid())
                return false;
            else
                return WMI::GetClass(m_pNamespace, lpszClassName, ppClass);
        }

        bool Namespace::GetObjs(LPCWSTR lpszClassName, vector<Obj>& objs)
        {
            bool fRet = false;
            vector<IWbemClassObject*> objects;

            objs.clear();

            if (!IsValid())
                goto END;

            if (!WMI::GetObjects(m_pNamespace, lpszClassName, objects))
                goto END;

            for (size_t i = 0; i < objects.size(); i++)
            {
                Obj obj(objects[i]);
                objs.push_back(obj);
            }

            fRet = true;

        END:
            return fRet;
        }

        bool Namespace::ExecMethod(Obj& obj, LPCWSTR lpszClassName, LPCWSTR lpszMethodName, Json::Value* pInParams, IWbemClassObject** ppOutParamsObj)
        {
            bool fRet = false;
            wstring strPath;
            IWbemClassObject* pClass = NULL;

            if (!obj.GetPath(strPath))
                goto END;

            if (!GetClass(lpszClassName, &pClass))
                goto END;

            if (!WMI::ExecMethod(m_pNamespace, pClass, _bstr_t(strPath.c_str()), lpszMethodName, pInParams, ppOutParamsObj))
                goto END;

            fRet = true;
        END:
            return fRet;
        }

        bool Namespace::ExecMethod(LPCWSTR lpszClassName, LPCWSTR lpszMethodName, Json::Value* pInParams, IWbemClassObject** ppOutParamsObj)
        {
            bool fRet = false;
            IWbemClassObject* pClass = NULL;

            if (!GetClass(lpszClassName, &pClass))
                goto END;

            if (!WMI::ExecMethod(m_pNamespace, pClass, _bstr_t(lpszClassName), lpszMethodName, pInParams, ppOutParamsObj))
                goto END;

            fRet = true;
        END:
            return fRet;
        }
    }  // namespace WMI
}  // namespace Util