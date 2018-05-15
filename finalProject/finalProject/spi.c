/*
 * spi.c
 *
 */ 
#include "sam.h"
#include "spi.h"

void initSPI(void) {
	
	// SLAVE SELECT PIN 8 ARDUINO
	REG_PORT_DIRSET0 = PORT_PA06;
	REG_PORT_OUTSET0 = PORT_PA06;
	// SLAVE SELECT PIN 9 ARDUINO
	REG_PORT_DIRSET0 = PORT_PA07;
	REG_PORT_OUTSET0 = PORT_PA07;
	// SLAVE SELECT PIN 10 ARDUINO
	REG_PORT_DIRSET0 = PORT_PA18;
	REG_PORT_OUTSET0 = PORT_PA18;
	
	// INIT SPI PROTOCOL
	PM->APBCMASK.bit.SERCOM1_ = 1;
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_SERCOM1_CORE;
	while(GCLK->STATUS.bit.SYNCBUSY);
	const SERCOM_SPI_CTRLA_Type ctrla = {
		.bit.DORD = 0, // MSB first
		.bit.CPHA = 0, // Mode 0
		.bit.CPOL = 0,
		.bit.FORM = 0, // SPI frame
		.bit.DIPO = 3, // MISO on PAD[3]
		.bit.DOPO = 0, // MOSI on PAD[0], SCK on PAD[1], SS_ on PAD[2]
		.bit.MODE = 3  // Master
	};
	SERCOM1->SPI.CTRLA.reg = ctrla.reg;
	const SERCOM_SPI_CTRLB_Type ctrlb = {
		.bit.RXEN = 1,   // RX enabled
		.bit.MSSEN = 0,  // Manual SC
		.bit.CHSIZE = 0 // 8-bit
	};
	SERCOM1->SPI.CTRLB.reg = ctrlb.reg;

	SERCOM1->SPI.BAUD.reg = 2; // Rate is clock / 2

	// Mux for SERCOM1 PA16,PA17,PA19
	const PORT_WRCONFIG_Type wrconfig = {
		.bit.WRPINCFG = 1,
		.bit.WRPMUX = 1,
		.bit.PMUX = MUX_PA16C_SERCOM1_PAD0,
		.bit.PMUXEN = 1,
		.bit.HWSEL = 1,
		.bit.PINMASK = (uint16_t)((PORT_PA16 | PORT_PA17 | PORT_PA19) >> 16)
	};
	PORT->Group[0].WRCONFIG.reg = wrconfig.reg;

	SERCOM1->SPI.CTRLA.bit.ENABLE = 1;
	while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE);
}

uint8_t spiSend(uint8_t data) {
	uint8_t ret;
	while(SERCOM1->SPI.INTFLAG.bit.DRE == 0);
	SERCOM1->SPI.DATA.reg = data;
	while(SERCOM1->SPI.INTFLAG.bit.TXC == 0);
	while(SERCOM1->SPI.INTFLAG.bit.RXC == 0);
	ret = SERCOM1->SPI.DATA.reg;
	return ret;
}

uint8_t spiSS(uint8_t device)
{
	spiSend(0xFF);
	int ret = 0;
	switch (device)
	{
		case SLAVE_SD_CARD:
		REG_PORT_OUTCLR0 = PORT_PA18;
		break;
		
		case SLAVE_CAN_0:
		REG_PORT_OUTCLR0 = PORT_PA07;
		break;
		
		case SLAVE_CAN_1:
		REG_PORT_OUTCLR0 = PORT_PA06;
		break;
		
		default:
		ret = 1;
		break;
	}
	return ret;
}

uint8_t spiSR(uint8_t device)
{
	int ret = 0;
	switch (device)
	{
		case SLAVE_SD_CARD:
		REG_PORT_OUTSET0 = PORT_PA18;
		break;
		
		case SLAVE_CAN_0:
		REG_PORT_OUTSET0 = PORT_PA07;
		break;
		
		case SLAVE_CAN_1:
		REG_PORT_OUTSET0 = PORT_PA06;
		break;
		
		default:
		ret = 1;
		break;
	}
	return ret;
}