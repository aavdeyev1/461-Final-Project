
#include "utils.h"
#include "tm4c123gh6pm.h"

volatile uint32_t ADC_Sample;
int setting;
int edge_cnt;
int pulse_width;
int timer_value1;
int timer_value2;
char str_setting[20];
int brightness;
int NUM_TURNS = 5;
int MAX_TURNS = 5;
//#define delay1      (*((volatile unsigned long *)0x00F550C8))

//volatile uint32_t delay1 = 0xF550C8;
//#define GPIO_PORTA_DR4R      (*((volatile unsigned long *)0x40004500))

void Disable_Interrupts(void)
{
    __asm ("  CPSID    I\n" // disable interrupts
           "  BX       LR\n"); // the PC return to LR value
}

void Enable_Interrupts(void)
{
    __asm ("  CPSIE    I\n"
           "  BX       LR\n");
}

uint32_t ADC0_InSeq3(void)
{
  volatile uint32_t result;
  ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
  while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
    // if you have an A0-A3 revision number, you need to add an 8 usec wait here
  result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
  ADC0_ISC_R = 0x0008;             // 4) acknowledge completion

//  GPIO_PORTF_DATA_R = 0x04;
//  Timer0A_delay(0x007A1200);
//  GPIO_PORTF_DATA_R = 0x00;
  return result;
}

void SysTick_Handler(void)// Can remove highlighted lines because there’s Nested Vectored Interrupts that’ll handle it
{
//    ADC_Sample = ADC0_InSeq3();
//    setting = ceil(ADC_Sample / 4.10);
//    printf("Setting %d\n", setting);
//    GPIO_PORTF_DATA_R = 0x00;
//
//    else if ((int)setting < 18) // blue
//    {
//    if (NUM_TURNS > 0) { // only turn the motor if there are still turns left
//            GPIO_PORTF_DATA_R = 0x00;
//            GPIO_PORTF_DATA_R = 0x04;
//            Turn_Motor_Fwd(NUM_TURNS);
//    }
//    }
//    else if((int)setting > 75) // red
//    {
//    if (NUM_TURNS < 0) { // only turn the motor if there are still turns left
//            GPIO_PORTF_DATA_R = 0x00;
//            GPIO_PORTF_DATA_R = 0x02;
//            Turn_Motor_Bkwd(NUM_TURNS);
//    }
//    }
//    else //green
//    {
//        GPIO_PORTF_DATA_R = 0x00;
//            GPIO_PORTF_DATA_R = 0x08;
//    }
//
//    GPIO_PORTF_DATA_R = 0x00;
//
//    TIMER0_ICR_R = 0x00000001;  // Clear flags in GPTMRIS and GPTMMIS
//}

}

void Config_ADC(void)
{
    SYSCTL_RCGCGPIO_R |= 0x10;      // Enable clock for Port E
    while((SYSCTL_PRGPIO_R & 0x10)!= 0x10){};// Wait until Port E is ready
    GPIO_PORTE_DIR_R &= ~0x20;      // Configure PE5 for input
    GPIO_PORTE_AFSEL_R |= 0x20;     // Enable alternate functions for PE52
    GPIO_PORTE_DEN_R &= ~0x20;      // Disable digital functions for PE5
    GPIO_PORTE_AMSEL_R |= 0x20;     // Enable analog function for PE5
}

void Config_SS(void)
{

   SYSCTL_RCGCADC_R |= 0x0001;                // Enable ADC0
   while((SYSCTL_PRADC_R & 0x01) != 0x01){};  // Wait until ADC0 is ready
   ADC0_PC_R &= ~0xF;       // Clear the ADCPC0 sample rate field
   ADC0_PC_R |= 0x1;        // Configure for 125K samples/sec
   ADC0_SSPRI_R = 0x0123;    // Sequencer 3 set to highest priority
   ADC0_ACTSS_R &= ~0x0008;  // Disable sample sequencer 3 for setup
   ADC0_EMUX_R &= ~0xF000;   // Software trigger selected
   ADC0_SSMUX3_R &= ~0x000F; // Clear the SSMUX3 input field
   ADC0_SSMUX3_R = 0x8;      // SSMUX3 sample input set for AIN8 (PE5)
   ADC0_SSCTL3_R = 0x0006;   // IE0 and END0 are set
   ADC0_IM_R &= ~0x0008;     // SSMUX3 interrupts are masked
   ADC0_ACTSS_R |= 0x0008;   // Enable sample sequencer 3

}


