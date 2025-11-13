#include <avr/io.h>
#include <lib/gpio/gpio.h>
#include "rotary_encoder.h"
#include "timer.h"

int main(void)
{
    // LEDs on PORTB as outputs
    gpio_mode_output(&DDRB, 5);

    encoder_init();

    while (1) {
        encoder_update();

        // toggle when moved
        int8_t d = encoder_get_delta();
        if (d != 0) {
            gpio_toggle(&PORTB, 5);
        }

        // reset position when button pressed
        if (encoder_button_pressed()) {
            encoder_set_position(0);
        }

        _delay_ms(2);  // update every ~2 ms
    }
}
