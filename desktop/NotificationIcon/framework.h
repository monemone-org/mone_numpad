#pragma once
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
//#define _ATL_NO_COM_SUPPORT //Turn off support for COM object implementation

#define _ATL_APARTMENT_THREADED

//#define _ATL_NO_AUTOMATIC_NAMESPACE

#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>

#include <strsafe.h>
#include <system_error>
#include <string>
#include <memory>

#include "LastError.h"

