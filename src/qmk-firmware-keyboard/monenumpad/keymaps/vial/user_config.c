#include QMK_KEYBOARD_H

#include "user_config.h"


user_config_t user_config;

void read_user_config() {
    user_config.raw = eeconfig_read_user();
}

void save_user_config()
{
    eeconfig_update_user(user_config.raw);
}






