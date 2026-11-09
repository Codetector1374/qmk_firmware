#include QMK_KEYBOARD_H
#include "trackpoint.h"
#include "keycode_config.h"

enum layers { _BASE, _NAV, _FN };
enum custom_keycodes { DRAG_SCROLL = SAFE_RANGE };

static bool scrolling;
static int16_t scroll_h;
static int16_t scroll_v;
enum scroll_axis { SCROLL_UNDECIDED, SCROLL_HORIZONTAL, SCROLL_VERTICAL };
static enum scroll_axis scroll_axis;

#define SCROLL_DIVISOR 32
#define SCROLL_ACTIVATION 8

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSPC,
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_LBRC, KC_RBRC, KC_BSLS,
        LT(_NAV, KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_ENT,
        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
        KC_LCTL, KC_LGUI, KC_LALT, KC_SPC, KC_RALT, KC_RGUI, MO(_FN), KC_RCTL,
        MS_BTN1, DRAG_SCROLL, MS_BTN2
    ),
    [_NAV] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS
    ),
    [_FN] = LAYOUT(
        KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_INS, KC_HOME, KC_PGUP, KC_PSCR, KC_SCRL, KC_TRNS,
        KC_CAPS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_END, KC_PGDN, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_MPRV, KC_MPLY, KC_MNXT, KC_VOLD, KC_VOLU, KC_MUTE, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS
    ),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == DRAG_SCROLL) {
        scrolling = record->event.pressed;
        scroll_h = scroll_v = 0;
        scroll_axis = SCROLL_UNDECIDED;
        trackpoint_clear_motion();
        return false;
    }
    return true;
}

// QMK applies the module's 90-degree correction before this callback.
report_mouse_t pointing_device_task_user(report_mouse_t report) {
    if (scrolling) {
        if (scroll_axis != SCROLL_VERTICAL) {
            scroll_h += report.x;
        }
        if (scroll_axis != SCROLL_HORIZONTAL) {
            scroll_v -= report.y; // Upward stick movement means wheel-up.
        }
        if (scroll_axis == SCROLL_UNDECIDED) {
            int16_t h = scroll_h < 0 ? -scroll_h : scroll_h;
            int16_t v = scroll_v < 0 ? -scroll_v : scroll_v;
            if (h >= SCROLL_ACTIVATION || v >= SCROLL_ACTIVATION) {
                // Prefer vertical for an exact diagonal tie.
                scroll_axis = h > v ? SCROLL_HORIZONTAL : SCROLL_VERTICAL;
            }
        }
        report.h = report.v = 0;
        if (scroll_axis == SCROLL_HORIZONTAL) {
            report.h = scroll_h / SCROLL_DIVISOR;
            scroll_h %= SCROLL_DIVISOR;
            scroll_v = 0;
        } else if (scroll_axis == SCROLL_VERTICAL) {
            report.v = scroll_v / SCROLL_DIVISOR;
            scroll_v %= SCROLL_DIVISOR;
            scroll_h = 0;
        }
        if (keymap_config.swap_lalt_lgui) { // DIP 1: Mac mode.
            report.h = -report.h;
            report.v = -report.v;
        }
        report.x = report.y = 0;
    }
    return report;
}