/* General Purpose Timer used for short delays (no interrupt) */
void Config_Timer0A(void)
{
    SYSCTL_RCGCTIMER_R |= 0x01;    // TIMER0 Activated
    while((SYSCTL_PRTIMER_R & 0x01) == 0){};  // Check if Timer0 is ready
    TIMER0_CTL_R = 0x00000000;    // TIMER0A disabled during setup
    TIMER0_CFG_R = 0x00000000;    // Select 32­bit timer configuration
    TIMER0_TAMR_R = 0x00000002;    // Select periodic mode, count down
    TIMER0_TAPR_R = 0x00;    // Prescale set 0
    TIMER0_TAILR_R = 0x1E847FF;    // Set reload value
    TIMER0_IMR_R |= 0x00000001;    // Interrupt enabled
    TIMER0_ICR_R = 0x00000001;    // Clear flags in GPTMRIS and GPTMMIS
    NVIC_PRI4_R = ((NVIC_PRI4_R & 0x00FFFFFF) | 0xA0000000); // INT 19, Priority 5
    NVIC_EN0_R = 0x00080000;                    // Enable INT 19 in NVIC
    TIMER0_CTL_R |= 0x00000001;                  // TIMER0A enabled
}

void Config_Port_E(void)
{
 SYSCTL_RCGCGPIO_R |= 0x10;    // Enable clock for Port E
 while((SYSCTL_PRGPIO_R & 0x10) == 0){}  // Wait until Port E is ready
 GPIO_PORTE_DIR_R &= ~0x10;    // Configure PE4 for input
 GPIO_PORTE_AFSEL_R |= 0x10;    // Enable alternate functions for PE4
 GPIO_PORTE_DEN_R &= ~0x10;    // Disable digital functions for PE4
 GPIO_PORTE_AMSEL_R |= 0x10;    // Enable analog function for PE4
}

void ADC0_Init(void)
{
 SYSCTL_RCGCADC_R |= 0x0001;    // Enable ADC0
 while((SYSCTL_PRADC_R & 0x01) != 0x01){};   // Wait until ADC0 is ready
 ADC0_PC_R &= ~0xF;    // Clear the ADCPC0 sample rate field
 ADC0_PC_R |= 0x1;    // Configure for 125K samples/sec
 ADC0_SSPRI_R = 0x0123;    // Sequencer 3 set to highest priority
 ADC0_ACTSS_R &= ~0x0008;    // Disable sample sequencer 3 for setup
 ADC0_EMUX_R &= ~0xF000;    // Software trigger selected
 ADC0_SSMUX3_R &= ~0x000F;    // Clear the SSMUX3 input field
 ADC0_SSMUX3_R = 0x9;    // SSMUX3 sample input set for AIN9 (PE4)
 ADC0_SSCTL3_R = 0x0006;    // IE0 and END0 are set
 ADC0_IM_R &= ~0x0008;    // SSMUX3 interrupts are masked
 ADC0_ACTSS_R |= 0x0008;    // Enable sample sequencer 3
}

