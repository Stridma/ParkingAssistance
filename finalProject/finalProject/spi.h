/*
 * spi.h
 *
 * Created: 03/11/2017 08:21:53
 *  Author: LuisEnrique
 */ 

#include "sam.h"

#ifndef SPI_H_
#define SPI_H_

#define SLAVE_SD_CARD	0
#define SLAVE_CAN_0		1 // should be 1 for some files
#define SLAVE_CAN_1		2	 // should be 2 for some files..

void initSPI(void);
uint8_t spiSend(uint8_t data);
uint8_t spiSS(uint8_t device);
uint8_t spiSR(uint8_t device);

#endif /* SPI_H_ */