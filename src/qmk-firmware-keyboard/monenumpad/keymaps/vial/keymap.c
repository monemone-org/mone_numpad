// Copyright 2022 monehsieh (@monehsieh)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <stdio.h>

#define MACRO_TIMER 10

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE,
    _FN1,
    _FN2,
    _FN3,
    _LAST = _FN3
};


enum MoneKeys {
   //YouTube Layer
   MK_YT_REWIND = SAFE_RANGE,  //YouTube Rewind
   MK_YT_FASTFORWD,  //YouTube Fast forward
   MK_YT_SPEEDUP, //YouTube Speed up
   MK_YT_SPEEDDOWN, //YouTube Speed down
   MK_YT_NEXTCH, //YouTube Next Chapter
   MK_YT_PREVCH, //YouTube Previous Chapter
   MK_YT_NEXTVID, //YouTube Next Video
   MK_YT_PREVVID, //YouTube Previous Video
   MK_YT_FULLSC, //YouTube Full Screen Player
   MK_YT_MINISC, //YouTube Mini Player
   MK_YT_PLAY, //YouTube play/pause
   MK_YT_WATCHLATER, //YouTube open watch later playlist
   MK_YT_SUBSCRIPTNS,

   //iOS Layer
   MK_IOS_HOME, //iOS home screen, WORD+H
   MK_IOS_SHOWKEED, //iOS show soft keyboard    
   MK_IOS_PREVTRACK, //iOS previous track  KC_MEDIA_PREV_TRACK
   MK_IOS_NEXTTRACK, //iOS next track  KC_MEDIA_NEXT_TRACK
   MK_IOS_PLAY, // iOS Play/pause media  KC_MEDIA_PLAY_PAUSE
   MK_IOS_SEARCH, //Spotlight Search. Cmd+Space
   MK_IOS_DOCK,   //Show Dock, World+A
   MK_IOS_APPLIB, //Show App Lib, Shift+World+A
   MK_IOS_QUICKNOTE, //Quick Note, World+Q
   MK_IOS_CONTROLCENTER, //Control Center, World+C
   MK_IOS_NOTIFICATION, //Notification Center, World+N
   MK_IOS_APPSWITCHER, //App Switcher, World+Up
   MK_IOS_PREVAPP, //Previous App, World+Left
   MK_IOS_NEXTAPP, //Next App, World+Right
   MK_IOS_BRIDOWN,
   MK_IOS_BRIDUP
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
            MK_YT_WATCHLATER,  MK_YT_SUBSCRIPTNS,  MK_YT_SPEEDDOWN,  MK_YT_SPEEDUP, 
            MK_YT_PREVCH,      _______,            MK_YT_NEXTCH,     MK_IOS_BRIDOWN, 
            MK_YT_REWIND,      MK_YT_PLAY,         MK_YT_FASTFORWD,  MK_IOS_BRIDUP, 
 _______,   MK_YT_PREVVID,     _______,            MK_YT_NEXTVID,  
 _______,                      MK_YT_FULLSC,       MK_YT_MINISC,     MK_YT_PLAY),


/* iPad Layer   
   MK_IOS_QUICKNOTE, //Quick Note, World+Q
*/
    [_FN2] = LAYOUT(
              MK_IOS_APPSWITCHER, MK_IOS_HOME, MK_IOS_NOTIFICATION, MK_IOS_CONTROLCENTER, 
              MK_IOS_PREVAPP,     KC_UP,       MK_IOS_NEXTAPP,      MK_IOS_BRIDOWN, 
              KC_LEFT,            MK_IOS_PLAY, KC_RIGHT,            MK_IOS_BRIDUP, 
 _______,     MK_IOS_PREVTRACK,   KC_DOWN,     MK_IOS_NEXTTRACK,  
 KC_APPLE_FN,                     _______,     MK_IOS_QUICKNOTE,    KC_ENTER),


/*
    FUNC Key Layer
*/
    [_FN3] = LAYOUT(
              _______, _______, _______, _______, 
              KC_F7,   KC_F8,   KC_F9,   _______, 
              KC_F4,   KC_F5,   KC_F6,   _______, 
 _______,     KC_F1,   KC_F2,   KC_F3,  
 KC_APPLE_FN,          _______, _______, _______)

};



void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  debug_enable=true;
  debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;
}

enum switch_layer_key_state {
    switchLayerKey_not_pressed = 0,
    switchLayerKey_pressed = 1,
    switchLayerKey_pressed_and_handled = 2
};

static keypos_t switchLayerKey_Pos = { .row= 0, .col= 4 };
static uint16_t switchLayerKey_state = switchLayerKey_not_pressed;
static uint16_t switchLayerKey_pressed_timer;

