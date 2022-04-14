// Copyright 2022 monehsieh (@monehsieh)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <stdio.h>

#include "user_config.h"
#include "press_and_hold_key.h"
#include "mone_keys.h"
#include "maxmix/maxmix.h"

#ifdef CONSOLE_ENABLE
//#define DEBUG_EDIT_SESSION
//#define DEBUG_MAXMIX
//#define DEBUG_LAYER
#endif

/**
 * Behaviour descriptions:
 * 
 * 1. Pressing and holding key (r=0, c=4) will change layers
 * 
 * 2. Turning rotary encoder while holding key (r=0, c=4) also changes layers.
 *    Turning clockwise advances to the next layer. Turning anti-clockwise 
 *    will move to previous layer.
 * 
 * 3. Pressing rotary encoder while holding key (r=0, c=4) will toggle the 
 *    platform mode between Win and Mac, which affects the pre-programmed key
 *    shortcuts.
 * 
 */


/**
 * Windows Shortcuts:
 * Fn + DEL to increase the brightness and Fn + Backspace to decrease it.
 * On Dell XPS laptop keyboard (pictured below), hold the Fn key and press F11 or F12 to adjust the brightness of the screen
 */


// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE,
    _FN1,
    _FN2,
    _FN3,
    _LAST = _FN3
};


/*
#define LAYOUT( \
         K01, K02, K03, K04, \
         K11, K12, K13, K14, \
         K21, K22, K23, K24, \
    K30, K31, K32, K33, \
    K40,   K42,    K43, K44  \
) { \

*/

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_BASE] = LAYOUT(
           KC_DLR, KC_PERC, KC_PSLS, KC_PAST, 
           KC_P7,  KC_P8,   KC_P9,   KC_PMNS, 
           KC_P4,  KC_P5,   KC_P6,   KC_PPLS, 
KC_MUTE,   KC_P1,  KC_P2,   KC_P3, 
KC_BSPACE, KC_P0,  KC_PDOT, KC_PENT),


// YouTube layer
    [_FN1] = LAYOUT(
            MK_YT_HOME,        MK_YT_SUBSCRIPTNS,  MK_YT_WATCHLATER, MK_YT_HISTORY, 
            MK_YT_PREVCH,      KC_UP,              MK_YT_NEXTCH,     MK_IOS_BRIDOWN, 
            MK_YT_REWIND,      MK_YT_PLAY,         MK_YT_FASTFORWD,  MK_IOS_BRIDUP, 
 _______,   MK_YT_SPEEDDOWN,   KC_DOWN,            MK_YT_SPEEDUP,  
 _______,                      MK_YT_FULLSCVW,     MK_YT_THEATERVW,  _______),


/* iPad Layer   
   MK_IOS_QUICKNOTE, //Quick Note, World+Q
*/
    [_FN2] = LAYOUT(
              MK_IOS_APPSWITCHER, MK_IOS_HOME, MK_IOS_NOTIFICATION, MK_IOS_CONTROLCENTER, 
              MK_IOS_PREVAPP,     KC_UP,       MK_IOS_NEXTAPP,      MK_IOS_BRIDOWN, 
              KC_LEFT,            MK_IOS_PLAY, KC_RIGHT,            MK_IOS_BRIDUP, 
 _______,     MK_IOS_PREVTRACK,   KC_DOWN,     MK_IOS_NEXTTRACK,  
 KC_APPLE_FN,                     _______,     MK_IOS_QUICKNOTE,    _______),


/*
    FUNC Key Layer
*/
    [_FN3] = LAYOUT(
              _______, _______, _______, _______, 
              KC_F7,   KC_F8,   KC_F9,   MK_IOS_BRIDOWN, 
              KC_F4,   KC_F5,   KC_F6,   MK_IOS_BRIDUP, 
 _______,     KC_F1,   KC_F2,   KC_F3,  
 KC_APPLE_FN,          _______, _______, _______)

};




//
// Layer management functions
//

// 
// void switchToNextLayer(bool advance)
// param
//      advance:  true, advance to the next layer
//                false, go to the previous layer
//  
void switchToNextLayer(bool advance);
layer_state_t getNextLayer(bool advance);


bool process_hold_switch_layer_key(uint16_t keycode);
bool process_tap_switch_layer_key(uint16_t keycode);
bool process_hold_rotary_encoder_key(uint16_t keycode);
bool process_tap_rotary_encoder_key(uint16_t keycode);

const keypos_t switch_layer_keypos = { .row= 0, .col= 4 };
press_and_hold_key_t switch_layer_key;
const keypos_t rotary_encoder_keypos = { .row= 3, .col= 0 };
press_and_hold_key_t rotary_encoder_key;

