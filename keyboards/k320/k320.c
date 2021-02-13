#include "k320.h"
#include "string.h"
#include "raw_hid.h"

__attribute((weak)) void matrix_scan_user(void) {
}

void matrix_scan_kb(void) {
    matrix_scan_user();
}

__attribute((weak)) void matrix_init_user(void) {
}

void matrix_init_kb(void) {
    matrix_init_user();
}

void bootloader_jump(void) {
    *((volatile uint32_t*) 0x200001FC) = 0xB0BACAFE;
    __asm__ __volatile__("dsb");
    SCB->AIRCR = 0x05FA0004; // Sys Reset
    __asm__ __volatile__("dsb");
    while(1) {}
}

bool led_update_kb(led_t status) {
    if (status.caps_lock) {
    	palClearLine(LINE_LED0);
    } else {
    	palSetLine(LINE_LED0);
    }
    return led_update_user(status);
}

void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (length >= 2) {
        // Set Default Layer
        if (data[0] == 0x01) {
            if (data[1] < k320_max_layer) {
                set_single_persistent_default_layer(data[1]);
            }
            return;
        }
    }
    if (length >= 1) {
        if (data[0] == 0x69) {
            palToggleLine(LINE_LED3);
        }
    }
}