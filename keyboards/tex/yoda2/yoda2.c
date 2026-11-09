#include "quantum.h"
#include "dip_switch.h"
#include "keycode_config.h"

void keyboard_pre_init_kb(void) {
    static const pin_t rows[MATRIX_ROWS] = MATRIX_ROW_PINS;
    static const pin_t cols[MATRIX_COLS] = MATRIX_COL_PINS;
    static const pin_t dips[NUM_DIP_SWITCHES] = DIP_SWITCH_PINS;
    // HT32 AF1 selects GPIO. Unselected matrix lines remain pulled-up inputs.
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        palSetLineMode(rows[row], PAL_MODE_INPUT_PULLUP | PAL_HT32_MODE_AF(1));
    }
    for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
        palSetLineMode(cols[col], PAL_MODE_INPUT_PULLUP | PAL_HT32_MODE_AF(1));
    }
    for (uint8_t index = 0; index < NUM_DIP_SWITCHES; ++index) {
        palSetLineMode(dips[index], PAL_MODE_INPUT_PULLUP | PAL_HT32_MODE_AF(1));
    }
    gpio_write_pin_high(LED_CAPS_LOCK_PIN);
    palSetLineMode(LED_CAPS_LOCK_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_HT32_MODE_AF(1));
    keyboard_pre_init_user();
}

bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
        return false;
    }
    if (index == 0 && (keymap_config.swap_lalt_lgui != active || keymap_config.swap_ralt_rgui != active)) {
        // Release the old modifier identities before changing their interpretation.
        clear_keyboard();
        keymap_config.swap_lalt_lgui = active;
        keymap_config.swap_ralt_rgui = active;
    }
    return true;
}
