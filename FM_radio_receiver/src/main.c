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
#include <string.h>
#include <util/atomic.h>
#include "rotary_encoder.h"

// Definice pinů (stejné jako předtím)
#define BTN_PORT      PIND
#define BTN_DDR       DDRD
#define BTN_UP_PIN    PD7
#define BTN_DOWN_PIN  PD6
// #define BTN_RST_PIN   PD4
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
    
    // --- Statické proměnné pro uchování "minulého" stavu ---
    // Inicializovány na nesmyslné hodnoty, aby se displej vykreslil poprvé určitě.
    static uint16_t last_freq = 0;
    static uint8_t last_vol = 255;
    static uint8_t last_mute = 255;
    static int last_rssi = -1;
    static char last_rds[10] = ""; // Buffer pro minule zobrazené RDS

    // Získáme aktuální RSSI (signál)
    int current_rssi = si4703_get_rssi();

    // ---------------------------------------------------------
    // 1. Sekce Frekvence (Vykreslit jen při změně)
    // ---------------------------------------------------------
    if (current_freq != last_freq) {
        oled_gotoxy(0, 0);
        oled_charMode(DOUBLESIZE);
        sprintf(buffer, "%d.%d MHz", current_freq / 100, (current_freq % 100) / 10);
        oled_puts(buffer);
        
        last_freq = current_freq; // Uložit novou hodnotu
    }

    // ---------------------------------------------------------
    // 2. Sekce Status (Hlasitost / Mute / RSSI)
    // ---------------------------------------------------------
    // Překreslíme řádek, pokud se změnilo MUTE, Hlasitost nebo výrazně RSSI
    // (u RSSI kontrolujeme změnu, aby čísla neustále neblikala, pokud je signál nestabilní)
    if (is_muted != last_mute || current_vol != last_vol || current_rssi != last_rssi) {
        
        oled_charMode(NORMALSIZE);
        oled_gotoxy(0, 3);

        if (is_muted) {
            // Používáme mezery na konci, abychom přepsali případný delší text z minula
            oled_puts("Status: MUTED       "); 
        } else {
            // Formátování s pevným počtem znaků pro přepsání starého textu
            sprintf(buffer, "Vol:%-2d RSSI:%-3d   ", current_vol, current_rssi);
            oled_puts(buffer);
        }

        // Uložení aktuálních stavů
        last_mute = is_muted;
        last_vol = current_vol;
        last_rssi = current_rssi;
    }

    // ---------------------------------------------------------
    // 3. Sekce RDS (Název stanice)
    // ---------------------------------------------------------
    // Porovnáme nový text se starým pomocí strcmp
    if (strcmp(rdsData.stationName, last_rds) != 0) {
        
        // Vykreslíme popisek pouze jednou (volitelné, pokud se nemění, může být v initu)
        oled_gotoxy(0, 5);
        oled_puts("Station:"); 

        oled_gotoxy(0, 6);
        // Pokud je nový text kratší než starý, musíme zbytek řádku vymazat mezerami
        // Zde to řešíme tak, že rdsData.stationName by mělo být doplněno mezerami už v clear_rds_buffer()
        oled_puts(rdsData.stationName);
        
        // Uložíme si, co jsme právě vypsali
        strcpy(last_rds, rdsData.stationName);
    }
    
    // Poznámka: Odstranili jsme oled_clrscr(), protože to způsobuje blikání.
    // Místo toho přepisujeme stará data novými.
    oled_display();
}

ISR(TIMER1_OVF_vect) {
    update_display_flag = 1;
}

ISR(TIMER0_OVF_vect) {
    encoder_update();
}



