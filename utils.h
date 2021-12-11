/*
 * utils.h
 *
 *  Created on: Nov 8, 2021
 *      Author: amelyaavdeyev
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "tm4c123gh6pm.h"

void Disable_Interrupts(void);
void Enable_Interrupts(void);

void Config_Timer0A(void);
void Config_Port_E(void);
void ADC0_Init(void);
void Config_Port_C(void);
void Config_Timer1A(void);
void Timer1A_delay(unsigned long delay);
void Turn_Motor_Fwd(int turns);
void Turn_Motor_Bkwd(int turns);
int Read_ADC(void);
void Timer0A_Handler(void);
void Config_SysTick_Int(unsigned long Ticks);
void SysTick_Handler(void);
void Config_Port_F(void);
uint32_t ADC0_InSeq3(void);

void Config_ADC(void);
void Config_SS(void);



//void Config_SysTick_Int(unsigned long Ticks);
//
//void Timer1A_delay(unsigned long delay);
//
//void SysTick_Handler(void);
//void Timer2A_Handler(void);
//
//void Start_Timer2A_InCap(void);
//void Calculate_and_Display(unsigned long pulse_width);
//
////int Read_ADC(void);
//void Timer1A_delay(unsigned long delay);


#endif /* UTILS_H_ */
