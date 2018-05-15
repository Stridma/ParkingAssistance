/*
 *uart.c
 *
 */ 

#include "sam.h"
#include "uart.h"

/* Replace with your library code */
void initUART(void) {

	/* APBCMASK */
   /* SERCOM 0 enable*/
	PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;

	/*GCLK configuration for sercom0 module: using generic clock generator 0, ID for sercom0, enable GCLK*/
    GCLK->GENCTRL.reg=GCLK_GENCTRL_SRC_OSC8M|GCLK_GENCTRL_ID(0)|
	GCLK_GENCTRL_GENEN;

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM0_GCLK_ID_CORE) |
	GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);

  /*CONFIGURAR PA10 y PA11 como salidas tipo C (SERCOM 0) */
  PORT->Group[0].PINCFG[PIN_PA10].reg= PA10_TX;
  PORT->Group[0].PINCFG[PIN_PA11].reg= PA11_RX;
  PORT->Group[0].PMUX[5].bit.PMUXE=Type_C;
  PORT->Group[0].PMUX[5].bit.PMUXO=Type_C;
	
	//desahibilitar UART para inicializacion
	SERCOM0->USART.CTRLA.bit.ENABLE=0;
	/* configure SERCOM0 module for UART as Standard Frame, 8 Bit size, No parity, BAUDRATE:9600*/
   /*DORD 0x1 MSB first
   MODE 0x1 USART with internal clock
   RXPO 0x3 SERCOM PAD[3] is used for data reception
   TXPO 0x1 SERCOM PAD[2] TX pin location
   
   DEFINES:
   1)SERCOM_USART_CTRLA_DORD (_U(0x1) << SERCOM_USART_CTRLA_DORD_Pos)
   2)SERCOM_USART_CTRLA_MODE_USART_INT_CLK (0x1 << 2)
   3)SERCOM_USART_CTRLA_RXPO(value) (SERCOM_USART_CTRLA_RXPO_Msk & ((value) << SERCOM_USART_CTRLA_RXPO_Pos) */
	SERCOM0->USART.CTRLA.reg =
	SERCOM_USART_CTRLA_DORD | SERCOM_USART_CTRLA_MODE_USART_INT_CLK |
	SERCOM_USART_CTRLA_RXPO(3) | SERCOM_USART_CTRLA_TXPO(1);
	
	while(SERCOM0->USART.SYNCBUSY.reg){} // waiting loading
	uint64_t br = (uint64_t)65536 * (8000000 - 16 * 9600) / 8000000;

	SERCOM0->USART.BAUD.reg = (uint16_t)br;
	
	while(SERCOM0->USART.SYNCBUSY.reg){} // waiting loading
	SERCOM0->USART.CTRLB.reg = SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_CHSIZE(0/*8 bits*/);
    while(SERCOM0->USART.SYNCBUSY.reg){} // waiting loading
	SERCOM0->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
}


