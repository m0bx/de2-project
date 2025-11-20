// inspired by https://github.com/mhx/librotaryencoder/
#include "rotary_encoder.h"
#ifdef ENCODER_DEBUG
#include <uart.h>
#endif

static int16_t encoder_position = 0;
static int8_t  encoder_delta    = 0;
static uint8_t last_state       = 0;


static const int8_t enc_transition_table[4][4] = {
    // new: 00, 01, 10, 11 (1,2,3,4)
    /* old=00 */ {  0,  +1, -1,  0 },
    /* old=01 */ { -1,  0,   0, +1 },
    /* old=10 */ { +1,  0,   0, -1 },  
    /* old=11 */ {  0,  -1, +1,  0 }
};


void encoder_init(void)
{
    gpio_mode_input_pullup(&ENC_CLK_DDR, ENC_CLK_PIN);
    gpio_mode_input_pullup(&ENC_DT_DDR,  ENC_DT_PIN);
    gpio_mode_input_pullup(&ENC_SW_DDR,  ENC_SW_PIN);

    // read initial state
    uint8_t clk_raw = gpio_read(&ENC_CLK_PINREG, ENC_CLK_PIN);
    uint8_t dt_raw  = gpio_read(&ENC_DT_PINREG,  ENC_DT_PIN);

    uint8_t clk = clk_raw ? 1 : 0;
    uint8_t dt  = dt_raw  ? 1 : 0;

    last_state = (clk << 1) | dt;
    encoder_position = 0;
    encoder_delta    = 0;
}

void encoder_update(void)
{
    uint8_t clk = (PIND & (1 << ENC_CLK_PIN)) ? 1 : 0;
    uint8_t dt  = (PIND & (1 << ENC_DT_PIN))  ? 1 : 0;

    // Calculate state
    uint8_t new_state = (clk << 1) | dt;

    int8_t diff = enc_transition_table[last_state][new_state];
    encoder_delta += diff;
    encoder_position += diff;
    
    last_state = new_state;

    // DO NOT PRINT HERE USING UART.
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
    // Button is active-low
    uint8_t val = gpio_read(&ENC_SW_PINREG, ENC_SW_PIN);
    return (val == 0) ? 1 : 0;
}
