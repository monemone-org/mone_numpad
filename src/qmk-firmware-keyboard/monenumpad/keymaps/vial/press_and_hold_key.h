#pragma once


//
// Layer management functions
//
enum press_and_hold_key_state_t {
    key_not_pressed = 0,
    key_pressed = 1,
    key_pressed_and_handled = 2
};


typedef struct {
    keypos_t pos;
    uint16_t state; //press_and_hold_key_state_t
    uint16_t timer;

    // return true if qmk should continue processing the pressed key 
    bool (*process_hold)(uint16_t keycode);
    // return true if qmk should continue processing the pressed key 
    bool (*process_tap)(uint16_t keycode);

} press_and_hold_key_t;


press_and_hold_key_t make_press_and_hold_key(
    	keypos_t i_pos,
    	bool (*i_process_hold)(uint16_t keycode),
    	bool (*i_process_tap)(uint16_t keycode));

// return true if qmk should continue processing the pressed key 
bool process_press_and_hold_key(
	press_and_hold_key_t *key, uint16_t keycode, keyrecord_t *record);

bool press_and_hold_key_is_pressed(press_and_hold_key_t *key);

void set_press_and_hold_key_to_handled(press_and_hold_key_t *key);




