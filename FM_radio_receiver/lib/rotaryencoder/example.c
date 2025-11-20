#include <avr/io.h>
#include <avr/interrupt.h>
#include "gpio.h"
#include "timer.h"
#include "rotary_encoder.h"
// example usage of the rotary encoder lib, periodical check is done using interrupts (inefficient but works)

// t0 every 1 ms
ISR(TIMER0_OVF_vect)
{
    encoder_update();
}


int main(void)
{
    // LED
    gpio_mode_output(&DDRB, 5);
    encoder_init();

    tim0_ovf_1ms();
    tim0_ovf_enable();

    sei();

    while (1) {
        // Get movement since last check
        int8_t d = encoder_get_delta();
        if (d != 0) {
            // Toggle LED on move
            gpio_toggle(&PORTB, 5);
        }

        // Reset position when button pressed
        if (encoder_button_pressed()) {
            encoder_set_position(0);
        }
    }
}
