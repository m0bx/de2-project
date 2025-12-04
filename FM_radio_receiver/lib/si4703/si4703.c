
 /**
  * @file si4703.c
  * @defgroup si4703 Si4703 Library <si4703.c>
  * @code #include <si4703.h> @endcode
  * 
  * @brief Si4703 FM radio module library implementation based on Tomas Fryza's TWI library
  */

#include "si4703.h"
#include "twi.h"
#include "gpio.h"
#include <util/delay.h>
#include "uart.h"

// buffer definitions
static uint8_t si4703_regs[32];         // I2C receive buffer 
static uint16_t shadow_regs[16];        // Si4703 register state-keeping variable with correct indices
static volatile uint8_t *g_rst_port;    // pointer to port containing Si4703 RST pin
static uint8_t g_rst_pin;               // Si4703 RST pin variable

// Definice SDA pinu pro ATmega328P (Arduino Uno) - PC4
#define SDA_PORT PORTC
#define SDA_DDR  DDRC
#define SDA_PIN  4

/*
 * Function for writing the shadow registers onto the Si4703 chip
 */
static void read_registers(void) {
    twi_start();
    twi_write((SI4703_ADDR << 1) | TWI_READ);
    // storing the read message into the buffer
    for (uint8_t i = 0; i < 32; i++) {
        si4703_regs[i] = twi_read((i == 31) ? TWI_NACK : TWI_ACK);
    }
    twi_stop();
}

/*
 * Function for reading the registers from the Si4703 chip and updating the shadow registers
 */
static void write_registers(void) {
    twi_start();
    twi_write((SI4703_ADDR << 1) | TWI_WRITE);

    // registers 0x02 to 0x07 are writable
    for (int reg = 0x02; reg <= 0x07; reg++) {
        uint16_t val = shadow_regs[reg];
        twi_write(val >> 8);   
        twi_write(val & 0xFF); 
    }
    twi_stop();
}

/*
 * Init routine according to AN230 programming manual table 3 (page 12)    
 */
void si4703_init(volatile uint8_t *rst_port, volatile uint8_t *rst_ddr, uint8_t rst_pin) {
    g_rst_port = rst_port;
    g_rst_pin = rst_pin;
    
    // briefly disable I2C for pin control
    TWCR &= ~(1 << TWEN);

    gpio_mode_output(rst_ddr, rst_pin);
    gpio_write_low(rst_port, rst_pin); // RST = 0

    // for 2 wire mode, SDA has to be low during RST rising edge
    // force SDA to low 
    SDA_DDR |= (1 << SDA_PIN);
    SDA_PORT &= ~(1 << SDA_PIN);
    
    _delay_ms(10);

    // RST = 1 (chip start, Two-wire I2C mode is loaded)
    gpio_write_high(rst_port, rst_pin);
    
    _delay_ms(10);

    // re-enable I2C
    SDA_DDR &= ~(1 << SDA_PIN); 
    SDA_PORT |= (1 << SDA_PIN);
    twi_init();

    uart_puts("DEBUG: HW reset finished.\r\n");

    /*
     * note datasheet page 29: "Bits 13:0 of register 07h (TEST2)
     * must be preserved as 0x0100 while in powerdown and
     * as 0x3C04 while in powerup"
    */


    /*
     * set TEST1 (0x07) register to enable external crystal
     * XOSCEN[15] bit set to 1 (enable crystal)
     * AHIZEN[14] bit set to 0 (disable Hi-Z audio output)
     * Reserved[13:0] set to 0x0100 to comply with datasheet in powerdown state
     */ 
    shadow_regs[0x07] = 0x8100; // (Bit 15 XOSCEN = 1, BIT 14 AHIZEN = 0) | 0x0100
    
    write_registers();
    
    uart_puts("DEBUG: Crystal enabled. Waiting for 500 ms...\r\n");
    
    _delay_ms(500); // crystal stabilization delay 


    /*
     * set ENABLE[0] bit to 1 and DISABLE[0] bit to 0 in the POWERCFG (0x02) register
     * to put the device into powerup state
     */ 
    shadow_regs[0x02] = 0xC001;
    write_registers();

    _delay_ms(120); // wait for device to powerup

    /*
     * change the TEST1 (0x07) register to the powered-up form
     * XOSCEN[15] bit set to 1 (enable crystal)
     * AHIZEN[14] bit set to 0 (disable Hi-Z audio output)
     * Reserved[13:0] set to 0x3C04 to comply with datasheet in powerup state
     */ 
    shadow_regs[0x07] = 0xBC04;


    /*
     * Setting up the SYSCONFIG2 (0x05) register for the EU region
     * BAND[7:6] bits set to 0b00 (87.5-108 MHz)
     * SPACE[5:4] bits set to 0b01 (100kHz spacing)
     * VOLUME[3:0] set to 15 (Max)
     */ 
    shadow_regs[0x05] = 0x001F; 
    write_registers();

    /*
     * Setting up the SYSCONFIG1 (0x04) register for the EU region
     * RDS[12] bit set to 1 (enable interrupt)
     * DE[11] bit set to 1 (50 us de-emphasis used in EU)
     */ 
    shadow_regs[0x04] = 0x1800; 

    write_registers();
            
    uart_puts("DEBUG: Radio Enabled. Init OK.\r\n");
}

