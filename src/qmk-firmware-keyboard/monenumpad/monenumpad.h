// Copyright 2022 monehsieh (@monehsieh)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"

/* This is a shortcut to help you visually see your layout.
 *
 * The first section contains all of the arguments representing the physical
 * layout of the board and position of the keys.
 *
 * The second converts the arguments into a two-dimensional array which
 * represents the switch matrix.
 */

/*
#define LAYOUT( \
    k00, k01, k02, \
      k10,  k12    \
) { \
    { k00, k01,   k02 }, \
    { k10, KC_NO, k12 }  \
}
*/

/* Mone numpad matrix layout
   * ,----------------------- .
   * |    | 01 | 02 | 03 | 04 |
   * |----|----|----|----|----|
   * |    | 11 | 12 | 13 | 14 |
   * |----|----|----|----|----|
   * |    | 21 | 22 | 23 | 24 |
   * |----|----|----|----|----|
   * | 30 | 31 | 32 | 33 |    |
   * |----|---------|----| 44 |
   * | 40 |   42    | 43 |    |
   * `------------------------'
 */


#define LAYOUT( \
         K01, K02, K03, K04, \
         K11, K12, K13, K14, \
         K21, K22, K23, K24, \
    K30, K31, K32, K33, \
    K40,      K42, K43, K44  \
) { \
    { KC_NO, K01,   K02,   K03,   K04 }, \
    { KC_NO, K11,   K12,   K13,   K14 }, \
    { KC_NO, K21,   K22,   K23,   K24 }, \
    { K30,   K31,   K32,   K33,   KC_NO }, \
    { K40,   KC_NO, K42,   K43,   K44 }  \
}