bool editing_session_mode = false;


void keyboard_post_init_user(void) 
{    
  // Customise these values to desired behaviour
  debug_enable=true;
  debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;

  read_user_config();

  switch_layer_key = make_press_and_hold_key(
        switch_layer_keypos,
        process_hold_switch_layer_key,
        process_tap_switch_layer_key
    );


  rotary_encoder_key = make_press_and_hold_key(
        rotary_encoder_keypos,
        process_hold_rotary_encoder_key,
        process_tap_rotary_encoder_key
    );

  initialize_maxmix();
}


bool process_record_user(uint16_t keycode, keyrecord_t *record) 
{

  // If console is enabled, it will print the matrix position and status of each key pressed
#ifdef CONSOLE_ENABLE
    uprintf("KL: kc: 0x%04X, c: %u, r: %u, pressed: %b, time: %u, interrupt: %b, count: %u\n",
    keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
#endif 

    if (! process_press_and_hold_key(&switch_layer_key, keycode, record))
    {
        return false;
    }

    if (! process_press_and_hold_key(&rotary_encoder_key, keycode, record))
    {
        return false;
    }

    if (record->event.pressed)
    {
        if (!process_mone_key(keycode)) 
        {
            return false;
        }
    }

    return true;
    
}



layer_state_t getNextLayer(bool advance) 
{
    const layer_state_t curr_layer = get_highest_layer(layer_state);

    layer_state_t next_layer = curr_layer;
    if (advance)
    {
        if (curr_layer == _LAST) {
            next_layer = _BASE;
        }
        else {
            next_layer = curr_layer + 1;
        }
    }
    else
    {
        if (curr_layer == _BASE) {
            next_layer = _LAST;
        }
        else {
            next_layer = curr_layer - 1;
        }
    }

#ifdef DEBUG_LAYER
    uprintf("getNextLayer(%d): %d\n", advance, next_layer);
#endif 

    return next_layer;
}

void switchToNextLayer(bool advance) 
{
    const layer_state_t next_layer = getNextLayer(advance);

    layer_clear();
    layer_on(_BASE);
    layer_on(next_layer);

#ifdef DEBUG_LAYER
   uprintf("KL: switchToNextLayer: layer_on = _BASE + %u\n", get_highest_layer(layer_state));
#endif 
}


#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_180;  // flips the display 180 degrees9*-+
}

// default screen size: 128x32 

void render_status(void) {

    // Host Keyboard Layer Status
    if (editing_session_mode)
    {
        oled_write_P(PSTR("Editing Audio"), false);
        oled_write_ln_P(PSTR("Session"), false);
        oled_write_ln_P(PSTR(""), false);
        oled_write_P(PSTR("Audio: "), false);
        oled_write_ln(curr_session_data.name, false);        
    }
    else
    {
        //oled_write_P(PSTR("Hi Mone!\n"), false);

        oled_write_P(user_config.is_win_mode ? PSTR("Mode: Win\n"): PSTR("Mode: Mac\n"), false);

        oled_write_P(PSTR("Layer: "), false);

        switch (get_highest_layer(layer_state)) {
            case _BASE:
                oled_write_P(PSTR("Base\n"), false);
                break;
            case _FN1:
                oled_write_P(PSTR("YouTube\n"), false);
                break;
            case _FN2:
                oled_write_P(PSTR("iOS\n"), false);
                break;
            case _FN3:
                oled_write_P(PSTR("Fn Keys\n"), false);
                break;
            default:
                // Or use the write_ln shortcut over adding '\n' to the end of your string
                oled_write_ln_P(PSTR("Undefined"), false);
        }

        oled_write_P(PSTR("Audio: "), false);
        oled_write_ln(curr_session_data.name, false);
    }

// #ifdef TEST_SUBMIT_WEBPAGE_TIMING
//     char szBuf[50] = {0};
//     sprintf(szBuf, "pre:%ld  post:%ld\n", (long)presubmit_webpage_wait_time, (long)postsubmit_webpage_wait_time);
//     oled_write(szBuf, false);
// #endif

    return;
}


bool oled_task_user(void) {
    render_status();
    return false;
}

#endif //OLED_ENABLE



// return true if qmk should continue processing the pressed key 
bool process_hold_switch_layer_key(uint16_t keycode)
{
#ifdef DEBUG_LAYER
    uprintf("KL: process_hold_switch_layer_key\n"),
#endif    

    switchToNextLayer(true);
    return false;
}

