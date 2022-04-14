//assert_precond.c

#include QMK_KEYBOARD_H

#include "assert_precond.h"

void assert_precond(bool cond, char* message)
{
#ifdef CONSOLE_ENABLE
	if (!cond)  
	{
	    uprintf("ERR assert_precond: %s\n", message);
	}
#endif 
}



