#pragma once
#include "framework.h"

#pragma warning( disable : 4290 )
#pragma warning( disable : 5040 )

#define str(s) TEXT(#s)

#define CHK_HR(hr) \
    {\
        HRESULT _hr = (hr); \
        if (FAILED(_hr)) \
        { \
            ThrowHR(__FILE__, __LINE__, __FUNCTION__, _hr, str(hr));\
        } \
    }

#define CHK_HR2(hr, msg) \
    {\
        HRESULT _hr = (hr); \
        if (FAILED(_hr)) \
        { \
            ThrowHR(__FILE__, __LINE__, __FUNCTION__, _hr, msg);\
        } \
    }

#define THROW_LAST_ERROR() \
    {\
        ThrowWinLastError(__FILE__, __LINE__, __FUNCTION__);\
    }

#define THROW_E_FAIL(msg) \
    {\
        ThrowHR(__FILE__, __LINE__, __FUNCTION__, E_FAIL, msg);\
    }

#define THROW_HR(hr, msg) \
    {\
        ThrowHR(__FILE__, __LINE__, __FUNCTION__, (hr), msg);\
    }

void ThrowHR(LPCSTR pszFileName, int line, LPCSTR lpszFunction, HRESULT err, LPCTSTR pszMsg) throw (HRESULT);
void ThrowWinLastError(LPCSTR pszFileName, int line, LPCSTR lpszFunction) throw (HRESULT);

bool GetWinLastErrorrMessage(
    __out HRESULT* pHR,
    __out CHeapPtr<TCHAR, CLocalAllocator>& p_msgBufPtr);

