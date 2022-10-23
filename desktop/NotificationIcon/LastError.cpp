#include "LastError.h"
#include "AutoPtr.h"

#pragma warning( push )
#pragma warning( disable : 6255 )
void ThrowHR(LPSTR pszFileName, int line, LPSTR lpszFunction, HRESULT hr, LPCTSTR pszMsg) throw (HRESULT)
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

void ThrowWinLastError(LPSTR pszFileName, int line, LPSTR lpszFunction) throw (HRESULT)
{
    // Retrieve the system error message for the last-error code
    LPTSTR lpMsgBuf = NULL;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // attach point and free on exit function
    CHeapPtr<TCHAR, CLocalAllocator> msgBufPtr;
    msgBufPtr.Attach(lpMsgBuf);

    HRESULT hr = HRESULT_FROM_WIN32(dw);
    ThrowHR(pszFileName, line, lpszFunction, hr, lpMsgBuf);
}

