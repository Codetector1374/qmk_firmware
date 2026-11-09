#include "hal.h"

// Inputs, no pulls, no alternate GPIO assignments. USB, HSE and SWD use AF0.
const PALConfig pal_default_config = {0};

// QMK wraps this hook; clock initialization runs once in hal_lld_init().
void __early_init(void) {}

void boardInit(void) {}