const uint8_t keyboard_toggle = 0x65;

// function declarations

/**
 * void switchToNextLayer(bool advance)
 * @param
 *      advance:  true, advance to the next layer
 *                false, go to the previous layer
 *  
*/
void switchToNextLayer(bool advance);
layer_state_t getNextLayer(bool advance);

bool tapMoneKeyCode(uint16_t keycode);

bool process_record_user(uint16_t keycode, keyrecord_t *record) {


  // If console is enabled, it will print the matrix position and status of each key pressed
#ifdef CONSOLE_ENABLE
    uprintf("KL: kc: 0x%04X, col: %u, row: %u, pressed: %b, time: %u, interrupt: %b, count: %u\n",
     keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
#endif 

    // if the right bottom k44 is pressed & hold, tranverse to the next layer.
    if (record->event.key.col == switchLayerKey_Pos.col && record->event.key.row == switchLayerKey_Pos.row) 
    {
        // if switchLayerKey is pressed
        if (record->event.pressed) 
        {
            if (switchLayerKey_state == switchLayerKey_not_pressed) 
            {
#ifdef CONSOLE_ENABLE
                uprintf("KL: switchLayerKey_state = switchLayerKey_pressed\n");
#endif 
                switchLayerKey_state = switchLayerKey_pressed;
                switchLayerKey_pressed_timer = timer_read();
            }

        }
        else // !record->event.pressed
        {
            // means the key is pressed for less than TAPPING_TERM
            if (switchLayerKey_state == switchLayerKey_pressed)
            {
                if (timer_elapsed(switchLayerKey_pressed_timer) < TAPPING_TERM)
                {
#ifdef CONSOLE_ENABLE
                    uprintf("KL: switchLayerKey_pressed, tap_code16(0x%04X);\n", keycode);
#endif 
                    if (! tapMoneKeyCode(keycode)) 
                    {
                        tap_code16(keycode);
                    }
                }
                else //(timer_elapsed(switchLayerKey_pressed_timer) >= TAPPING_TERM)
                {
#ifdef CONSOLE_ENABLE
                    uprintf("KL: switchLayerKey_pressed and holded, switchToNextLayer()\n");
#endif 
                    switchToNextLayer(true);
                }                
            } //if (switchLayerKey_state == switchLayerKey_pressed)

            switchLayerKey_state = switchLayerKey_not_pressed;
        }    

        return false;
    }
    else  // else if not switchLayerKey
    {
        switchLayerKey_state = switchLayerKey_not_pressed;

        if (record->event.pressed)
        {
            tapMoneKeyCode(keycode);
        }

        return true;
    }
    
}


//return true if keycode is Mone defined and has been handled in tapMoneKeyCode
bool tapMoneKeyCode(uint16_t keycode)
{
    bool handled = true;

    //Example:
    //  SEND_STRING(SS_LALT(SS_TAP(X_V)SS_TAP(X_ENTER)));
    switch (keycode)
    {
        case MK_YT_REWIND:
            SEND_STRING(SS_TAP(X_LEFT));
            break;
        case MK_YT_FASTFORWD:
            SEND_STRING(SS_TAP(X_RIGHT));
            break;
        case MK_YT_SPEEDUP:
            SEND_STRING(SS_LSFT(">"));
            break;
        case MK_YT_SPEEDDOWN:
            SEND_STRING(SS_LSFT("<"));
            break;
        case MK_YT_NEXTCH:
            SEND_STRING(SS_LALT(SS_TAP(X_RIGHT)));
            break;
        case MK_YT_PREVCH:
            SEND_STRING(SS_LALT(SS_TAP(X_LEFT)));
            break;
        case MK_YT_NEXTVID:
            SEND_STRING(SS_LSFT(SS_TAP(X_N)));
            break;
        case MK_YT_PREVVID:
            SEND_STRING(SS_LGUI("[")); // CMD + [
            break;
        case MK_YT_FULLSC:
            SEND_STRING(SS_TAP(X_F));
            break;
        case MK_YT_MINISC:
            SEND_STRING(SS_TAP(X_I));
            break;
        case MK_YT_PLAY:
            SEND_STRING(SS_TAP(X_SPACE));
            break;
        case MK_YT_WATCHLATER:
            SEND_STRING(SS_LGUI(SS_TAP(X_L)));
            SEND_STRING("https://www.youtube.com/playlist?list=WL");
            wait_ms(MACRO_TIMER);
            SEND_STRING(SS_TAP(X_ENTER));
            break;
        case MK_YT_SUBSCRIPTNS:
            SEND_STRING(SS_LGUI(SS_TAP(X_L)));
            SEND_STRING("https://www.youtube.com/feed/subscriptions");
            wait_ms(MACRO_TIMER);
            SEND_STRING(SS_TAP(X_ENTER));
            break;

        case MK_IOS_HOME: // FN+H
            register_code16(KC_APPLE_FN);
            tap_code(KC_H);
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_SHOWKEED:
            break;

        case MK_IOS_PREVTRACK:  //iOS previous track  KC_MEDIA_PREV_TRACK
            SEND_STRING(SS_TAP(X_MEDIA_PREV_TRACK));
            break;

        case MK_IOS_NEXTTRACK: //iOS next track  KC_MEDIA_NEXT_TRACK
            SEND_STRING(SS_TAP(X_MEDIA_NEXT_TRACK));
            break;

        case MK_IOS_PLAY: // iOS Play/pause media  KC_MEDIA_PLAY_PAUSE
            SEND_STRING(SS_TAP(X_MEDIA_PLAY_PAUSE));
            break;

        case MK_IOS_SEARCH: //Spotlight Search. Cmd+Space
            SEND_STRING(SS_LGUI(SS_TAP(X_SPACE)));
            break;

        case MK_IOS_DOCK: //Show Dock, World+A
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_A));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_APPLIB: //Show App Lib, Shift+World+A
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_LSFT(SS_TAP(X_A)));
            unregister_code16(KC_APPLE_FN);
           break;

        case MK_IOS_QUICKNOTE:  //Quick Note, World+Q
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_Q));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_CONTROLCENTER: //Control Center, World+C
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_C));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_NOTIFICATION: //Notification Center, World+N
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_N));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_APPSWITCHER: //App Switcher, World+Up
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_UP));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_PREVAPP: //Previous App, World+Left
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_LEFT));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_NEXTAPP: //Next App, World+Right
            register_code16(KC_APPLE_FN);
            SEND_STRING(SS_TAP(X_RIGHT));
            unregister_code16(KC_APPLE_FN);
            break;

        case MK_IOS_BRIDOWN:
            SEND_STRING(SS_TAP(X_BRIGHTNESS_DOWN));
            break;

        case MK_IOS_BRIDUP:
            SEND_STRING(SS_TAP(X_BRIGHTNESS_UP));
            break;

        default:
            handled = false;
            break;

    } //switch (keycode) 

    return handled;
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

