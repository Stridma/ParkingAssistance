/*
* GPIOcontrol_3.0.c
*
* Created: 15/10/2017
 Author Cinthia Sanchez
 Updated: 13nov
 Removed function TogglePin
*/ 

#include "sam.h"
#include "stdbool.h"
#include "GPIOcontrol_3.h"

/* Funcion para configurar un puerto como salida recibe dos parametros:  group: espera un 0 o 1 que indique el puerto, y pin: espera un valor entero 
que indique el pin en el puerto*/
void InitDigOut(_Bool group, uint8_t pin){
	
	PORT->Group[(uint8_t)group].PINCFG[pin].reg = 0x00;
	PORT->Group[(uint8_t)group].DIRSET.reg = (1 << pin) ;
}

/*Funcion set level recibe tres parametros: group: espera un 0 o 1 que indica el puerto, pin: espera un entero que indique el pin 
estado: espera un entero que indique el nivel */
void SetLevel(_Bool group, uint8_t pin, uint8_t estado){
	if(estado == 1){
		PORT ->Group[(uint8_t)group].OUTSET.reg = (1 << pin);
	}
	if(estado == 0){
		PORT ->Group[(uint8_t)group].OUTCLR.reg = (1 << pin);
	}
}

void InitDigIn(_Bool group, uint8_t pin)
{
	/*Dado un puerto y un numero de pin, del 0 al 31 para el puerto A(0), y ...AGREGAR PINES PUERTO B 
	los configura como entrada, es decir activa INEN,bit1, y PULLEN, bit2, y con el registro DIRCLR se activa la direccion como entrada*/
	PORT->Group[(uint8_t)group].PINCFG[pin].reg=0x03;
	PORT->Group[(uint8_t)group].DIRCLR.reg= (1<<pin);
	
	//habilita SAMPLING
	PORT->Group[(uint8_t)group].CTRL.reg=(1<<pin);
}
	
uint8_t readDigIn(_Bool group, uint8_t pin)
{
	uint32_t mask= (1<<pin);
	if((PORT->Group[(uint8_t)group].IN.reg&mask)==0)
		return 0;
	else
	return 1;
}