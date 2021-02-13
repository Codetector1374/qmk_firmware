/*
 * Copyright (c) 2020 Yaotian Feng / Codetector
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <hal.h>
#include "matrix.h"
#include "timer.h"

static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];
static uint32_t debounce_times[MATRIX_ROWS];

extern const ioline_t row_list[MATRIX_ROWS];
extern const ioline_t col_list[MATRIX_COLS];

void matrix_init(void) {
    memset(matrix, 0, MATRIX_ROWS * sizeof(matrix_row_t));
    memset(matrix_debouncing, 0, MATRIX_ROWS * sizeof(matrix_row_t));
    memset(debounce_times, 0, MATRIX_ROWS * sizeof(uint32_t));

    matrix_init_quantum();
}

uint8_t matrix_scan(void) {
    // cache of input ports for columns
    uint16_t port_cache[2];
    // scan each row
    for (int row = 0; row < MATRIX_ROWS; row++) {
        palSetLine(row_list[row]);
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        // read i/o ports
        port_cache[0] = palReadPort(GPIOB);
        port_cache[1] = palReadPort(GPIOC);
        palClearLine(row_list[row]);

        // get row from ports
        matrix_row_t data = 0;
        for (int col = 0; col < MATRIX_COLS; ++col) {
            ioline_t line = col_list[col];
            uint16_t port;
            if (PAL_PORT(line) == GPIOB) {
                port = port_cache[0];
            } else {
                port = port_cache[1];
            }
            data |= (((port & (1 << PAL_PAD(line))) ? 1 : 0) << col); // Inverted
        }

        // if a key event happens <5ms before the system time rolls over,
        // the event will "never" debounce
        // but any event on the same row will reset the debounce timer
        if (matrix_debouncing[row] != data) {
            // whenever row changes restart debouncing
            matrix_debouncing[row] = data;
            debounce_times[row] = timer_read32();
        } else if(debounce_times[row] && timer_elapsed32(debounce_times[row]) >= DEBOUNCE){
            // when debouncing complete, update matrix
            matrix[row] = matrix_debouncing[row];
            debounce_times[row] = 0;
        }
    }

    matrix_scan_quantum();
    return 1;
}

bool matrix_is_on(uint8_t row, uint8_t col) {
    return (matrix[row] & (1 << col));
}

matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}


void matrix_print(void) {
}
