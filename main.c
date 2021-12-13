/* EENG 461 Lab 2 program listing */

#include "tm4c123gh6pm.h"
#include "utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

unsigned int task_num = 0;
unsigned int T_List[] = {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};


int main(void)
{
    Disable_Interrupts();
//    Config_SysTick_Int(0x0030D400);     // 200ms interval
//    Config_SysTick_Int(0xF550C8);       // 1 sec tick
//    Config_SysTick_Int(0x1001);     // 200ms interval
    Config_Port_E();
    Config_Timer0A();
    Config_Port_C();
    Config_Port_F();
    Config_Timer1A();
    Config_ADC();
    Config_SS();
//    Config_Timer2A_InCap();
    Enable_Interrupts();
    while(1){}
}
