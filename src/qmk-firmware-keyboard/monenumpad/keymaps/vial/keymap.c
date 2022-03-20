// Copyright 2022 monehsieh (@monehsieh)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include <stdio.h>

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE,
    _FN1,
    _FN2,
    _FN3
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
KC_MUTE, KC_P1,  KC_P2,   KC_P3, 
KC_BSPACE,    KC_P0   ,   KC_PDOT,  KC_PENT),

    [_FN1] = LAYOUT(
         KC_DLR, KC_PERC, KC_PSLS, KC_PAST, 
         KC_P7,  KC_P8,   KC_P9,   KC_PMNS, 
         KC_P4,  KC_P5,   KC_P6,   KC_PPLS, 
KC_MUTE, KC_P1,  KC_P2,   KC_P3, 
KC_BSPACE,    KC_P0   ,   KC_PDOT,  KC_PENT),

    [_FN2] = LAYOUT(
         KC_DLR, KC_PERC, KC_PSLS, KC_PAST, 
         KC_P7,  KC_P8,   KC_P9,   KC_PMNS, 
         KC_P4,  KC_P5,   KC_P6,   KC_PPLS, 
KC_MUTE, KC_P1,  KC_P2,   KC_P3, 
KC_BSPACE,    KC_P0   ,   KC_PDOT,  KC_PENT),

    [_FN3] = LAYOUT(
         KC_DLR, KC_PERC, KC_PSLS, KC_PAST, 
         KC_P7,  KC_P8,   KC_P9,   KC_PMNS, 
         KC_P4,  KC_P5,   KC_P6,   KC_PPLS, 
KC_MUTE, KC_P1,  KC_P2,   KC_P3, 
KC_BSPACE,    KC_P0   ,   KC_PDOT,  KC_PENT)

};


// bool encoder_update_user(uint8_t index, bool clockwise) 
// {
//     const layer_state_t curr_layer = get_highest_layer(layer_state);

// #ifdef CONSOLE_ENABLE
//     uprintf("KL: encoder: index: %u, clockwise: %u\n", index, (int)clockwise);
// #endif

//     if(curr_layer == _BASE) {
//         if (clockwise) {
//             tap_code(KC_BRIU);
//         } else {
//             tap_code(KC_BRID);
//         }
//     }
//     else  {
//         if (clockwise) {
//             tap_code(KC_VOLU);
//         } else {
//             tap_code(KC_VOLD);
//         }
//     }
//     return false;
// }


void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  debug_enable=true;
  debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;
}


layer_state_t getNextLayer(void) {

    const layer_state_t curr_layer = get_highest_layer(layer_state);

    layer_state_t next_layer = curr_layer + 1;

    if (next_layer > _FN3) {
        next_layer = _BASE;
    }


#ifdef CONSOLE_ENABLE
    uprintf("getNextLayer: %d\n", next_layer);
#endif 

    return next_layer;

}

static char szKeyCode[20] = {0};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {


  // If console is enabled, it will print the matrix position and status of each key pressed
#ifdef CONSOLE_ENABLE
    uprintf("KL: kc: 0x%04X, col: %u, row: %u, pressed: %b, time: %u, interrupt: %b, count: %u\n",
     keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
#endif 

// if the left bottom button(0,4) is pressed , tranverse the different layers one after one.
    if (record->event.key.col == 0 && record->event.key.row == 4) {

        if (record->event.pressed == 1) {
            const layer_state_t next_layer = getNextLayer();

            layer_clear();
            layer_on(next_layer);

    #ifdef CONSOLE_ENABLE
           uprintf("KL: layer_on: %d\n", next_layer);
           uprintf("KL: get_highest_layer: %u\n", get_highest_layer(layer_state));
    #endif 

        }

        return false;
    }

    
  sprintf(szKeyCode, "kc: 0x%04X\n", keycode);
  return true;
}


#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_180;  // flips the display 180 degrees9*-+
}

// default screen size: 128x32 

void render_status(void) {

    // Host Keyboard Layer Status
    oled_write_P(PSTR("Mone Num Pad\n"), false);

    oled_write_P(PSTR("Layer: "), false);

    switch (get_highest_layer(layer_state)) {
        case _BASE:
            oled_write_P(PSTR("Base\n"), false);
            break;
        case _FN1:
            oled_write_P(PSTR("FN1\n"), false);
            break;
        case _FN2:
            oled_write_P(PSTR("FN2\n"), false);
            break;
        case _FN3:
            oled_write_P(PSTR("FN3\n"), false);
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

#endif



