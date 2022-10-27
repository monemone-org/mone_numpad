// Copyright 2022 monehsieh (@monehsieh)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <stdio.h>
#include <string.h>

#include "user_config.h"
#include "press_and_hold_key.h"
#include "mone_keys.h"
#include "maxmix/maxmix.h"

#define min(a, b) \
    ((a) < (b) ? (a) : (b));

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
enum layers {
    _BASE,
    _FN1,
    _FN2,
    _FN3,
    _LAST = _FN3
};


const char* layer_names[] = {
    "Base",
    "YouTube",
    "iOS",
    "Fn Keys"
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
//     [_BASE] = LAYOUT(
//            KC_DLR, KC_PERC, KC_PSLS, KC_PAST, 
//            KC_P7,  KC_P8,   KC_P9,   KC_PMNS, 
//            KC_P4,  KC_P5,   KC_P6,   KC_PPLS, 
// KC_MUTE,   KC_P1,  KC_P2,   KC_P3, 
// KC_BSPACE, KC_P0,  KC_PDOT, KC_PENT),
    [_BASE] = LAYOUT(
           KC_DLR, KC_PERC, KC_PSLS,     KC_PAST, 
           KC_7,   KC_8,    KC_9,        KC_PMNS, 
           KC_4,   KC_5,    KC_6,        KC_PPLS, 
KC_MUTE,   KC_1,   KC_2,    KC_3, 
MK_FN,             KC_0,    KC_KP_DOT,   KC_PENT),

// YouTube layer
    [_FN1] = LAYOUT(
            MK_YT_HOME,        MK_YT_SUBSCRIPTNS,  MK_YT_WATCHLATER, MK_YT_HISTORY, 
            MK_YT_PREVCH,      MK_YT_SPEEDUP,      MK_YT_NEXTCH,     KC_BRID, 
            MK_YT_REWIND,      MK_YT_PLAY,         MK_YT_FASTFORWD,  KC_BRIU, 
 _______,   MK_YT_PREVCH,      MK_YT_SPEEDDOWN,    MK_YT_NEXTCH,         
 _______,                      MK_YT_CYCLEVW,      MK_YT_FULLSCVW,   MK_YT_PLAY),


/* iPad Layer   
   MK_IOS_QUICKNOTE, //Quick Note, World+Q
*/
    [_FN2] = LAYOUT(
              MK_IOS_APPSWITCHER, MK_IOS_HOME, MK_IOS_NOTIFICATION, MK_IOS_CONTROLCENTER, 
              MK_IOS_PREVAPP,     KC_UP,       MK_IOS_NEXTAPP,      KC_BRID, 
              KC_LEFT,            MK_IOS_PLAY, KC_RIGHT,            KC_BRIU, 
 _______,     MK_IOS_PREVTRACK,   KC_DOWN,     MK_IOS_NEXTTRACK,  
 _______,                         _______,     MK_IOS_QUICKNOTE,    _______),


/*
    FUNC Key Layer
*/
    [_FN3] = LAYOUT(
              KC_F10,  KC_F11,  KC_F12, _______, 
              KC_F7,   KC_F8,   KC_F9,   KC_BRID, 
              KC_F4,   KC_F5,   KC_F6,   KC_BRIU, 
 _______,     KC_F1,   KC_F2,   KC_F3,  
 _______,              KC_INS,  KC_DEL, _______)

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


// bool process_hold_switch_layer_key(uint16_t keycode);
// bool process_tap_switch_layer_key(uint16_t keycode);

bool process_hold_rotary_encoder_key(uint16_t keycode);
bool process_tap_rotary_encoder_key(uint16_t keycode);

bool process_hold_fn_key(uint16_t keycode);
bool process_tap_fn_key(uint16_t keycode);

const keypos_t asterisk_keypos = { .row= 0, .col= 4 };  // * key
const keypos_t fn_keypos = { .row= 4, .col= 0 }; //Fn
press_and_hold_key_t fn_key;

// const keypos_t switch_layer_keypos = { .row= 0, .col= 4 };  // * key
// press_and_hold_key_t switch_layer_key;

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

//   switch_layer_key = make_press_and_hold_key(
//         switch_layer_keypos,
//         process_hold_switch_layer_key,
//         process_tap_switch_layer_key
//     );

    fn_key = make_press_and_hold_key(
        fn_keypos,
        process_hold_fn_key,
        process_tap_fn_key
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
    uprintf("KL: kc: 0x%04X, c: %u, r: %u, pressed: %b, tm: %u, int: %b, cnt: %u\n",
    keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
#endif 

    if (! process_press_and_hold_key(&fn_key, keycode, record))
    {
        return false;
    }

    if (! process_press_and_hold_key(&rotary_encoder_key, keycode, record))
    {
        return false;
    }
 
    if (record->event.pressed)
    {
        // pressing key(c=0,r=4) (the Fn key on the left bottom corner)
        //  while holding asterisk key will switch between Win and Mac mode.
        if (press_and_hold_key_is_pressed(&fn_key))
        {
            if (record->event.key.col == asterisk_keypos.col
                && record->event.key.row == asterisk_keypos.row)
            {
                user_config.is_win_mode = !user_config.is_win_mode;
                save_user_config();
                set_press_and_hold_key_to_handled(&fn_key); //set to handled so it won't process fn when key is up or when key is hold
                return false;
            }
            else if (keycode == KC_KP_DOT) //send backspace
            {
#ifdef CONSOLE_ENABLE
                uprintf("KL: keycode == KC_KP_DOT\n");
#endif 
                tap_code(KC_BACKSPACE);
                set_press_and_hold_key_to_handled(&fn_key); //set to handled so it won't process fn when key is up or when key is hold
                return false;
            }
            else if (keycode == KC_0) //send ins
            {
#ifdef CONSOLE_ENABLE
                uprintf("KL: keycode == KC_0\n");
#endif 
                tap_code(KC_INS);            
                set_press_and_hold_key_to_handled(&fn_key); //set to handled so it won't process fn when key is up or when key is hold
                return false;
            }
        }

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
    uprintf("%s(%d): %d\n", __FUNCTION__, advance, next_layer);
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
   uprintf("KL: %s: layer_on = _BASE + %u\n", __FUNCTION__, get_highest_layer(layer_state));
#endif 
}


#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_180;  // flips the display 180 degrees9*-+
}

// default screen size: 128x32 
//                char size: 20x4
#define OLED_MAX_WIDTH_IN_CHARS   20
char szLine[OLED_MAX_WIDTH_IN_CHARS + 1]; //space firmware size

// return # of lines it rendered
int render_session_name(char* name, int maxLines)
{
    int nLinesRendered = 0;
    size_t nNameLen = strlen(curr_session_data.name);

    size_t nLine1Len = min( strlen(curr_session_data.name), OLED_MAX_WIDTH_IN_CHARS );
    strncpy(szLine, curr_session_data.name, nLine1Len);
    szLine[nLine1Len] = 0; 
    oled_write_ln(szLine, false);
    nLinesRendered += 1;

    if (nNameLen > OLED_MAX_WIDTH_IN_CHARS && maxLines > nLinesRendered)
    {
        //wrap to line2
        size_t nLine2Len = nNameLen - nLine1Len;
        strncpy(szLine, curr_session_data.name+nLine1Len, nLine2Len);
        szLine[nLine2Len] = 0; 
        oled_write_ln(szLine, false);
        
        nLinesRendered += 1;
    }

    return nLinesRendered;
}

void render_status(void) 
{
    // Host Keyboard Layer Status
    if (editing_session_mode)
    {
        oled_write_ln_P(PSTR("Choosing Audio"), false);
        oled_write_ln_P(PSTR("Session:"), false);
        // line 3, 4
        int nLinesRendered = render_session_name(curr_session_data.name, 2);
        if (nLinesRendered == 1)
        {
            //line 4
            oled_write_ln_P(PSTR(""), false);
        }
    }
    else
    {
        //oled_write_P(PSTR("Hi Mone!\n"), false);

        // line 1
        oled_write_P(user_config.is_win_mode ? PSTR("Mode: Win\n"): PSTR("Mode: Mac\n"), false);

        // line 2
        oled_write_P(PSTR("Layer: "), false);

        int layer = get_highest_layer(layer_state);
        const char *layer_name = layer_names[layer];
        oled_write_ln(layer_name, false);

        // line 3
        render_session_name(curr_session_data.name, 1);

        // we only know the volumne data if maxmix is sending us those data.
        if (maxmix_is_running() && !curr_session_data.volume.unknown)
        {
            if (curr_session_data.volume.isMuted)
            {
                oled_write_ln_P(PSTR("Muted"), false);
            }
            else
            {
                uint8_t vol = curr_session_data.volume.volume;
                if (vol > 100) {
                    vol = 100;
                }
                memset(szLine, 0, sizeof(szLine));
                itoa( (int)curr_session_data.volume.volume, szLine, 10 );
                oled_write_P(PSTR("Vol: "), false);
                oled_write_ln(szLine, false);
            }            
        }
        else
        {
            oled_write_ln_P(PSTR(""), false);
        }
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

bool process_hold_fn_key(uint16_t keycode)
{
#ifdef DEBUG_FNKEY
    uprintf("KL: %s\n", __FUNCTION__),
#endif

    //hold fn to switch layer
    switchToNextLayer(true);
    return false;
}

bool process_tap_fn_key(uint16_t keycode)
{
    //tap fn to edit next session
#ifdef DEBUG_FNKEY
    uprintf("KL: %s\n", __FUNCTION__),
#endif    
    edit_next_session(true/**loop back to the first session if reach the last session*/);
    return false;
}


// // return true if qmk should continue processing the pressed key 
// bool process_hold_switch_layer_key(uint16_t keycode)
// {
// #ifdef DEBUG_LAYER
//     uprintf("KL: %s\n", __FUNCTION__),
// #endif    

//     switchToNextLayer(true);
//     return false;
// }

// // return true if qmk should continue processing the pressed key 
// bool process_tap_switch_layer_key(uint16_t keycode)
// {
// #ifdef DEBUG_LAYER
//     uprintf("KL: %s\n",__FUNCTION__);
// #endif    
//     return true;
// }


// return true if qmk should continue processing the pressed key 
bool process_hold_rotary_encoder_key(uint16_t keycode)
{
#ifdef DEBUG_EDIT_SESSION
//    uprintf("KL: TODO: change audio channel\n");
#endif 
    editing_session_mode = !editing_session_mode;
    return false;
}

// return true if qmk should continue processing the pressed key 
bool process_tap_rotary_encoder_key(uint16_t keycode)
{
#ifdef DEBUG_LAYER
    uprintf("KL: %s fn_key.state=%d\n", __FUNCTION__, (int)fn_key.state);
#endif     

    if (editing_session_mode)
    {
#ifdef DEBUG_EDIT_SESSION
    uprintf("KL: %s editing_session_mode=%d\n", __FUNCTION__, (int)editing_session_mode);
#endif 

        editing_session_mode = false;        
        return false;
    }
    else
    {
        if (maxmix_is_running())
        {
            toggle_curr_session_mute();
            return false;
        }

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
        uprintf("KL: %s(i=%d, clockwise=%d)\n", __FUNCTION__, (int)index, (int)clockwise);
#endif 

    if (editing_session_mode)
    {
#ifdef DEBUG_EDIT_SESSION
        uprintf("KL: %s: change curr session\n", __FUNCTION__);
#endif 
        if (clockwise)
        {
            edit_prev_session();
        }
        else
        {
            edit_next_session(false/*if reach the last session, do not loop back.*/);
        }
        return true;
    }
    //  also handle fnKey_pressed_and_handled, because the dial can be turnt more than once
    else if (press_and_hold_key_is_pressed(&fn_key)) 
    {
#ifdef DEBUG_LAYER
        uprintf("KL: %s: switchLayerKey_pressed, changing layer\n", __FUNCTION__);
#endif 
        if (clockwise)
        {
            switchToNextLayer(false);
        }
        else 
        {
            switchToNextLayer(true);
        }

        set_press_and_hold_key_to_handled(&fn_key);

        return true;
    }
    else
    {
#ifdef VIAL_ENCODERS_ENABLE        
        if (! maxmix_is_running())
        {
#ifdef DEBUG_MAXMIX
            uprintf("KL: %s: !maxmix_is_running\n", __FUNCTION__);
#endif 
            return vial_encoder_update(index, clockwise);
        }

        uint16_t keycode = vial_get_encoder_keycode(index, clockwise);
        bool encoder_is_volumne_control = ((keycode == KC_VOLD) && clockwise) || ((keycode == KC_VOLU) && !clockwise);
        if (! encoder_is_volumne_control)
        {
#ifdef DEBUG_MAXMIX
            uprintf("KL: %s: !encoder_is_volumne_control\n", __FUNCTION__);
#endif 
            return vial_encoder_update(index, clockwise);
        }        
#endif // VIAL_ENCODERS_ENABLE

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

#ifdef DEBUG_RECEIVE_KB
    uprintf("KL: %s(length=%d data=%02X %02X %02X %02X %02X)\n", __FUNCTION__, (int)length,
        data[0], data[1], data[2], data[3], data[4]);
#endif 

    uint8_t *command_id = &data[0];

#ifdef DEBUG_RECEIVE_KB
    uprintf("KL: %s: raw_hid_receive_kb(command_id=0x%02X)\n", __FUNCTION__, *command_id);
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


