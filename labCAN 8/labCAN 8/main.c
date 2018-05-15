/*
 * labCAN 8.c
 *
 * Created: 4/5/2018 6:17:15 PM
 * Author : Andrés Elizondo
 */ 


#include "sam.h"
#include "FreeRTOS.h"
#include "spi.h"
#include "uart.h"
#include "mcp_can.h"
#include "mcp_can_dfs.h"
#include "myprintf.h"
#include "GPIOcontrol_3.h"
#include "math.h"
#include "semphr.h"

#define CAN0_SPEED CAN_500KBPS


uint8_t canSlave = SLAVE_SD_CARD;

uint16_t canID;
uint8_t length;
uint8_t stmp[8];
uint8_t result;
uint32_t distance;
uint16_t delay;
uint32_t tmpDist;

// Semaphores 
xSemaphoreHandle distanceSemaphore = 0;
xSemaphoreHandle access_myprintf = 0;


void initializeTimer(){
	PORT->Group[0].PINCFG[PIN_PA27].reg = 0x00;	        // disable peripheral functions at PIN27
	PORT->Group[0].DIRSET.reg=0x08000400;				// Pin 27 configured as output
	PORT->Group[0].OUTTGL.reg = 0x08000400;            	// Output a 0 to Pin 27
	
	/* APBCMASK enable timer 3 */
	PM->APBCMASK.reg |= PM_APBCMASK_TC3;
	
	/*GCLK configuration for timer TC3*/
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TCC2_TC3) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);

	//TC3->COUNT16.CTRLA.reg = TC_CTRLA_PRESCSYNC(TC_CTRLA_PRESCSYNC_RESYNC) | TC_CTRLA_MODE(TC_CTRLA_MODE_COUNT16) | TC_CTRLA_PRESCALER(TC_CTRLA_PRESCALER_DIV1);
	
	TC3->COUNT16.CC[0].reg = TC_COUNT16_CC_CC(0x1F40);
	
	TC3->COUNT16.COUNT.reg = TC_COUNT16_COUNT_COUNT(0x0000);
	
	TC3->COUNT16.CTRLA.reg = TC_CTRLA_PRESCSYNC_RESYNC | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV1 | TC_CTRLA_ENABLE;
	TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1; // Clear flag
}

void delayMiliseconds(uint32_t max){
	while(((TC3->COUNT16.INTFLAG.reg)&(TC_INTFLAG_MC1)) != TC_INTFLAG_MC1 ) {

	}
	PORT->Group[0].OUTTGL.reg = 0x08000400;           // Output a 0 to Pin 27
	TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1;			// Clear flag
	
	
	uint32_t milliseconds = 0; //Keep tracks of the time in microseconds
	//uint32_t max = 50; //Keep tracks of when to stop a loop
	while (milliseconds < max) {
		while(((TC3->COUNT16.INTFLAG.reg)&(TC_INTFLAG_MC1)) != TC_INTFLAG_MC1 ) {

		}
		milliseconds++;
		PORT->Group[0].OUTTGL.reg = 0x08000400;           // Output a 0 to Pin 27
		TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1;			// Clear flag
	}
}

void recieveCANMessage(){
	while (1)
	{
		result = readMsgBufID(&canID,&length,stmp);
		xSemaphoreTake(distanceSemaphore,100);
		distance = (stmp[6]<<8)|stmp[7]; // Read the last two bytes of data (using two bytes because maximum value es 408 cm)
		xSemaphoreGive(distanceSemaphore); // Critical section can end because no other thread is modifying distance
		xSemaphoreTake(access_myprintf,100);
		myprintf("ID: %d \t Data: %d\n", canID, distance);
		xSemaphoreGive(access_myprintf);
		vTaskDelay(333/portTICK_PERIOD_MS/10); // (6 times a second) Half the data sending period on the other Arduino
	}
}

void sendBeepSignal(){
	while(1){
		xSemaphoreTake(distanceSemaphore,100);
		tmpDist = distance;
		xSemaphoreGive(distanceSemaphore);
		if (tmpDist < 150 && tmpDist > 0){
			// Magic formula, 20 cm = 18 ms, 150 cm = 820 ms (logarithmic)
			delay = (uint16_t) (400*log(0.0523*tmpDist)); // uses log() from math.h
			PORT->Group[0].OUTSET.reg = (1<<14); // Pin 2 (Send high signal to buzzer)
			delayMiliseconds(50); // Using Async delays to improve program flow
			
			// If distance is less than 20cm, don´t turn off the buzzer
			if (tmpDist > 20){
				PORT->Group[0].OUTCLR.reg = (1<<14); // Pin 2 (Send low signal to buzzer)
				xSemaphoreTake(access_myprintf,100);
				myprintf("Delay: \t %d ms\n",delay);
				xSemaphoreGive(access_myprintf);
				vTaskDelay(delay/portTICK_PERIOD_MS/8); // Translate milliseconds to ticks
			}else{
				vTaskDelay(20/portTICK_PERIOD_MS/8); // Translate milliseconds to ticks
			}
		}else {
			PORT->Group[0].OUTCLR.reg = (1<<14); // Pin 2
			vTaskDelay(20/portTICK_PERIOD_MS/8); // Translate milliseconds to ticks
		}
	}
}


int main(void)
{
    SystemInit();
	
	SYSCTRL->OSC8M.bit.PRESC = 0;
	initSPI();
	initUART();
	initializeTimer();
	
	canBegin(canSlave,CAN0_SPEED); // regresa 0 o 1
	uint32_t mask = 0x00000700;
	init_Mask(0,mask);
	init_Mask(1,mask);
	uint32_t filtercrit = 0x00000064;
	uint32_t filtermedium = 0x0000012C;
	uint32_t filterlow = 0x00000258;
	uint8_t extFilter = 0x00;
	init_Filt(0,extFilter,filtercrit);
	init_Filt(1,extFilter,filtermedium);
	init_Filt(2,extFilter,filterlow);
	
	
	
	InitDigOut(0,14);
	InitDigIn(0,9);
	
	// Semaphore initialization
	distanceSemaphore = xSemaphoreCreateMutex();
	access_myprintf = xSemaphoreCreateMutex();
	
	// Task creation
	// Priority 2 (Highest)
	xTaskCreate( recieveCANMessage, "recieveCANMessage", 512, NULL, 2, NULL );
	// Priority 1 (Lowest - With no interrupts but preemption)
	xTaskCreate( sendBeepSignal, "sendBeepSignal", 512, NULL, 1, NULL );
		
	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	for( ;; );
}