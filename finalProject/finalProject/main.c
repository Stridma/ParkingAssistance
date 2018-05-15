/* Kernel includes. */
#include "sam.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "myprintf.h"
#include "GPIOcontrol_3.h"
#include "mcp_can.h"
#include "mcp_can_dfs.h"
#include "spi.h"

/* Priorities at which the tasks are created. */
#define	myTASK_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
/* Set can bus baud rate */
#define CAN0_SPEED CAN_500KBPS


uint8_t canSlave = 0;
uint16_t canIDcrit = 0x0064; //The lower the ID the higher the priority
uint16_t canIDmedium = 0x012C;
uint16_t canIDlow = 0x0258;
uint8_t dataSize = 0x08;
uint8_t dataToSend[8] = {0,0,0,0,0,0,0,0};

/* Method to initialize the timer to make set the interrupt flag every 10 microseconds */
void initializeTimer(){
	PORT->Group[0].PINCFG[PIN_PA27].reg = 0x00;	        // disable peripheral functions at PIN27
	PORT->Group[0].DIRSET.reg=0x08000400;				// Pin 27 configured as output
	PORT->Group[0].OUTTGL.reg = 0x08000400;            	// Output a 0 to Pin 27
	
	/* APBCMASK enable timer 3 */
	PM->APBCMASK.reg |= PM_APBCMASK_TC3;
	
	/*GCLK configuration for timer TC3*/
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC2_TC3) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);

	//TC3->COUNT16.CTRLA.reg = TC_CTRLA_PRESCSYNC(TC_CTRLA_PRESCSYNC_RESYNC) | TC_CTRLA_MODE(TC_CTRLA_MODE_COUNT16) | TC_CTRLA_PRESCALER(TC_CTRLA_PRESCALER_DIV1);
	
	TC3->COUNT16.CC[0].reg = TC_COUNT16_CC_CC(0x0050);
	
	TC3->COUNT16.COUNT.reg = TC_COUNT16_COUNT_COUNT(0x0000);
	
	TC3->COUNT16.CTRLA.reg = TC_CTRLA_PRESCSYNC_RESYNC | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_ENABLE;
	TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1; // Clear flag
}

/* Method to measure the distance for the ultrasonic sensor by averaging 4 measurements during a time interval of exactly 0.2 seconds */
uint32_t measureAverageDistance(){
	uint32_t distance = 0; //Distance to return
	uint32_t nbrOfCycles = 3; //How many measurements to average from
	for (int i = 0; i<nbrOfCycles; i++){
		uint32_t microseconds = 0; //Keep tracks of the time in microseconds
		uint32_t max = 10+microseconds; //Keep tracks of when to stop a loop
		PORT->Group[0].OUTCLR.reg = (1<<6); //Before trigging the ultrasonic sensor, make sure it's quite for a while (let's say 10 micro seconds)
		while (microseconds < max) { 
			while(((TC3->COUNT16.INTFLAG.reg)&(TC_INTFLAG_MC1)) != TC_INTFLAG_MC1 ) {

			}
			microseconds+=10;
			PORT->Group[0].OUTTGL.reg = 0x08000400;           // Output a 0 to Pin 27
			TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1;			// Clear flag
		}
		/* When having been 0 for 10 microseconds trigger it by writing 1 to port 6 (Arduino zero digital pin 8) */
		PORT->Group[0].OUTSET.reg = (1<<6);
		max += 10; //Trigger it for 10 microseconds
		while (microseconds < max) {
			while(((TC3->COUNT16.INTFLAG.reg)&(TC_INTFLAG_MC1)) != TC_INTFLAG_MC1 ) {

			}
			microseconds+=10;
			PORT->Group[0].OUTTGL.reg = 0x08000400;           // Output a 0 to Pin 27
			TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1;			// Clear flag
		}
		// When done triggering write 0 again to port 6
		PORT->Group[0].OUTCLR.reg = (1<<6);
		/* Read the input from the echo signal, which is sent to port 7. Wait until it gets 1*/
		while (!readDigIn(0,7)){ 
			while(((TC3->COUNT16.INTFLAG.reg)&(TC_INTFLAG_MC1)) != TC_INTFLAG_MC1 ) {
			}
			microseconds+=10;
			PORT->Group[0].OUTTGL.reg = 0x08000400;           // Output a 0 to Pin 27
			TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1;			// Clear flag
			if (microseconds > 24000){break;}
		};
		
		/* Now the echo signal is 1, count how long the echo signal is.*/
		uint32_t start = microseconds;
		while (readDigIn(0,7)){
			while(((TC3->COUNT16.INTFLAG.reg)&(TC_INTFLAG_MC1)) != TC_INTFLAG_MC1 ) {
			}
			microseconds+=10;
			PORT->Group[0].OUTTGL.reg = 0x08000400;           // Output a 0 to Pin 27
			TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1;			// Clear flag
			if (microseconds > (24000+start)){break;}
		};
		/* The echo signal is 0 again, we know how long the pulse was in microseconds. Translate to a distance */
		distance += (microseconds-start)*0.034/2; /* distance = time*speed of sound / 2; speed of sound= 0.034 cm/microsecond, and divide by 2 since we have to go back and forth */
		max = (333333/nbrOfCycles)-microseconds; // Make sure it runs for exactly 0.2 seconds in total
		while (microseconds < max) {
			while(((TC3->COUNT16.INTFLAG.reg)&(TC_INTFLAG_MC1)) != TC_INTFLAG_MC1 ) {

			}
			microseconds+=10;
			PORT->Group[0].OUTTGL.reg = 0x08000400;           // Output a 0 to Pin 27
			TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1;			// Clear flag
		}
	}
	return distance/nbrOfCycles; //The distance is an average of the measurements made
}

void sendDistanceOverCan(uint32_t distance) {
	/*	The biggest distance to be handled is 408 cm, and a 8 bit integer can hold numbers up to 255. 
		Therefore we pass the number in two parts, one that represents how many 256 we have (0 or 1)
		and one that represents the rest. This is done by shifting and masking */
	dataToSend[6] = (uint8_t) (distance >> 8);
	dataToSend[7]= (uint8_t) (distance & 0x000000FF);
	if (distance < 100){
		sendMsgBuf(canIDcrit, 0, dataSize, (uint8_t *) dataToSend);
	} else if (distance <400) {
		sendMsgBuf(canIDmedium, 0, dataSize, (uint8_t *) dataToSend);
	} else {
		dataToSend[6] = 0xFF;
		dataToSend[7]= 0xFF;
		sendMsgBuf(canIDlow, 0, dataSize, (uint8_t *) dataToSend);
	}
}

int main()
{
	SystemInit();
	/* Switch to 8MHz clock (disable prescaler) */
	SYSCTRL->OSC8M.bit.PRESC = 0;
	/* Initiate SPI, UART, and Can */
	initSPI();
	initUART();
	canBegin(canSlave,CAN0_SPEED);
	
	/* Initialize the pins for reading and writing to the ultrasonic sensor */
	InitDigOut(0,6); // PA06 = pin 8
	InitDigIn(0,7); //PA07 = pin 9
	
	/* Initialize the timer */
	initializeTimer();
	
	while (1)
	{
		uint32_t distance = measureAverageDistance();
		sendDistanceOverCan(distance);
		myprintf("HERE");
	}
	return(0);
}



