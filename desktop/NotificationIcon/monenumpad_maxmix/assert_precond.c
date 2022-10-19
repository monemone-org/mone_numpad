//assert_precond.c

#ifdef VIA_ENABLE
#include QMK_KEYBOARD_H
#endif

#ifdef _MSC_VER
#include <stdbool.h>
#include <windows.h>
#endif

#include "assert_precond.h"

void assert_precond(bool cond, char* message)
{
#ifdef CONSOLE_ENABLE
	if (!cond)  
	{
	    uprintf("ERR assert_precond: %s\n", message);
	}
#else
	#ifdef _MSC_VER
	if (!cond)
	{
#ifdef _DEBUG
		(void)OutputDebugStringA(message);
#endif
	}
	#endif
#endif 
}