int main(void) {
    _delay_ms(500);
    // 1. UART Init
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    
    // --- DŮLEŽITÁ ZMĚNA: Povolit přerušení HNED ZDE ---
    // Aby se debug zprávy vypisovaly i když se program později zasekne

    encoder_init();

    sei(); 
    
    uart_puts("--- START SYSTEMU ---\r\n");

    // 2. I2C a OLED
    twi_init();
    oled_init(OLED_DISP_ON);
    oled_clrscr();
    oled_puts("Startuji...");
    oled_display();
    
    uart_puts("DEBUG: OLED OK. Jdu na radio...\r\n");

    _delay_ms(100);

    // 3. Tlačítka
    gpio_mode_input_pullup(&BTN_DDR, BTN_UP_PIN);
    gpio_mode_input_pullup(&BTN_DDR, BTN_DOWN_PIN);
    //gpio_mode_input_pullup(&BTN_DDR, BTN_RST_PIN);
    gpio_mode_input_pullup(&BTN_DDR, BTN_MUTE_PIN);

    // 4. Si4703 Rádio
    // Pokud se to zasekne, uvidíte v konzoli poslední zprávu "DEBUG: OLED OK..."
    si4703_init(&RADIO_RST_PORT, &RADIO_RST_DDR, RADIO_RST_PIN);
    
    uart_puts("DEBUG: Radio Init OK.\r\n");
    
    si4703_set_volume(current_vol);
    si4703_set_freq(current_freq);
    clear_rds_buffer();

    // 5. Timer
    tim1_ovf_33ms(); 
    tim1_ovf_enable();

    tim0_ovf_4ms(); 
    tim0_ovf_enable();

    uart_puts("DEBUG: Smycka bezi.\r\n");

    int8_t seek_accumulator = 0; // Tracks rotation momentum
    uint8_t btn_was_pressed = 0;

    while (1) {
        // --- ROTARY ENCODER LOGIC ---
        int8_t delta = encoder_get_delta();

        if (delta != 0) {
            // 1. Update Seek Accumulator (Momentum)
            // If turning same direction, adds up. If turning opposite, subtracts.
            if ((delta > 0 && seek_accumulator < 0) || (delta < 0 && seek_accumulator > 0)) {
                seek_accumulator = 0; // Reset momentum if direction reverses abruptly
            }
            seek_accumulator += delta;
            
            
            // 3. Normal Manual Tuning (+/- 0.1 MHz)
            // Assuming freq format: 10150 = 101.5MHz, so +/- 10 is 0.1MHz
            if (delta > 0) current_freq -= 10;
            else current_freq += 10;

            // Simple bounds checking
            if (current_freq > 10800) current_freq = 8750;
            if (current_freq < 8750) current_freq = 10800;

            si4703_set_freq(current_freq);
            

            update_display_flag = 1; // Update screen on any movement
        }

        if (encoder_button_pressed()) {
            if (!btn_was_pressed) {
                _delay_ms(20); // Debounce
                
                if (encoder_button_pressed()) {
                    uart_puts(">> RESET TRIGGERED <<\r\n");
                    
                    // Re-init and set defaults
                    si4703_init(&RADIO_RST_PORT, &RADIO_RST_DDR, RADIO_RST_PIN);
                    current_freq = 9500; 
                    current_vol = 10; 
                    is_muted = 0;
                    seek_accumulator = 0; // Clear seek memory
                    
                    si4703_set_freq(current_freq); 
                    si4703_set_volume(current_vol);
                    
                    btn_was_pressed = 1;
                    update_display_flag = 1;
                }
            }
        } else {
            btn_was_pressed = 0;
        }

        if (gpio_read(&BTN_PORT, BTN_MUTE_PIN) == 0) {
            _delay_ms(50);
            if (gpio_read(&BTN_PORT, BTN_MUTE_PIN) == 0) {
                is_muted = !is_muted;
                si4703_set_volume(is_muted ? 0 : 10);
                while(gpio_read(&BTN_PORT, BTN_MUTE_PIN) == 0);
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

        // --- RDS & DISPLAY TASKS ---
        si4703_update_rds(&rdsData);

        if (update_display_flag) {
            update_display_flag = 0;
            draw_display();
        }
    }
    return 0;
}
