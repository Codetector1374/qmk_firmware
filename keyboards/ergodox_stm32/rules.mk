SRC = matrix.c

MCU_FAMILY = STM32
MCU_SERIES = STM32F1xx

MCU_LDSCRIPT = stm32f103_bootloader

MCU_STARTUP = stm32f1xx

BOARD = ERGODOX_STM32_BOARD

MCU = cortex-m3

ARMV = 7

OPT_DEFS =

BOOTMAGIC_ENABLE = no
MOUSEKEY_ENABLE = no	# Mouse keys
EXTRAKEY_ENABLE = yes	# Audio control and System control
CONSOLE_ENABLE = no	# Console for debug
COMMAND_ENABLE = no    # Commands for debug and configuration
SLEEP_LED_ENABLE = no  # Breathing sleep LED during USB suspend
NKRO_ENABLE = yes	    # USB Nkey Rollover
CUSTOM_MATRIX = yes # Custom matrix file
BACKLIGHT_ENABLE = no
VISUALIZER_ENABLE = no
