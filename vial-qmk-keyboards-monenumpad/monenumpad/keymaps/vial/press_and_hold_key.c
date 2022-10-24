
#include QMK_KEYBOARD_H
#include "press_and_hold_key.h"
#include "mone_keys.h"


#define HOLD_DETECTION_DURATION     500

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
#ifdef DEBUG_PRESS_AND_HOLD
        uprintf("KL: ASSERT WARNING press_and_hold_key_is_handled(c: %u, r: %u):  key->state should be key_pressed.\n",
        	key->pos.row, key->pos.col);
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

// #ifdef DEBUG_PRESS_AND_HOLD
//     uprintf("KL: %s(c: %u, r: %u) - key->pos matched\n", __FUNCTION__,
//    		key->pos.row, key->pos.col);
// #endif    

    // if key is pressed
    if (record->event.pressed) 
    {
        if (key->state == key_not_pressed) 
        {
#ifdef DEBUG_PRESS_AND_HOLD
		    uprintf("KL: %s(c: %u, r: %u) - key->state == key_not_pressed. Changed state to pressed.\n", __FUNCTION__,
        			key->pos.row, key->pos.col);
#endif    
            key->state = key_pressed;
            key->timer = timer_read();
        }
    }
    else // !record->event.pressed
    {
	    bool result = false;

        // means the key is pressed for less than HOLD_DETECTION_DURATION
        if (key->state == key_pressed)
        {
            if (timer_elapsed(key->timer) < HOLD_DETECTION_DURATION)
            {
#ifdef DEBUG_PRESS_AND_HOLD
    			uprintf("KL: %s(c: %u, r: %u) - timer_elapsed(key->timer) < HOLD_DETECTION_DURATION. invoke procees_tap().\n", __FUNCTION__,
    				key->pos.row, key->pos.col);
#endif    

                result = key->process_tap(keycode);
            }
            else //(timer_elapsed(key.timer) >= HOLD_DETECTION_DURATION)
            {
#ifdef DEBUG_PRESS_AND_HOLD
    			uprintf("KL: %s(c: %u, r: %u) - timer_elapsed(key->timer) >= HOLD_DETECTION_DURATION. invoke procees_hold().\n", __FUNCTION__,
    				key->pos.row, key->pos.col);
#endif    
                result = key->process_hold(keycode);
            }                
        }

        if (result) 
        {
#ifdef DEBUG_PRESS_AND_HOLD
   			uprintf("KL: %s(c: %u, r: %u) - result == false, process_mone_key() and tap_code16().\n", __FUNCTION__,
   				key->pos.row, key->pos.col);
#endif    
		    if (process_mone_key(keycode)) 
		    {
		        tap_code16(keycode);
		    }                	
        }
	
#ifdef DEBUG_PRESS_AND_HOLD
		uprintf("KL: %s(c: %u, r: %u) - restore key->state = key_not_pressed.\n", __FUNCTION__,
			key->pos.row, key->pos.col);
#endif    
        key->state = key_not_pressed;

    }

    return false;
}

