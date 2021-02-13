SRC = matrix.c

LAYOUTS +=

# MCU
MCU = cortex-m0
ARMV = 6
USE_FPU = no
MCU_FAMILY = STM32
MCU_SERIES = STM32F0xx
MCU_LDSCRIPT = stm32f070_bl
MCU_STARTUP = stm32f0xx

EEPROM_DRIVER = transient

OPT_DEFS = -Wno-unused-function -fdump-rtl-dfinish -fstack-usage
#EXTRALDFLAGS = -Wl,--print-memory-usage

# Options
BOARD = K320_BOARD

# Keys
CUSTOM_MATRIX = yes
NKRO_ENABLE = yes
MOUSEKEY_ENABLE = no
EXTRAKEY_ENABLE = yes
KEY_LOCK_ENABLE = no

# Other featues
BOOTMAGIC_ENABLE = no
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
RAW_ENABLE = yes
MIDI_ENABLE = no
VIRTSER_ENABLE = no
COMBO_ENABLE = no
UNICODE_ENABLE = no
