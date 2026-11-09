#include "hal.h"
#include "bootloader.h"

void mcu_reset(void) {
    NVIC_SystemReset();
}

// The reference DFU bootloader has no software-entry magic.
// This resets normally; forcing DFU requires an external recovery method.
void bootloader_jump(void) {
    mcu_reset();
}
