// Copyright 2022 monehsieh (@monehsieh)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define VIAL_KEYBOARD_UID {0xE1, 0x03, 0xDC, 0xC8, 0xB3, 0x98, 0x04, 0x18}

// top 2 keys of right most column
#define VIAL_UNLOCK_COMBO_ROWS {0,1}
#define VIAL_UNLOCK_COMBO_COLS {4,4}


#ifdef VIAL_ENCODERS_ENABLE
	#define VIAL_ENCODER_DEFAULT { KC_VOLU, KC_VOLD, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS }

	//override encoder_update_kb, this allows us to customize encoder behaviour 
	// 1. enable holding key(0,4) while turning encoder0 will change layers
	// 2. support changing audio sessions.
	#define VIAL_MONE_ENCODERS_ENABLE
#endif

#define MONE_HID_ENABLE


// reduce RAM and EEPROM usage
#define VIAL_COMBO_ENTRIES 4



// #define DEBUG_EDIT_SESSION
// #define DEBUG_MAXMIX
// #define DEBUG_MAXMIX_DEBUG
// #define DEBUG_RECEIVE_KB
// #define DEBUG_LAYER
// #define DEBUG_SUBMIT_WEBPAGE_TIMING
// #define DEBUG_PRESS_AND_HOLD    1
// #define DEBUG_FNKEY