void Config_Port_C(void)  // Configure PC4­PC7 as outputs
{
 SYSCTL_RCGCGPIO_R |= 0x04;  // Enable clock for Port C
 while((SYSCTL_PRGPIO_R & 0x04) == 0){}  // Wait until Port C is ready
 GPIO_PORTC_AMSEL_R &= ~0xF0;    // Disable analog function for PC4­PC7
 GPIO_PORTC_PCTL_R &= ~0xFFFF0000;    // Configure PC4­PC7 as I/O ports
 GPIO_PORTC_DIR_R |= 0xF0;    // PC4­PC7 configured as outputs
 GPIO_PORTC_AFSEL_R &= ~0xF0;    // Disable alternate function for PC4­PC7
 GPIO_PORTC_DEN_R |= 0xF0;    // Enable digital I/O for PC4­PC7
 GPIO_PORTC_DATA_R &= ~0xF0;    // Clear PC4­PC7
}

void Config_Port_F(void)
{
    SYSCTL_RCGCGPIO_R |= 0x20;              // Enable clock for Port F
    while((SYSCTL_PRGPIO_R&0x20) == 0){}    // Wait until Port F is ready
    GPIO_PORTF_AMSEL_R &= ~0x1E;            // Disable analog function for PF1-PF4 - 1111.1111.1111.1111.1111.1111.1110.0001
    GPIO_PORTF_PCTL_R &= ~0x000FFFF0;       // Configure PF1-PF4 as I/O ports - 1111.1111.1111.1111.0000.0000.0000.1111
    GPIO_PORTF_DIR_R |= 0x0E;               // PF1-PF3 configured as outputs
    GPIO_PORTF_DIR_R &= ~0x10;              // PF4 configured as input
    GPIO_PORTF_AFSEL_R &= ~0x1E;            // Disable alternate function for PF1-PF4
    GPIO_PORTF_DEN_R |= 0x1E;               // Enable digital I/O for PF1-PF4
    GPIO_PORTF_PUR_R |= 0x10;               // Set weak pull-up on PF4
    GPIO_PORTF_DATA_R &= ~0x0E;             // Clear PF1-PF3

    /* Interrupt settings for PF4 */
    GPIO_PORTF_IM_R &= ~0x10;               // Mask interrupt on PF4
    GPIO_PORTF_IS_R &= ~0x10;               // Set PF4 to edge-sensitive
    GPIO_PORTF_IBE_R &= ~0x10;              // Set PF4 for not both edges
    GPIO_PORTF_IEV_R &= ~0x10;              // Set PF4 for falling edge detect
    GPIO_PORTF_ICR_R = 0x10;                // Clear interrupt flag in GPIORIS and GPIOMIS
    NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000;  // Set to priority level 5
    NVIC_EN0_R |= 0x40000000;               // Enable interrupt 30 in NVIC
    GPIO_PORTF_IM_R |= 0x10;                // Unmask interrupt on PF4
}

void Config_Timer1A(void)
{
 SYSCTL_RCGCTIMER_R |= 0x02;    // Activate TIMER1
 while((SYSCTL_PRTIMER_R & 0x02) == 0){};  // Check if Timer1 is ready
 TIMER1_CTL_R &= ~0x00000001;    // Disable TIMER1A
 TIMER1_CFG_R = 0x00000000;    // Select 32­bit timer configuration
 TIMER1_TAMR_R = 0x00000001;    // Select one­shot mode, timer counting down
 TIMER1_TAPR_R = 0x00;    // Prescale set to 0
}
void Timer1A_delay(unsigned long delay)
{
 TIMER1_ICR_R = 0x00000001;    // Clear TIMER1A timeout flag
 TIMER1_TAILR_R = delay;    // Load count interval 0xF550C8
 TIMER1_CTL_R |= 0x00000001;    // Enable TIMER1A
 while((TIMER1_RIS_R & 0x01) == 0);  // Wait for timeout flag to set
 TIMER1_CTL_R &= ~0x00000001;    // Disable TIMER1A
}

