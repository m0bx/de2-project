 /**
  * @file si4703.h
  * @defgroup si4703 Si4703 Library <si4703.h>
  * @code #include <si4703.h> @endcode
  * 
  * @brief Si4703 FM radio module library based on Tomas Fryza's TWI library
  * 
  * Library for interacting with the Si4703 FM radio module.
  * This library was developed for the purposes of a DE2 class project and does
  * not offer full functionality.
  * 
  * inspired by the SparkFun Si4703 Arduino library and
  * the bare metal AVR_SI4703 library from github user eziya
  * 
  * Developed for ATmega328p/Arduino Uno R3
  * @{
  */

#ifndef SI4703_H
#define SI4703_H

#include <stdint.h>

// I2C address for Si4703
#define SI4703_ADDR 0x10

// FM radio frequency range for EU (87.5 - 108.0 MHz)
#define FREQ_MIN 8750
#define FREQ_MAX 10800

// Seeking directions
#define SEEK_DOWN 0
#define SEEK_UP   1

// struct for keeping RDS (station info) data
typedef struct {
    char stationName[9]; // 8 characters + null terminator
    uint8_t ready;       // data ready indicator
} RdsInfo;

/**
 * @brief Si4703 module initialization
 * @param rst_port module RST pin port (eg. &PORTC)
 * @param rst_ddr  module RST pin Data Direction Register (eg. &DDRC)
 * @param rst_pin  module RST pin number (eg. PC0)
 */
void si4703_init(volatile uint8_t *rst_port, volatile uint8_t *rst_ddr, uint8_t rst_pin);

/**
 * @brief Output volume setting function
 * @param volume scale from 0 (silence) to 15 (max volume)
 */
void si4703_set_volume(uint8_t volume);

/**
 * @brief Tunes the module to the chosen frequency
 * @param freq Chosen frequency in MHz multiplied by 100 (94.8 MHz => 9480).
 */
void si4703_set_freq(uint16_t freq);

/**
 * @brief Seeks the next station with a strong signal in the chosen direction
 * @param direction SEEK_UP for a higher frequency or SEEK_DOWN for a lower frequency.
 * @return Tuned frequency.
 */
uint16_t si4703_seek(uint8_t direction);

/**
 * @brief Reads the currently tuned frequency.
 */
uint16_t si4703_get_freq(void);

/**
 * @brief Returns the RSSI (signal strength) value.
 * @return Value in the range of 0-127.
 */
uint8_t si4703_get_rssi(void);

/**
 * @brief Function for handling RDS (station data)
 * @note  Has to be called in the main loop of the program.
 * @param rdsInfo Pointer to RdsInfo structure
 */
void si4703_update_rds(RdsInfo *rdsInfo);

/** @} */

#endif