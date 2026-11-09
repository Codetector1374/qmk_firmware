// SPDX-License-Identifier: GPL-3.0-or-later
#include "quantum.h"
#include "pointing_device.h"
#include "atomic_util.h"
#include "trackpoint.h"

#define TRACKPOINT_ADDRESS 0x30u
#define TRACKPOINT_I2C_ERRORS (I2C_SR_BUSERR | I2C_SR_ARBLOS | I2C_SR_TOUTF)

static uint8_t packet[3];
static uint8_t packet_length;
static bool receiving;
static volatile int32_t pending_x;
static volatile int32_t pending_y;

static int32_t accumulate(int32_t pending, int8_t delta) {
    int32_t sum = pending + delta;
    return sum < -32767 ? -32767 : (sum > 32767 ? 32767 : sum);
}

static void receive_byte(uint8_t byte) {
    if (!receiving) {
        return;
    }
    packet[packet_length++] = byte;
    if (packet_length == sizeof(packet)) {
        // SN8 payload is flags, signed Y, signed X; flags are not matrix buttons.
        pending_x = accumulate(pending_x, (int8_t)packet[2]);
        pending_y = accumulate(pending_y, (int8_t)packet[1]);
        packet_length = 0;
    }
}

OSAL_IRQ_HANDLER(HT32_I2C1_IRQ_VECTOR) {
    OSAL_IRQ_PROLOGUE();
    // ADRS/STO are read-to-clear: preserve every event from this snapshot.
    uint32_t status = I2C1->SR;
    if (status & TRACKPOINT_I2C_ERRORS) {
        I2C1->SR = status & (TRACKPOINT_I2C_ERRORS | I2C_SR_RXNACK);
        receiving = false;
        packet_length = 0;
        for (uint8_t count = (status & I2C_SR_RXDNE) ? ((status & I2C_SR_RXBF) ? 2 : 1) : 0; count; --count) {
            (void)I2C1->DR;
        }
    } else {
        if (status & I2C_SR_ADRS) {
            // Each addressed write begins a fresh packet stream.
            receiving = !(status & I2C_SR_TXNRX);
            packet_length = 0;
        }
        // RXBF means both data and shift registers hold bytes. Drain both
        // before processing a co-pending STOP, without rereading RC flags.
        for (uint8_t count = (status & I2C_SR_RXDNE) ? ((status & I2C_SR_RXBF) ? 2 : 1) : 0; count; --count) {
            receive_byte((uint8_t)I2C1->DR);
        }
        if ((status & I2C_SR_TXNRX) && (status & I2C_SR_TXDE) && !(status & I2C_SR_RXNACK)) {
            // The known protocol only writes; release an unexpected read safely.
            I2C1->DR = 0xff;
        }
        if (status & I2C_SR_RXNACK) {
            I2C1->SR = I2C_SR_RXNACK;
        }
        if (status & I2C_SR_STO) {
            receiving = false;
            packet_length = 0; // Never carry a partial packet across transactions.
        }
    }
    OSAL_IRQ_EPILOGUE();
}

bool pointing_device_driver_init(void) {
    CKCU->APBCCR0 |= CKCU_APBCCR0_I2C1EN;
    CKCU->APBPCSR0 &= ~(3u << 2); // I2C1 uses the undivided peripheral clock.
    // Preload open-drain lines high before selecting I2C1 AF7.
    gpio_write_pin_high(C9);
    gpio_write_pin_high(C10);
    palSetLineMode(C9, PAL_MODE_OUTPUT_OPENDRAIN | PAL_HT32_MODE_INE | PAL_HT32_MODE_PU | PAL_HT32_MODE_AF(7));
    palSetLineMode(C10, PAL_MODE_OUTPUT_OPENDRAIN | PAL_HT32_MODE_INE | PAL_HT32_MODE_PU | PAL_HT32_MODE_AF(7));
    I2C1->IER = 0;
    I2C1->CR = 0;
    I2C1->ADDR = TRACKPOINT_ADDRESS;
    I2C1->ADDMR = 0;
    packet_length = 0;
    receiving = false;
    pending_x = pending_y = 0;
    I2C1->SR = TRACKPOINT_I2C_ERRORS | I2C_SR_RXNACK;
    I2C1->CR = I2C_CR_AA | I2C_CR_I2CEN;
    (void)I2C1->SR;
    I2C1->IER = I2C_IER_ADRSIE | I2C_IER_RXDNEIE | I2C_IER_STOIE |
                I2C_IER_BUSERRIE | I2C_IER_RXNACKIE | I2C_IER_TXDEIE;
    nvicEnableVector(I2C1_IRQn, CORTEX_MAX_KERNEL_PRIORITY);

    // Q10 is active low. Enable power only after the receiver is ready.
    gpio_write_pin_low(A10);
    palSetLineMode(A10, PAL_MODE_OUTPUT_PUSHPULL | PAL_HT32_MODE_AF(1));
    return true;
}

void trackpoint_clear_motion(void) {
    ATOMIC_BLOCK_RESTORESTATE {
        pending_x = 0;
        pending_y = 0;
    }
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t report) {
    ATOMIC_BLOCK_RESTORESTATE {
        report.x = CONSTRAIN_HID_XY(pending_x);
        report.y = CONSTRAIN_HID_XY(pending_y);
        pending_x -= report.x;
        pending_y -= report.y;
    }
    return report;
}
