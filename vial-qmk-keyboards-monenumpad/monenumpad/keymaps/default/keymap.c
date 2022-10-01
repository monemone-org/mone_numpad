// Copyright 2022 monehsieh (@monehsieh)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE,
    _FN
};

/*
#define LAYOUT( \
         K01, K02, K03, K04, \
         K11, K12, K13, K14, \
         K21, K22, K23, K24, \
    K30, K31, K32, K33,      \
    K40,      K42, K43, K44  \
) { \

*/

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_BASE] = LAYOUT(
         KC_DLR, KC_PERC, KC_PSLS, KC_PAST, 
         KC_P7,  KC_P8,   KC_P9,   KC_PMNS, 
         KC_P4,  KC_P5,   KC_P6,   KC_PPLS, 
KC_MUTE, KC_P1,  KC_P2,   KC_P3, 
KC_BSPACE,       KC_P0,   KC_DOT,  KC_PENT),

    [_FN] = LAYOUT(
          _______, _______, _______, _______, 
          _______, _______, _______, _______, 
          _______, _______, _______, _______, 
 _______, _______, _______, _______,  
 _______,          _______, _______, _______)
};


bool encoder_update_user(uint8_t index, bool clockwise) 
{
    const layer_state_t curr_layer = get_highest_layer(layer_state);

#ifdef CONSOLE_ENABLE
    uprintf("KL: encoder: index: %u, clockwise: %u\n", index, (int)clockwise);
#endif

    if(curr_layer == 1) {
        if (clockwise) {
            tap_code(KC_BRIU);
        } else {
            tap_code(KC_BRID);
        }
    }
    else  {
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    }
    return false;
}


void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  debug_enable=true;
  debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;
}


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  // If console is enabled, it will print the matrix position and status of each key pressed
#ifdef CONSOLE_ENABLE
    uprintf("KL: kc: 0x%04X, col: %u, row: %u, pressed: %b, time: %u, interrupt: %b, count: %u\n", keycode, record->event.key.col, record->event.key.row, record->event.pressed, record->event.time, record->tap.interrupted, record->tap.count);
#endif 
  return true;
}


