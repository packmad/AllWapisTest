#include "Com.h"
#include "Utils.h"

#include <taskschd.h>
#include <comutil.h>

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsuppw.lib")

// Template for safe COM release
template<typename T>
void SafeRelease(T** ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

// Implementation of COM tests
void Com::TestITaskService() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        PrintTestResult(L"ITaskService", false, L"CoInitializeEx failed: 0x" + std::to_wstring(hr));
        return;
    }

    ITaskService* pService = NULL;
    hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr)) {
        PrintTestResult(L"ITaskService", false, L"CoCreateInstance failed: 0x" + std::to_wstring(hr));
        CoUninitialize();
        return;
    }

    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        PrintTestResult(L"ITaskService", false, L"Connect failed: 0x" + std::to_wstring(hr));
        SafeRelease(&pService);
        CoUninitialize();
        return;
    }

    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        PrintTestResult(L"ITaskService", false, L"GetFolder failed: 0x" + std::to_wstring(hr));
        SafeRelease(&pService);
        CoUninitialize();
        return;
    }

    ITaskDefinition* pTask = NULL;
    hr = pService->NewTask(0, &pTask);
    bool success = SUCCEEDED(hr);
    if (success) {
        // Simplified: Set basic info, trigger (now +5min), action (cmd.exe), register as SYSTEM
        IRegistrationInfo* pInfo = NULL;
        pTask->get_RegistrationInfo(&pInfo);
        if (pInfo) pInfo->put_Author(_bstr_t(L"Test")); SafeRelease(&pInfo);

        ITriggerCollection* pTriggers = NULL; pTask->get_Triggers(&pTriggers);
        ITrigger* pTrigger = NULL; pTriggers->Create(TASK_TRIGGER_TIME, &pTrigger);
        ITimeTrigger* pTimeTrigger = NULL; pTrigger->QueryInterface(IID_ITimeTrigger, (void**)&pTimeTrigger);
        // Mock time: Use current +5min
        SYSTEMTIME st; GetLocalTime(&st); st.wMinute += 5;
        wchar_t buf[32]; swprintf_s(buf, L"%04d-%02d-%02dT%02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        pTimeTrigger->put_StartBoundary(_bstr_t(buf));
        SafeRelease(&pTimeTrigger); SafeRelease(&pTrigger); SafeRelease(&pTriggers);

        IActionCollection* pActions = NULL; pTask->get_Actions(&pActions);
        IAction* pAction = NULL; pActions->Create(TASK_ACTION_EXEC, &pAction);
        IExecAction* pExec = NULL; pAction->QueryInterface(IID_IExecAction, (void**)&pExec);
        pExec->put_Path(_bstr_t(L"C:\\Windows\\System32\\cmd.exe"));
        SafeRelease(&pExec); SafeRelease(&pAction); SafeRelease(&pActions);

        IRegisteredTask* pRegTask = NULL;
        hr = pRootFolder->RegisterTaskDefinition(_bstr_t(L"TestTask"), pTask, TASK_CREATE_OR_UPDATE,
            _variant_t(L"S-1-5-18"), _variant_t(), TASK_LOGON_SERVICE_ACCOUNT, _variant_t(L""), &pRegTask);
        success = SUCCEEDED(hr);
        if (pRegTask) SafeRelease(&pRegTask);
    }
    SafeRelease(&pTask); SafeRelease(&pRootFolder); SafeRelease(&pService); CoUninitialize();
    PrintTestResult(L"ITaskService", success, success ? L"Task created as SYSTEM" : L"Register failed: 0x" + std::to_wstring(hr));
}