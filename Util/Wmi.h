#pragma once

#include <objbase.h>                                
#include <stdio.h>
#include <wbemidl.h> 
#include <comdef.h>
#include <WbemCli.h>  // CIMType

#include "../jsoncpp/include/json/json.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

#pragma comment(lib, "Wbemuuid.lib")  // WMI class and functions
#pragma comment(lib, "Propsys.lib")  // VariantToXX functions

namespace Util {

    // Need call CoInitialize() / CoUninitialize() for the thread.
    namespace WMI {

        bool Initialize();
        void UnInitialize();
        bool GetNamespace(LPCWSTR lpszNamespace, IWbemServices** ppNamespace);
        bool GetClass(IWbemServices* pNamespace, LPCWSTR lpszClassName, IWbemClassObject** ppClass);
        bool GetObjects(IWbemServices* pNamespace, LPCWSTR lpszClassName, vector<IWbemClassObject*>& objs);

        bool GetObjectProperty(IWbemClassObject* pObj, LPCWSTR lpszPropName, _variant_t& var);

        bool GetObjectPath(IWbemClassObject* pObj, wstring& strPath);

        // pInParams: The param is expressed in JSON format:
        // name: param name, value: param value in string format, type: CIMTYPE defined in "WbemCli.h".
        // Ex:
        // sprintf(buffer, "[ {\"name\": \"Timeout\", \"value\": \"100\", \"type\": %d}, {\"name\": \"Brightness\", \"value\": \"%d\", \"type\": %d ]", CIM_UINT32, nPercent, CIM_UIN8);
        bool SetInParams(Json::Value* pInParams, IWbemClassObject* pInInstance);

        // bstrPath can be one of the following 2 values:
        // 1. "__PATH" property of an object in pEnum after IWbemServices->ExecQuery() excuted. Ex: L"\\\\XXU-WIN8-8250\\root\\wmi:WmiMonitorBrightnessMethods.InstanceName=\"DISPLAY\\\\LGD0259\\\\5&779a65e&0&UID1048848_0\""
        // 2. Class name. Ex: "Win32_Process".
        //
        // pInParams: The param is expressed in JSON format:
        // name: param name, value: param value in string format, type: CIMTYPE defined in "WbemCli.h".
        // Ex:
        // sprintf(buffer, "[ {\"name\": \"Timeout\", \"value\": \"100\", \"type\": %d}, {\"name\": \"Brightness\", \"value\": \"%d\", \"type\": %d ]", CIM_UINT32, nPercent, CIM_UIN8);
        bool ExecMethod(IWbemServices* pNamespace, IWbemClassObject* pClass, BSTR bstrPath, LPCWSTR lpszMethodName, Json::Value* pInParams, IWbemClassObject** ppOutParamsObj);

        class Obj
        {
        public:
            Obj();
            Obj(IWbemClassObject* pObj);
            Obj(const Obj& obj);  // Copy Constructor. Used for std container's push_back().
            ~Obj();

            void Set(IWbemClassObject* pObj);
            bool isValid();

            bool GetProperty(LPCWSTR lpszName, _variant_t& var);
            bool GetPropNames(vector<wstring>& propNames);
            bool GetProps(map<wstring, _variant_t>& props, bool fIgnoreWMISystemProps = true);
            bool GetProps(Json::Value& root, bool fIgnoreWMISystemProps = true);
            bool GetProps(wstring& strProps, bool fIgnoreWMISystemProps = true);
            bool GetPath(wstring& strPath);

            IWbemClassObject* m_pObj;
        };

        class Namespace
        {
        public:
            Namespace();
            Namespace(LPCWSTR lpszName);
            Namespace(const Namespace& ns);  // Copy Constructor. Used for std container's push_back().
            ~Namespace();

            void Set(LPCWSTR lpszName);
            bool IsValid();

            bool GetClass(LPCWSTR lpszClassName, IWbemClassObject** ppClass);
            bool GetObjs(LPCWSTR lpszClassName, vector<Obj>& objs);
            bool ExecMethod(Obj& obj, LPCWSTR lpszClassName, LPCWSTR lpszMethodName, Json::Value* pInParams, IWbemClassObject** ppOutParamsObj);
            bool ExecMethod(LPCWSTR lpszClassName, LPCWSTR lpszMethodName, Json::Value* pInParams, IWbemClassObject** ppOutParamsObj);

            IWbemServices* m_pNamespace;
        };

    }  // namespace WMI
}  // namespace Util