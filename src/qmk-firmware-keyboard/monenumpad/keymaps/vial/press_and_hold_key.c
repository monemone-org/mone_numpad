
#include QMK_KEYBOARD_H
#include "press_and_hold_key.h"
#include "mone_keys.h"

press_and_hold_key_t make_press_and_hold_key(
    	keypos_t i_pos,
    	bool (*i_process_hold)(uint16_t keycode),
    	bool (*i_process_tap)(uint16_t keycode))
{
    press_and_hold_key_t key = {
    	.pos = i_pos,
    	.state = key_not_pressed,
    	.timer = 0,
    	.process_hold = i_process_hold,
    	.process_tap = i_process_tap
    };

    return key;
}

bool press_and_hold_key_is_pressed(press_and_hold_key_t *key)
{
	return key->state == key_pressed || key->state == key_pressed_and_handled;
}

void set_press_and_hold_key_to_handled(press_and_hold_key_t *key) 
{
	if (key->state == key_pressed)
	{
		key->state = key_pressed_and_handled;
	}
	else 
	{
#ifdef CONSOLE_ENABLE
        uprintf("KL: ASSERT WARNING press_and_hold_key_is_handled():  key->state should be key_pressed.\n");
#endif 

	}
}

// return true if qmk should continue processing the pressed key 
bool process_press_and_hold_key(
	press_and_hold_key_t *key, 
	uint16_t keycode, 
	keyrecord_t *record)
{
    if (! KEYEQ(record->event.key, key->pos)) 
    {
        return true;
    }

// #ifdef CONSOLE_ENABLE
//     uprintf("KL: process_press_and_hold_key - key->pos matched\n");
// #endif    

    // if key is pressed
    if (record->event.pressed) 
    {
        if (key->state == key_not_pressed) 
        {
// #ifdef CONSOLE_ENABLE
//     uprintf("KL: process_press_and_hold_key - key->state == key_not_pressed. Changed state to pressed\n");
// #endif    
            key->state = key_pressed;
            key->timer = timer_read();
        }
    }
    else // !record->event.pressed
    {
	    bool result = false;

        // means the key is pressed for less than TAPPING_TERM
        if (key->state == key_pressed)
        {
            if (timer_elapsed(key->timer) < TAPPING_TERM)
            {
// #ifdef CONSOLE_ENABLE
//     			uprintf("KL: process_press_and_hold_key - timer_elapsed(key->timer) < TAPPING_TERM. invoke procees_tap\n");
// #endif    

                result = key->process_tap(keycode);
            }
            else //(timer_elapsed(key.timer) >= TAPPING_TERM)
            {
// #ifdef CONSOLE_ENABLE
//     			uprintf("KL: process_press_and_hold_key - timer_elapsed(key->timer) >= TAPPING_TERM. invoke procees_hold\n");
// #endif    
                result = key->process_hold(keycode);
            }                
        }

        if (result) 
        {
// #ifdef CONSOLE_ENABLE
//    			uprintf("KL: process_press_and_hold_key - result == false, process_mone_key\n");
// #endif    
		    if (process_mone_key(keycode)) 
		    {
		        tap_code16(keycode);
		    }                	
        }
	
// #ifdef CONSOLE_ENABLE
// 		uprintf("KL: process_press_and_hold_key - restore key->state = key_not_pressed\n");
// #endif    
        key->state = key_not_pressed;

    }

    return false;
}

