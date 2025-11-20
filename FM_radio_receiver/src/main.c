/* src/main.c */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "gpio.h"
#include "twi.h"
#include "oled.h"
#include "uart.h"
#include "timer.h"
#include "si4703.h"

// Definice pinů (stejné jako předtím)
#define BTN_PORT      PIND
#define BTN_DDR       DDRD
#define BTN_UP_PIN    PD2
#define BTN_DOWN_PIN  PD3
#define BTN_RST_PIN   PD4
#define BTN_MUTE_PIN  PD5
#define RADIO_RST_PORT PORTC
#define RADIO_RST_DDR  DDRC
#define RADIO_RST_PIN  PC0

// Globální proměnné
volatile uint8_t update_display_flag = 0;
uint16_t current_freq = 9500; 
uint8_t current_vol = 10;
uint8_t is_muted = 0;
RdsInfo rdsData; 

void clear_rds_buffer(void) {
    for (int i = 0; i < 9; i++) rdsData.stationName[i] = (i == 8) ? '\0' : ' ';
    rdsData.ready = 0;
}

void draw_display(void) {
    char buffer[32];
    oled_clrscr();
    oled_gotoxy(0, 0);
    oled_charMode(DOUBLESIZE);
    sprintf(buffer, "%d.%d MHz", current_freq / 100, (current_freq % 100) / 10);
    oled_puts(buffer);
    oled_charMode(NORMALSIZE);
    oled_gotoxy(0, 3);
    if (is_muted) {
        oled_puts("Status: ZTLUMENO");
    } else {
        sprintf(buffer, "Vol: %d  RSSI: %d", current_vol, si4703_get_rssi());
        oled_puts(buffer);
    }
    oled_gotoxy(0, 5);
    oled_puts("Stanice:");
    oled_gotoxy(0, 6);
    oled_puts(rdsData.stationName);
    oled_display();
}

ISR(TIMER1_OVF_vect) {
    update_display_flag = 1;
}

int main(void) {
    // 1. UART Init
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    
    // --- DŮLEŽITÁ ZMĚNA: Povolit přerušení HNED ZDE ---
    // Aby se debug zprávy vypisovaly i když se program později zasekne
    sei(); 
    
    uart_puts("--- START SYSTEMU ---\r\n");

    // 2. I2C a OLED
    twi_init();
    oled_init(OLED_DISP_ON);
    oled_clrscr();
    oled_puts("Startuji...");
    oled_display();
    
    uart_puts("DEBUG: OLED OK. Jdu na radio...\r\n");

    // 3. Tlačítka
    gpio_mode_input_pullup(&BTN_DDR, BTN_UP_PIN);
    gpio_mode_input_pullup(&BTN_DDR, BTN_DOWN_PIN);
    gpio_mode_input_pullup(&BTN_DDR, BTN_RST_PIN);
    gpio_mode_input_pullup(&BTN_DDR, BTN_MUTE_PIN);

    // 4. Si4703 Rádio
    // Pokud se to zasekne, uvidíte v konzoli poslední zprávu "DEBUG: OLED OK..."
    si4703_init(&RADIO_RST_PORT, &RADIO_RST_DDR, RADIO_RST_PIN);
    
    uart_puts("DEBUG: Radio Init OK.\r\n");
    
    si4703_set_volume(current_vol);
    si4703_set_freq(current_freq);
    clear_rds_buffer();

    // 5. Timer
    tim1_ovf_262ms(); 
    tim1_ovf_enable();

    uart_puts("DEBUG: Smycka bezi.\r\n");

    while (1) {
        if (gpio_read(&BTN_PORT, BTN_UP_PIN) == 0) {
            _delay_ms(50);
            if (gpio_read(&BTN_PORT, BTN_UP_PIN) == 0) {
                uart_puts("Seek UP\r\n");
                clear_rds_buffer();
                uint16_t ret = si4703_seek(1);
                if (ret) current_freq = ret;
                else { current_freq = 8750; si4703_set_freq(current_freq); }
                while(gpio_read(&BTN_PORT, BTN_UP_PIN) == 0);
            }
        }
        if (gpio_read(&BTN_PORT, BTN_DOWN_PIN) == 0) {
            _delay_ms(50);
            if (gpio_read(&BTN_PORT, BTN_DOWN_PIN) == 0) {
                uart_puts("Seek DOWN\r\n");
                clear_rds_buffer();
                uint16_t ret = si4703_seek(0);
                if (ret) current_freq = ret;
                else { current_freq = 10800; si4703_set_freq(current_freq); }
                while(gpio_read(&BTN_PORT, BTN_DOWN_PIN) == 0);
            }
        }
        if (gpio_read(&BTN_PORT, BTN_RST_PIN) == 0) {
             _delay_ms(50);
             if (gpio_read(&BTN_PORT, BTN_RST_PIN) == 0) {
                 si4703_init(&RADIO_RST_PORT, &RADIO_RST_DDR, RADIO_RST_PIN);
                 current_freq = 9500; current_vol = 10; is_muted = 0;
                 si4703_set_freq(current_freq); si4703_set_volume(current_vol);
                 while(gpio_read(&BTN_PORT, BTN_RST_PIN) == 0);
             }
        }
        if (gpio_read(&BTN_PORT, BTN_MUTE_PIN) == 0) {
            _delay_ms(50);
            if (gpio_read(&BTN_PORT, BTN_MUTE_PIN) == 0) {
                is_muted = !is_muted;
                si4703_set_volume(is_muted ? 0 : 10);
                while(gpio_read(&BTN_PORT, BTN_MUTE_PIN) == 0);
            }
        }

        si4703_update_rds(&rdsData);

        if (update_display_flag) {
            update_display_flag = 0;
            draw_display();
        }
    }
    return 0;
}
