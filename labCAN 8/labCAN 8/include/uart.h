/*
 * uart.h
 *
 */ 


#ifndef UART_H_
#define UART_H_

//SALIDA TX ENTRADA RX
#define PA10_TX 0x01
#define PA11_RX 0x07 
#define Type_C 0x2u;
//calculo BAUDRATE con Fref=8Mhz, Fbaud=9600 S=16	
//#define BAUDRATESET(x) uint64_t br = (uint64_t)65536 * (8000000 - 16 * x) / 8000000;

void initUART(void);
#endif /* UART_H_ */