void Turn_Motor_Fwd(int turns)
{
    NUM_TURNS = NUM_TURNS - 1;
 int i = 0;
 for(i=1; i <= turns; i++)
 {
     GPIO_PORTC_DATA_R = 0xA0;  // Bin2: 0, Bin1: 1, Ain2: 0, Ain1: 1 3
      Timer1A_delay(0x7D00);
//     Timer1A_delay(0x30D400);
//     Timer1A_delay(0xFA0);
      GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
      GPIO_PORTC_DATA_R = 0x60;  // Bin2: 0, Bin1: 1, Ain2: 1, Ain1: 0  3
      Timer1A_delay(0x7D00);
//      Timer1A_delay(0x30D400);
//      Timer1A_delay(0xFA0);
      GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
      GPIO_PORTC_DATA_R = 0x50;  // Bin2: 1, Bin1: 0, Ain2: 1, Ain1: 0  c
      Timer1A_delay(0x7D00);
//      Timer1A_delay(0x30D400);
//      Timer1A_delay(0xFA0);
      GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
      GPIO_PORTC_DATA_R = 0x90;  // Bin2: 1, Bin1: 0, Ain2: 0, Ain1: 1  9
      Timer1A_delay(0x7D00);
//      Timer1A_delay(0x30D400);
//      Timer1A_delay(0xFA0);
      GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
// GPIO_PORTC_DATA_R = 0x50;  // Bin2: 0, Bin1: 1, Ain2: 0, Ain1: 1
// Timer1A_delay(0x7D00);
// GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
// GPIO_PORTC_DATA_R = 0x60;  // Bin2: 0, Bin1: 1, Ain2: 1, Ain1: 0
// Timer1A_delay(0x7D00);
// GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
// GPIO_PORTC_DATA_R = 0xA0;  // Bin2: 1, Bin1: 0, Ain2: 1, Ain1: 0
// Timer1A_delay(0x7D00);
// GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
// GPIO_PORTC_DATA_R = 0x90;  // Bin2: 1, Bin1: 0, Ain2: 0, Ain1: 1
// Timer1A_delay(0x7D00);
// GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
 }
}

void Turn_Motor_Bkwd(int turns)
{
    NUM_TURNS = NUM_TURNS + 1;
 int i = 0;
 for(i=1; i <= turns; i++) {
     GPIO_PORTC_DATA_R = 0x90;  // Bin2: 0, Bin1: 1, Ain2: 0, Ain1: 1 3
           Timer1A_delay(0x7D00);
     //     Timer1A_delay(0x30D400);
     //     Timer1A_delay(0xFA0);
           GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
           GPIO_PORTC_DATA_R = 0x50;  // Bin2: 0, Bin1: 1, Ain2: 1, Ain1: 0  3
           Timer1A_delay(0x7D00);
     //      Timer1A_delay(0x30D400);
     //      Timer1A_delay(0xFA0);
           GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
           GPIO_PORTC_DATA_R = 0x60;  // Bin2: 1, Bin1: 0, Ain2: 1, Ain1: 0  c
           Timer1A_delay(0x7D00);
     //      Timer1A_delay(0x30D400);
     //      Timer1A_delay(0xFA0);
           GPIO_PORTC_DATA_R = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
           GPIO_PORTC_DATA_R = 0xA0;  // Bin2: 1, Bin1: 0, Ain2: 0, Ain1: 1  9
           Timer1A_delay(0x7D00);
     //      Timer1A_delay(0x30D400);
     //      Timer1A_delay(0xFA0);
           GPIO_PORTC_DATA_R = 0x00; }
}

int Read_ADC(void) {
//      volatile uint32_t result;
//      ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
//      while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
//        // if you have an A0-A3 revision number, you need to add an 8 usec wait here
//      result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
//      ADC0_ISC_R = 0x0008;             // 4) acknowledge completion

//      GPIO_PORTF_DATA_R = 0x04;
//      Timer1A_delay(0x007A1200);
//      GPIO_PORTF_DATA_R = 0x00;
//      return result;
//    }
    return 0;
}


