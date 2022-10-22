#include "framework.h"
#include "Log.h"

void log(TCHAR* format, ...)
{
    va_list arg_list;
    va_start(arg_list, format);

#pragma warning(push)
#pragma warning(disable: 6386)
    const size_t cbOut = 256;
    TCHAR pszOut[cbOut] = { 0 };
    _vsnwprintf_s(pszOut, cbOut - 1, format, arg_list);
    OutputDebugString(pszOut);
#pragma warning(pop)

    va_end(arg_list);
}


