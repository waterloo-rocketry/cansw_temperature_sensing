#ifndef TEMPERATURE_H
#define	TEMPERATURE_H

#include <XC.h>
#include <stdbool.h>

#include "spi.h"

// 5msb are sensor type. 0b11100 for diode
// next 3 bits are config. 0b100 for single ended, 2 current levels, no averaging
// next 2 are excitation current. 0b11 for 80uA
// the rest are for ideality factor, which is currently unknown
#define DIODE_CONFIG_WORD 0xE4C00000

// thermocouple
// 5msb are TC type. 0b00010 for K
// next 5 bits are cold junction pointer, set by config function
// next 4 bits are sensor config. 0b1000 for single ended, no open circuit check (faster)
// 0b0001 0000 0010 00 ...
#define TC_CONFIG_WORD 0x10200000

void cs_write_1(uint8_t state);
void cs_write_2(uint8_t state);
void cs_write_3(uint8_t state);

bool config_channel(uint8_t channel_num, uint32_t config_word,
                    void (*cs_write)(uint8_t));

// Sets up the temperature reference diodes on channels 6, 10, 16, 20
bool config_temp_ref_diodes(void (*cs_write)(uint8_t));

bool config_tc(uint8_t channel_num, void (*cs_write)(uint8_t));

bool start_conversion(uint8_t channel_num, void (*cs_write)(uint8_t));

uint32_t get_conversion(uint8_t channel_num, void (*cs_write)(uint8_t));

uint8_t get_status(void (*cs_write)(uint8_t));

#endif	/* TEMPERATURE_H */

