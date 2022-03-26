# MCU name
MCU = atmega32u4

# Bootloader selection
BOOTLOADER = atmel-dfu

# Build Options
#   change yes to no to disable
#
BOOTMAGIC_ENABLE = yes      # Enable Bootmagic Lite

#set to no to reduce firmware size
MOUSEKEY_ENABLE = no       # Mouse keys
EXTRAKEY_ENABLE = yes       # Audio control and System control
CONSOLE_ENABLE = yes         # Console for debug
COMMAND_ENABLE = no         # Commands for debug and configuration
NKRO_ENABLE = no            # Enable N-Key Rollover
BACKLIGHT_ENABLE = no       # Enable keyboard backlight functionality
RGBLIGHT_ENABLE = no        # Enable keyboard RGB underglow
AUDIO_ENABLE = no           # Audio output

ENCODER_ENABLE = yes

# LTO makes the compiler work harder when optimizing your code, resulting in a smaller firmware size
LTO_ENABLE = yes
# resulting in a smaller firmware size
QMK_SETTINGS = no
TAP_DANCE_ENABLE = no


# OLED driver SSD1306
# High resolution: 128 x 64 
OLED_ENABLE = yes
OLED_DRIVER = SSD1306

# incompatible with APPLE_FN_ENABLE
NKRO_ENABLE = no

#set to no to reduce firmware size
#EXTRAFLAGS+=-flto
#define NO_ACTION_MACRO
#define NO_ACTION_FUNCTION

SRC += user_config.c