/*
 * Function for setting volume
 *
 * args:
 * volume - int value from 0 to 15
 */
void si4703_set_volume(uint8_t volume) {
    if (volume > 15) volume = 15;
    shadow_regs[0x05] &= 0xFFF0; 
    shadow_regs[0x05] |= volume;
    write_registers();
}

/*
 * Function for setting frequency
 *
 * args:
 * freq - frequency in MHz multiplied by 100 (eg. 87.5 MHz => 8750)
 */
void si4703_set_freq(uint16_t freq) {
    if (freq < 8750) freq = 8750;
    if (freq > 10800) freq = 10800;
    
    uint16_t channel = (freq - 8750) / 10;

    shadow_regs[0x03] &= 0xFE00; 
    shadow_regs[0x03] |= (1 << 15) | channel; // TUNE bit + Channel
    write_registers();

    // wait for STC
    uint16_t timeout = 0;
    while(1) {
        read_registers();
        if (si4703_regs[0] & 0x40) break; // STC bit
        _delay_ms(10);
        if (++timeout > 200) break; 
    }

    shadow_regs[0x03] &= ~(1 << 15); // Clear TUNE
    write_registers();
    
    // wait for STC
    timeout = 0;
    while(1) {
        read_registers();
        if (!(si4703_regs[0] & 0x40)) break; 
        _delay_ms(10);
        if (++timeout > 200) break;
    }
}

/*
 * Function for finding the next available station
 *
 * args:
 * direction - SEEKUP or SEEKDOWN depending on the chosen direction
 */
uint16_t si4703_seek(uint8_t direction) {
    // Nastavení podle AN230 Table 14 [cite: 592]
    

    /*
     * Setting up the POWERCFG (0x02) register for seeking
     * SKMODE[10] bit set to 0 (wrap at band limit and continue seeking)
     * SEEKUP[9] bit set to either 0 or 1 based on selected direction
     * SEEK[8] bit set to 1 to enable seeking
     */ 

    shadow_regs[0x02] &= ~(1 << 10); 
    
    if (direction == SEEK_UP) {
        shadow_regs[0x02] |= (1 << 9);
    } else {
        shadow_regs[0x02] &= ~(1 << 9);
    }

    // start seeking
    shadow_regs[0x02] |= (1 << 8); 
    write_registers();

    // wait for STC (seek/tune complete bit)
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

    // disable seeking
    shadow_regs[0x02] &= ~(1 << 8); 
    write_registers();

    // check if STC bit is back to 0
    timeout = 0;
    while(1) {
        read_registers();
        if (!(si4703_regs[0] & 0x40)) break; 
        _delay_ms(10);
        if (++timeout > 200) break;
    }

    return si4703_get_freq();
}


/*
 * Function for returning the currently set frequency
 *
 * returns:
 * Frequency in MHz
 */
uint16_t si4703_get_freq(void) {
    read_registers();
    // Čteme kanál z registru 0B (READCHAN)
    uint16_t channel = ((si4703_regs[2] & 0x03) << 8) | si4703_regs[3];
    // Přepočet zpět: Freq = (Channel * 0.1) + 87.5
    return (channel * 10) + 8750;
}

/*
 * Function for returning the RSSI (signal strenght)
 *
 * returns:
 * RSSI (0-127)
 */
uint8_t si4703_get_rssi(void) {
    read_registers();
    return si4703_regs[1]; // RSSI is stored in the bottom byte of the 0x0A register
}

/*
 * Function for returning the RSSI (signal strenght)
 *
 * args:
 * pointer to RdsInfo structure
 */
void si4703_update_rds(RdsInfo *rdsInfo) {
    read_registers();
    if (si4703_regs[0] & 0x80) { // RDSR Ready bit
        uint16_t blockB = (si4703_regs[6] << 8) | si4703_regs[7];
        uint16_t blockD = (si4703_regs[10] << 8) | si4703_regs[11];
        uint8_t groupType = (blockB & 0xF800) >> 11;
        
        // Group 0A or 0B contains the station name (PS)
        if (groupType == 0 || groupType == 1) {
            uint8_t textOffset = (blockB & 0x03) * 2; 
            char char1 = (blockD >> 8) & 0xFF;
            char char2 = (blockD) & 0xFF;

            // Filtering unprintable characters
            if (char1 >= 32 && char1 <= 126) rdsInfo->stationName[textOffset] = char1;
            if (char2 >= 32 && char2 <= 126) rdsInfo->stationName[textOffset+1] = char2;
            
            rdsInfo->stationName[8] = '\0';
            rdsInfo->ready = 1; 
        }
    }
}