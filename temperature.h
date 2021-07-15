#ifndef TEMPERATURE_H
#define	TEMPERATURE_H

#include <XC.h>
#include <stdbool.h>

#include "spi.h"

#define DIODE_CONFIG_WORD 0xE4C00000

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

