#ifndef TEMPERATURE_H
#define	TEMPERATURE_H

#include <XC.h>
#include <stdbool.h>

#include "spi.h"

bool init_temp_ref_diodes(void);

void cs_write_1(uint8_t state);
void cs_write_2(uint8_t state);
void cs_write_3(uint8_t state);


bool config_channel(uint8_t channel_num, uint32_t config_word, 
                    void *cs_write(uint8_t));

#endif	/* TEMPERATURE_H */

