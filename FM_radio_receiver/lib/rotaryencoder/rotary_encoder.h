#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include <stdint.h>
#include "gpio.h"

/*
 * pin mapping for ATmega328P:
 *
 *   CLK -> PD2
 *   DT  -> PD3
 *   SW  -> PD4 (push button, active-low)
 */

// --- CLK (rotary A) ---
#define ENC_CLK_DDR      DDRD
#define ENC_CLK_PINREG   PIND
#define ENC_CLK_PIN      2   // PD2

// --- DT (rotary B) ---
#define ENC_DT_DDR       DDRD
#define ENC_DT_PINREG    PIND
#define ENC_DT_PIN       3   // PD3

// --- SW (push button) ---
#define ENC_SW_DDR       DDRD
#define ENC_SW_PINREG    PIND
#define ENC_SW_PIN       4   // PD4


/**
 * Sets CLK, DT, SW as inputs with pull-ups (using gpio library)
 * Initializes internal state machine
 */
void encoder_init(void);


/**
 * Call this often (e.g. every 1–2 ms) from main loop or timer ISR.
 * It reads CLK/DT, decodes movement, and updates internal position.
 */
void encoder_update(void);


/**
 * Get current encoder position (signed).
 */
int16_t encoder_get_position(void);


/**
 * Set encoder position (e.g. to zero).
 */
void encoder_set_position(int16_t value);


/**
 * Get position delta since last call.
 * Internally stored delta is reset to zero.
 */
int8_t encoder_get_delta(void);


/**
 * return 1 if button is pressed, 0 if released.
 * @note   No debouncing is performed here.
 */
uint8_t encoder_button_pressed(void);

#endif