#ifdef CONSOLE_ENABLE
    uprintf("getNextLayer(%d): %d\n", (int)advance, next_layer);
#endif 

    return next_layer;
}

void switchToNextLayer(bool advance) 
{
    const layer_state_t next_layer = getNextLayer(advance);

    layer_clear();
    layer_on(_BASE);
    layer_on(next_layer);

#ifdef CONSOLE_ENABLE
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
    oled_write_P(PSTR("Hi Mone!\n"), false);

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

    
    //oled_write_P(szKeyCode, false);
    return;
}


// void render_logo(void) {
//     // 8x8 byes = 64 byes
//     static const char PROGMEM qmk_logo[] = {
//         0x80, 0x81, 0x82, 0x83,  0x84, 0x85, 0x86, 0x87, 
//         0x88, 0x89, 0x8A, 0x8B,  0x8C, 0x8D, 0x8E, 0x8F,
//         0x90, 0x91, 0x92, 0x93,  0x94, 0xA0, 0xA1, 0xA2, 
//         0xA3, 0xA4, 0xA5, 0xA6,  0xA7, 0xA8, 0xA9, 0xAA, 
        
//         0xAB, 0xAC, 0xAD, 0xAE,  0xAF, 0xB0, 0xB1, 0xB2, 
//         0xB3, 0xB4, 0xC0, 0xC1,  0xC2, 0xC3, 0xC4, 0xC5, 
//         0xC6, 0xC7, 0xC8, 0xC9,  0xCA, 0xCB, 0xCC, 0xCD, 
//         0xCE, 0xCF, 0xD0, 0xD1,  0xD2, 0xD3, 0xD4, 0x00
//     };

//     oled_write_P(qmk_logo, false);
// }

bool oled_task_user(void) {
    render_status();  // Renders the current keyboard state (layer, lock, caps, scroll, etc)

    // render_logo();  // Renders a static logo
    // oled_scroll_left();  // Turns on scrolling
    return false;
}


bool mone_encoder_update(uint8_t index, bool clockwise) 
{
    //  also handle switchLayerKey_pressed_and_handled, because the dial can be turnt more than once
    if (switchLayerKey_state == switchLayerKey_pressed || switchLayerKey_state == switchLayerKey_pressed_and_handled) 
    {
#ifdef CONSOLE_ENABLE
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

        switchLayerKey_state = switchLayerKey_pressed_and_handled;

        return true;
    }
    else
    {
        return vial_encoder_update(index, clockwise);
    }
}



#endif



