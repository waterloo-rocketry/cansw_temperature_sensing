#ifndef SETUP_H
#define	SETUP_H

#include <XC.h>
#include <stdint.h>
#include <stdbool.h>

uint8_t test_data[7];

// sets up SPI module
void spi_init(void);

// transfers an arbitrary amount of data
bool spi_transfer(uint8_t* send_data, uint8_t* receive_data, 
                  uint8_t datalen, void (*cs_write)(uint8_t));

#endif	/* SETUP_H */

