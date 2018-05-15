/*
 * GPIOcontrol_2.h
 *
 * Created: 15/10/2017
 Author: Cinthia Sanchez
 Updated: 13nov
 Removed function TogglePin
 */


#ifndef GPIOCONTROL_3_H_
#define GPIOCONTROL_3_H_

void InitDigOut(_Bool group, uint8_t pin);
void SetLevel(_Bool group, uint8_t pin, uint8_t estado);
void InitDigIn(_Bool group, uint8_t pin);
uint8_t readDigIn(_Bool group, uint8_t pin);

#endif /* GPIOCONTROL_3_H_ */