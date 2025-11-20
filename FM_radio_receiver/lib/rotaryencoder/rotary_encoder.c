// inspired by https://github.com/mhx/librotaryencoder/
#include "rotary_encoder.h"

static int16_t encoder_position = 0;
static int8_t  encoder_delta    = 0;   // accumulated since last get_delta
static uint8_t last_state       = 0;   // previous 2-bit state: [CLK:bit1 | DT:bit0]


// States: 0b00, 0b01, 0b11, 0b10  (0,1,3,2)
// Each [old][new] -> delta {-1, 0, +1}
static const int8_t enc_transition_table[4][4] = {
    // new:  00   01   10   11   (in decimal: 0,1,2,3)
    /* old=00 */ {  0,  +1, -1,  0 },
    /* old=01 */ { -1,  0,   0, +1 },
    /* old=10 */ { +1,  0,   0, -1 },  
    /* old=11 */ {  0,  -1, +1,  0 }
};


void encoder_init(void)
{
    // Configure inputs with pull-ups using gpio library
    gpio_mode_input_pullup(&ENC_CLK_DDR, ENC_CLK_PIN);
    gpio_mode_input_pullup(&ENC_DT_DDR,  ENC_DT_PIN);
    gpio_mode_input_pullup(&ENC_SW_DDR,  ENC_SW_PIN);

    // Read initial state of CLK and DT
    uint8_t clk = gpio_read(&ENC_CLK_PINREG, ENC_CLK_PIN);
    uint8_t dt  = gpio_read(&ENC_DT_PINREG,  ENC_DT_PIN);

    last_state = (clk << 1) | dt;
    encoder_position = 0;
    encoder_delta    = 0;
}


void encoder_update(void)
{
    // Read current state of CLK (A) and DT (B)
    uint8_t clk = gpio_read(&ENC_CLK_PINREG, ENC_CLK_PIN);
    uint8_t dt  = gpio_read(&ENC_DT_PINREG,  ENC_DT_PIN);

    uint8_t new_state = (clk << 1) | dt;

    if (new_state > 3) {
        // Should never happen, but guard anyway
        last_state = new_state & 0x03;
        return;
    }

    int8_t step = enc_transition_table[last_state][new_state];

    if (step != 0) {
        encoder_position += step;

        // Keep delta within int8_t range (simple saturation)
        int16_t tmp = encoder_delta + step;
        if (tmp > 127) {
            encoder_delta = 127;
        } else if (tmp < -128) {
            encoder_delta = -128;
        } else {
            encoder_delta = (int8_t)tmp;
        }
    }

    last_state = new_state;
}


int16_t encoder_get_position(void)
{
    return encoder_position;
}


void encoder_set_position(int16_t value)
{
    encoder_position = value;
    encoder_delta    = 0;
}


int8_t encoder_get_delta(void)
{
    int8_t d = encoder_delta;
    encoder_delta = 0;
    return d;
}


uint8_t encoder_button_pressed(void)
{
    // Button is active-low: pressed when pin reads 0
    uint8_t val = gpio_read(&ENC_SW_PINREG, ENC_SW_PIN);
    return (val == 0) ? 1 : 0;
}
