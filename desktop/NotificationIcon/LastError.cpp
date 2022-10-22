#include "LastError.h"
#include "AutoPtr.h"

bool GetWinLastErrorrMessage(
    __out HRESULT* pHR,
    __out CHeapPtr<TCHAR, CLocalAllocator>& o_msgBufPtr)
{
    // Retrieve the system error message for the last-error code

    LPTSTR lpMsgBuf = NULL;
    DWORD dw = GetLastError();

    DWORD ret = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    o_msgBufPtr.Attach(lpMsgBuf);

    return (ret != 0);
}


#pragma warning( push )
#pragma warning( disable : 6255 )
void ThrowHR(LPCSTR pszFileName, int line, LPCSTR lpszFunction, HRESULT hr, LPCTSTR pszMsg) throw (HRESULT)
{
    USES_CONVERSION;

    CHeapPtr<TCHAR, CLocalAllocator> lpDisplayBuf;
    const SIZE_T cchDisplayBuf = lstrlen((LPCTSTR)pszMsg) + strlen(lpszFunction) + strlen(pszFileName) + 40;
    lpDisplayBuf.Allocate(cchDisplayBuf);

    if (lpDisplayBuf != NULL)
    {
        StringCchPrintf(
            lpDisplayBuf,
            cchDisplayBuf,
            TEXT("%s (%d) %s: HRESULT=0x%08x Message=%s\n"),
            A2T(pszFileName), line, A2T(lpszFunction), hr, pszMsg);

        ATLTRACE(lpDisplayBuf);
    }

    throw hr;

}
#pragma warning( pop )

void ThrowWinLastError(LPCSTR pszFileName, int line, LPCSTR lpszFunction) throw (HRESULT)
{
    // attach point and free on exit function
    HRESULT hr = S_OK;
    CHeapPtr<TCHAR, CLocalAllocator> msgBufPtr;
    GetWinLastErrorrMessage(&hr, msgBufPtr);
    
    ThrowHR(pszFileName, line, lpszFunction, hr, msgBufPtr);
}