// return true if qmk should continue processing the pressed key 
bool process_tap_switch_layer_key(uint16_t keycode)
{
#ifdef DEBUG_LAYER
    uprintf("KL: process_tap_switch_layer_key\n");
#endif    
    return true;
}


// return true if qmk should continue processing the pressed key 
bool process_hold_rotary_encoder_key(uint16_t keycode)
{
#ifdef DEBUG_EDIT_SESSION
    uprintf("KL: rotaryEncoderKey_Pos is hold, TODO: change audio channel\n");
#endif 
    editing_session_mode = !editing_session_mode;
    return false;
}

// return true if qmk should continue processing the pressed key 
bool process_tap_rotary_encoder_key(uint16_t keycode)
{
#ifdef DEBUG_LAYER
    uprintf("KL: rotaryEncoderKey_Pos is pressed, switch_layer_key.state=%d\n", (int)switch_layer_key.state);
#endif     

    if (editing_session_mode)
    {
#ifdef DEBUG_EDIT_SESSION
    uprintf("KL: rotaryEncoderKey_Pos is pressed, editing_session_mode=%d\n", (int)editing_session_mode);
#endif 

        editing_session_mode = false;        
        return false;
    }
    // pressing rotary encoder button while holding switchLayerKey will 
    // switch between win and mac  mode.
    else if (press_and_hold_key_is_pressed(&switch_layer_key))
    {
        user_config.is_win_mode = !user_config.is_win_mode;

        save_user_config();

        set_press_and_hold_key_to_handled(&switch_layer_key);

        return false;
    }
    else
    {
        return true;
    }
}


#ifndef VIAL_ENCODERS_ENABLE
bool mone_encoder_update(uint8_t index, bool clockwise);

bool encoder_update_kb(uint8_t index, bool clockwise)
{
    return mone_encoder_update(index, clockwise);
}
#endif

bool mone_encoder_update(uint8_t index, bool clockwise) 
{
#ifdef DEBUG_EDIT_SESSION
        uprintf("KL: mone_encoder_update(index=%d, clockwise=%d)\n", (int)index, (int)clockwise);
#endif 

    if (editing_session_mode)
    {
#ifdef DEBUG_EDIT_SESSION
        uprintf("KL: mone_encoder_update: editing_session_mode == true, changing curr session\n");
#endif 
        if (clockwise)
        {
            edit_prev_session();
        }
        else
        {
            edit_next_session();
        }
        return true;
    }
    //  also handle switchLayerKey_pressed_and_handled, because the dial can be turnt more than once
    else if (press_and_hold_key_is_pressed(&switch_layer_key)) 
    {
#ifdef DEBUG_LAYER
        uprintf("KL: mone_encoder_update: switchLayerKey_state == switchLayerKey_pressed, changing layer\n");
#endif 
        if (clockwise)
        {
            switchToNextLayer(false);
        }
        else 
        {
            switchToNextLayer(true);
        }

        set_press_and_hold_key_to_handled(&switch_layer_key);

        return true;
    }
    else
    {
#ifdef VIAL_ENCODERS_ENABLE        
        if (! maxmix_is_running())
        {
#ifdef DEBUG_MAXMIX
            uprintf("KL: mone_encoder_update !maxmix_is_running\n");
#endif 
            return vial_encoder_update(index, clockwise);
        }

        uint16_t keycode = vial_get_encoder_keycode(index, clockwise);
        bool encoder_is_volumne_control = ((keycode == KC_VOLD) && clockwise) || ((keycode == KC_VOLU) && !clockwise);
        if (! encoder_is_volumne_control)
        {
#ifdef DEBUG_MAXMIX
            uprintf("KL: mone_encoder_update !encoder_is_volumne_control\n");
#endif 
            return vial_encoder_update(index, clockwise);
        }        
#endif

        if (clockwise)
        {
            dec_curr_session_volumne();
        }
        else
        {
            inc_curr_session_volumne();
        }
        return true;

    }
}


void raw_hid_receive_kb(uint8_t *data, uint8_t length) {

#ifdef DEBUG_EDIT_SESSION
    uprintf("KL: raw_hid_receive_kb(length=%d)\n", (int)length);
#endif 

    uint8_t *command_id = &data[0];

#ifdef DEBUG_EDIT_SESSION
    uprintf("KL: raw_hid_receive_kb(command_id=0x%02X)\n", *command_id);
#endif 

    if (*command_id != id_mone_prefix) {
        // Unhandled message.
        *command_id = id_unhandled;
        //*command_data = *command_data; // force use of variable
        return;
    }

    uint8_t *maxmix_data = &data[1];
    uint8_t maxmix_len = length - 1;
    handle_maxmix_command(
        maxmix_data,
        maxmix_len
        );
}