void Timer0A_Handler(void)
{
//    Turn_Motor_Fwd(100);
// brightness = Read_ADC();
// insert motor control code here ***
// The motor control code doesn’t have to be complicated.  Pick a range of rotation between 90 and 180
// degrees and divide the range up into steps (the number will vary depending on the motor you’re using).
// When the light level changes by 10%, rotate the stepper motor through 1/10 the number of steps in the
// range.

 // This stepper motor has a stride angle of 5.625 degree. That means 28BYJ-48 will complete one revolution in
 // (360/5.625) 64 steps by taking one step at a time and in one step it covers 5.625 degree distance. But this
 // stepper motor can also be used in full step mode. In full step mode, the angle of each step is 11.25 degrees.
 // That means the motor completes its one revolution in 32 steps ( 360/11.25).

 // stride = 5.635'
 // 90/5.635' = steps for full rotation of blinds = 16
 // input resolution => .3 V/2^12 = 73 uV
 // input/4 (for 25% increments) = .03 V
 // step size = .03V/73uV =



 // if brightness > 3280 // (75%)
 //     Turn_Motor_Fwd(4);
 // }
 // else if brightness < 1025 // (25%)

 //     Turn_Motor_Bkwd(4);
 // else
     // printf("do nothing\n");
    ADC_Sample = ADC0_InSeq3();
    setting = ceil(ADC_Sample / 4.10);
    printf("Setting %d\n", setting);
    GPIO_PORTF_DATA_R = 0x00;

    if ((int)setting < 18) // blue
    {
        if (NUM_TURNS > 0) { // only turn the motor if there are still turns left
            GPIO_PORTF_DATA_R = 0x00;
            GPIO_PORTF_DATA_R = 0x04;
            Turn_Motor_Fwd(50);
        }
    }
    else if((int)setting > 75) // red
    {
    if (NUM_TURNS < MAX_TURNS) { // only turn the motor if there are still turns left
            GPIO_PORTF_DATA_R = 0x00;
            GPIO_PORTF_DATA_R = 0x02;
            Turn_Motor_Bkwd(50);
    }
    }
    else //green
    {
        GPIO_PORTF_DATA_R = 0x00;
            GPIO_PORTF_DATA_R = 0x08;
    }

    GPIO_PORTF_DATA_R = 0x00;

    TIMER0_ICR_R = 0x00000001;  // Clear flags in GPTMRIS and GPTMMIS

}


//void SysTick_Handler(void) // Can remove highlighted lines because there’s Nested Vectored Interrupts that’ll handle it
//{
////    ADC_Sample = ReadADC();
////
////    if ((int)ADC_Sample < 2870)
////    {
////        GPIO_PORTF_DATA_R = 0x04;
////        Timer1A_delay(0x007A1200);
////    }
////    else if((int)ADC_Sample > 3280)
////    {
////            GPIO_PORTF_DATA_R = 0x02;
////            Timer1A_delay(0x007A1200);
////    }
////    else
////    {
////            GPIO_PORTF_DATA_R = 0x08;
////            Timer1A_delay(0x007A1200);
////    }
////
//    GPIO_PORTF_DATA_R = 0x00;
//    GPIO_PORTF_DATA_R = 0x08;
////    Timer1A_delay(0x00061A80);
////    ADC_Sample = ADC0_InSeq3();
////    setting = ceil(ADC_Sample / 41.0);
////    printf("Setting %d\n", setting);
////    setting = ceil(ADC_Sample / 41.0);
////    printf("Setting: %d%", setting);
//
//    Timer0A_Handler();
////    Timer1A_delay(0x00061A80);
//}


void Config_SysTick_Int(unsigned long Ticks)
{
    NVIC_ST_CTRL_R = 0x0;                           // Disable the timer
    NVIC_ST_RELOAD_R = (Ticks - 1);                 // Set the Reload Value register
    NVIC_ST_CURRENT_R = 0x0;                        // Reset the Current Value register
    NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF)|0x80000000; // Set to Priority Level 4
    NVIC_ST_CTRL_R = 0x00000007;                    // Enable SysTick with interrupts
}
