#include "framework.h"
#include "GetProcessName.h"
#include <ShlObj.h>    // Shell API
#include <Propkey.h>   // PKEY_* constants

std::wstring GetProgrameNameFromPath(const std::wstring& processPath)  throw (HRESULT)
{
    PROPERTYKEY key = PKEY_Software_ProductName;

    // Use CComPtr to automatically release the IShellItem2 interface when the function returns
    // or an exception is thrown.
    CComPtr<IShellItem2> pItem;
    CHK_HR(SHCreateItemFromParsingName(processPath.c_str(), nullptr, IID_PPV_ARGS(&pItem)));

    // Use CComHeapPtr to automatically release the string allocated by the shell when the function returns
    // or an exception is thrown (calls CoTaskMemFree).
    CComHeapPtr<WCHAR> pValue;
    CHK_HR(pItem->GetString(key, &pValue));

    // Copy to wstring for convenience
    return std::wstring(pValue);
}

std::wstring GetProcessNameFromID(DWORD processId) throw (HRESULT)
{
    if (processId == 0)
    {
        return std::wstring();
    }

    std::wstring processPath;
    CHandle handle;
    handle.Attach(OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        processId /* This is the PID, you can find one from windows task manager */
    ));
    if (handle == NULL)
    {
        THROW_LAST_ERROR();
    }

    DWORD buffSize = 1024;
    TCHAR buffer[1024] = TEXT("");
    if (!QueryFullProcessImageName(handle, 0, buffer, &buffSize))
    {
        THROW_LAST_ERROR();
    }

    processPath = buffer;

    return GetProgrameNameFromPath(processPath);
}


