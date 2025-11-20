/*
 * si4703.h
 * Ovladač pro FM tuner Si4703 s podporou RDS a audio výstupu.
 * Platforma: AVR/ATmega328P (bez Arduino frameworku)
 */

#ifndef SI4703_H
#define SI4703_H

#include <stdint.h>

// I2C Adresa Si4703
#define SI4703_ADDR 0x10

// Definice pro Evropu (87.5 - 108.0 MHz)
#define FREQ_MIN 8750
#define FREQ_MAX 10800

// Směry ladění
#define SEEK_DOWN 0
#define SEEK_UP   1

// Struktura pro uchování RDS dat (Název stanice)
typedef struct {
    char stationName[9]; // 8 znaků + nulový terminátor
    uint8_t ready;       // Příznak, že máme nová data
} RdsInfo;

/**
 * @brief Inicializace modulu Si4703, zapnutí audia a RDS.
 * @param rst_port Port registru pro RESET pin (např. &PORTC)
 * @param rst_ddr  DDR registr pro RESET pin (např. &DDRC)
 * @param rst_pin  Číslo pinu RESET (např. PC0)
 */
void si4703_init(volatile uint8_t *rst_port, volatile uint8_t *rst_ddr, uint8_t rst_pin);

/**
 * @brief Nastaví hlasitost výstupu (sluchátek).
 * @param volume Úroveň 0 (ticho) až 15 (max).
 */
void si4703_set_volume(uint8_t volume);

/**
 * @brief Naladí konkrétní frekvenci.
 * @param freq Frekvence v 10kHz (např. 9480 pro 94.8 MHz).
 */
void si4703_set_freq(uint16_t freq);

/**
 * @brief Vyhledá nejbližší silnou stanici.
 * @param direction SEEK_UP nebo SEEK_DOWN.
 * @return Naladěná frekvence.
 */
uint16_t si4703_seek(uint8_t direction);

/**
 * @brief Přečte aktuální frekvenci z čipu.
 */
uint16_t si4703_get_freq(void);

/**
 * @brief Získá sílu signálu (RSSI).
 * @return Hodnota 0-127.
 */
uint8_t si4703_get_rssi(void);

/**
 * @brief Hlavní funkce pro RDS. Musí se volat v hlavní smyčce.
 * Kontroluje, zda přišla data, a pokud ano, aktualizuje název stanice.
 * @param rdsInfo Ukazatel na strukturu, kam se uloží název.
 */
void si4703_update_rds(RdsInfo *rdsInfo);

#endif