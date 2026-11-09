#pragma once

// DWT does not provide a usable running counter on this board.
#define WAIT_US_TIMER GPTD_BFTM0

// Module axes: physical up is +X, physical right is +Y.
#define POINTING_DEVICE_ROTATION_90

// Physical DIP switches 1–6, closed to GND when ON.
#define DIP_SWITCH_PINS {B2, B3, B4, B5, C7, C8}
