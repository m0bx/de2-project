/*
 * si4703.c - FINALNI OPRAVENA VERZE (AN230 Compliant)
 */

#include "si4703.h"
#include "twi.h"
#include "gpio.h"
#include <util/delay.h>
#include "uart.h"

// Interní buffery
static uint8_t si4703_regs[32]; 
static uint16_t shadow_regs[16];
static volatile uint8_t *g_rst_port;
static uint8_t g_rst_pin;

// Definice SDA pinu pro ATmega328P (Arduino Uno) - PC4
#define SDA_PORT PORTC
#define SDA_DDR  DDRC
#define SDA_PIN  4

// --- Pomocné funkce ---

static void read_registers(void) {
    twi_start();
    twi_write((SI4703_ADDR << 1) | TWI_READ);
    // Si4703 vrací registry v pořadí: 0A, 0B, 0C, 0D, 0E, 0F, 00, 01, 02...
    for (uint8_t i = 0; i < 32; i++) {
        si4703_regs[i] = twi_read((i == 31) ? TWI_NACK : TWI_ACK);
    }
    twi_stop();
}

static void write_registers(void) {
    twi_start();
    twi_write((SI4703_ADDR << 1) | TWI_WRITE);
    // Zápis probíhá vždy sekvenčně od registru 0x02 po 0x07
    for (int reg = 0x02; reg <= 0x07; reg++) {
        uint16_t val = shadow_regs[reg];
        twi_write(val >> 8);   
        twi_write(val & 0xFF); 
    }
    twi_stop();
}

// --- Implementace ---

void si4703_init(volatile uint8_t *rst_port, volatile uint8_t *rst_ddr, uint8_t rst_pin) {
    g_rst_port = rst_port;
    g_rst_pin = rst_pin;

    // 1. HARDWARE RESET & I2C MODE SELECTION
    // Viz AN230 Obrázek 3 a sekce 2.1.1: Pro I2C mód musí být SDIO (SDA) LOW
    // během náběžné hrany RST.
    
    // Vypnout TWI, abychom mohli hýbat s piny
    TWCR &= ~(1 << TWEN);

    gpio_mode_output(rst_ddr, rst_pin);
    gpio_write_low(rst_port, rst_pin); // RST = 0
    
    // Vnutit SDA = 0
    SDA_DDR |= (1 << SDA_PIN);
    SDA_PORT &= ~(1 << SDA_PIN);
    
    _delay_ms(10);

    // RST = 1 (Start čipu, načte si I2C mód)
    gpio_write_high(rst_port, rst_pin);
    
    _delay_ms(10);

    // Uvolnit SDA
    SDA_DDR &= ~(1 << SDA_PIN); 
    SDA_PORT |= (1 << SDA_PIN);

    // Zapnout TWI
    twi_init();

    uart_puts("DEBUG: Reset HW hotov.\r\n");

    // 2. INICIALIZACE PODLE TABULKY 3 (AN230)
    
    // Nejprve načteme aktuální stav (abychom měli čisté shadow registry)
    // Poznámka: Po resetu jsou registry vynulované, ale pro jistotu.
    // shadow_regs[0..15] jsou nyní 0x0000.

    // KROK 1: Zapnout OSCILÁTOR (XOSCEN) v registru 07h
    // Registry 02h-06h necháme na 0 (hlavně 02h ENABLE musí být 0!)

    // datasheet page 29 "Bits 13:0 of register 07h (TEST2)
    // must be preserved as 0x0100 while in powerdown and
    //  as 0x3C04 while in powerup"

    shadow_regs[0x07] = 0xBC04; // Bit 15 XOSCEN = 1, BIT 14 AHIZEN = 0 | 0x3C04
    
    write_registers(); // Zapíše reg 02=0000 ... reg 07=8100
    
    uart_puts("DEBUG: Krystal zapnut. Cekam 500ms...\r\n");
    
    // KROK 2: Počkat na stabilizaci krystalu
    _delay_ms(500); 

    // KROK 3: Zapnout RÁDIO (ENABLE) v registru 02h
    // Současně nastavíme konfiguraci pro Evropu
    
    shadow_regs[0x02] = 0xC001; // 0x02: DMUTE=1 (zapnout zvuk), ENABLE=1
    write_registers(); // Zapíše Enable a konfiguraci
    _delay_ms(1000);
    // Nastavení pro Evropu (AN230 Table 12 & 13) [cite: 523, 531]
    // Reg 0x05: 
    // BAND (bity 7:6)  = 00 (87.5-108 MHz)
    // SPACE (bity 5:4) = 01 (100 kHz - Evropa/ČR) - DŮLEŽITÉ!
    // VOLUME (bity 3:0)= 15 (Max)
    // Výsledek: 0000 0000 0001 1111 = 0x001F
    shadow_regs[0x05] = 0x001F; 
    write_registers(); // Zapíše Enable a konfiguraci
    _delay_ms(1000);
    // Reg 0x04: RDS=1, DE=1 (50us pro Evropu) [cite: 538]
    shadow_regs[0x04] = 0x1800; 

    write_registers(); // Zapíše Enable a konfiguraci
    
    _delay_ms(200); // Čas na powerup rádia
    
    shadow_regs[0x07] = 0xBC04; // Bit 15 = XOSCEN
    
    write_registers(); // Zapíše reg 02=0000 ... reg 07=8100
    
    uart_puts("DEBUG: Krystal zapnut. Cekam 500ms...\r\n");

    _delay_ms(1000); 

    uart_puts("DEBUG: Radio Enabled. Init OK.\r\n");
}

