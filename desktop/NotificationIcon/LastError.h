#pragma once
#include "framework.h"

#pragma warning( disable : 4290 )

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


void ThrowHR(LPSTR pszFileName, int line, LPSTR lpszFunction, HRESULT err, LPCTSTR pszMsg) throw (HRESULT);
void ThrowWinLastError(LPSTR pszFileName, int line, LPSTR lpszFunction) throw (HRESULT);