void si4703_set_volume(uint8_t volume) {
    if (volume > 15) volume = 15;
    shadow_regs[0x05] &= 0xFFF0; 
    shadow_regs[0x05] |= volume;
    write_registers();
}

void si4703_set_freq(uint16_t freq) {
    if (freq < 8750) freq = 8750;
    if (freq > 10800) freq = 10800;
    
    // Výpočet kanálu: (Frekvence - 87.5) / 0.1 MHz (pro 100kHz spacing)
    // Pokud používáme 100kHz spacing (nastaveno v init), je to správně.
    uint16_t channel = (freq - 8750) / 10;

    shadow_regs[0x03] &= 0xFE00; 
    shadow_regs[0x03] |= (1 << 15) | channel; // TUNE bit + Channel
    write_registers();

    // Čekání na STC (Seek/Tune Complete)
    uint16_t timeout = 0;
    while(1) {
        read_registers();
        if (si4703_regs[0] & 0x40) break; // STC bit
        _delay_ms(10);
        if (++timeout > 200) break; 
    }

    shadow_regs[0x03] &= ~(1 << 15); // Clear TUNE
    write_registers();
    
    // Čekání na shození STC
    timeout = 0;
    while(1) {
        read_registers();
        if (!(si4703_regs[0] & 0x40)) break; 
        _delay_ms(10);
        if (++timeout > 200) break;
    }
}

uint16_t si4703_seek(uint8_t direction) {
    // Nastavení podle AN230 Table 14 [cite: 592]
    
    // 1. Nastavit SKMODE = 0 (Wrap at band limits)
    shadow_regs[0x02] &= ~(1 << 10); 
    
    // 2. Nastavit směr (SEEKUP)
    if (direction == SEEK_UP) {
        shadow_regs[0x02] |= (1 << 9);
    } else {
        shadow_regs[0x02] &= ~(1 << 9);
    }

    // 3. Start SEEK
    shadow_regs[0x02] |= (1 << 8); 
    write_registers();

    // 4. Čekat na STC
    uint16_t timeout = 0;
    while(1) {
        read_registers();
        if (si4703_regs[0] & 0x40) break; 
        _delay_ms(10);
        if (++timeout > 500) { // 5s timeout
             uart_puts("ERR: Seek timeout.\r\n");
             break; 
        }
    }

    // 5. Vypnout SEEK bit
    shadow_regs[0x02] &= ~(1 << 8); 
    write_registers();
    
    // 6. Čekat na shození STC
    timeout = 0;
    while(1) {
        read_registers();
        if (!(si4703_regs[0] & 0x40)) break; 
        _delay_ms(10);
        if (++timeout > 200) break;
    }

    return si4703_get_freq();
}

uint16_t si4703_get_freq(void) {
    read_registers();
    // Čteme kanál z registru 0B (READCHAN)
    uint16_t channel = ((si4703_regs[2] & 0x03) << 8) | si4703_regs[3];
    // Přepočet zpět: Freq = (Channel * 0.1) + 87.5
    return (channel * 10) + 8750;
}

uint8_t si4703_get_rssi(void) {
    read_registers();
    return si4703_regs[1]; // RSSI je v dolním bytu registru 0A
}

void si4703_update_rds(RdsInfo *rdsInfo) {
    read_registers();
    if (si4703_regs[0] & 0x80) { // RDSR Ready bit
        uint16_t blockB = (si4703_regs[6] << 8) | si4703_regs[7];
        uint16_t blockD = (si4703_regs[10] << 8) | si4703_regs[11];
        uint8_t groupType = (blockB & 0xF800) >> 11;
        
        // Group 0A nebo 0B obsahuje název stanice (PS)
        if (groupType == 0 || groupType == 1) {
            uint8_t textOffset = (blockB & 0x03) * 2; 
            char char1 = (blockD >> 8) & 0xFF;
            char char2 = (blockD) & 0xFF;

            // Filtrace netisknutelných znaků
            if (char1 >= 32 && char1 <= 126) rdsInfo->stationName[textOffset] = char1;
            if (char2 >= 32 && char2 <= 126) rdsInfo->stationName[textOffset+1] = char2;
            
            rdsInfo->stationName[8] = '\0';
            rdsInfo->ready = 1; 
        }
    }
}