
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
int NUM_TURNS = 10;
#define GPIO_PORTA_DR4R      (*((volatile unsigned long *)0x40004500))

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

/* General Purpose Timer used for short delays (no interrupt) */
void Config_Timer0A(void)
{
    SYSCTL_RCGCTIMER_R |= 0x01;    // TIMER0 Activated
    while((SYSCTL_PRTIMER_R & 0x01) == 0){};  // Check if Timer0 is ready
    TIMER0_CTL_R = 0x00000000;    // TIMER0A disabled during setup
    TIMER0_CFG_R = 0x00000000;    // Select 32­bit timer configuration
    TIMER0_TAMR_R = 0x00000002;    // Select periodic mode, count down
    TIMER0_TAPR_R = 0x00;    // Prescale set 0
    TIMER0_TAILR_R = (delay­1);    // Set reload value
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
 TIMER1_TAILR_R = (delay­1);    // Load count interval
 TIMER1_CTL_R |= 0x00000001;    // Enable TIMER1A
 while((TIMER1_RIS_R & 0x01) == 0);  // Wait for timeout flag to set
 TIMER1_CTL_R &= ~0x00000001;    // Disable TIMER1A
}

void Turn_Motor_Fwd(int turns)
{
 int i = 0;
 for(i=1; i <= turns; i++)
 {
 GPIO_PORTC_DATA_R_PC4_7 = 0x50;  // Bin2: 0, Bin1: 1, Ain2: 0, Ain1: 1
 Timer1A_delay(0x7D00);
 GPIO_PORTC_DATA_R_PC4_7 = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
 GPIO_PORTC_DATA_R_PC4_7 = 0x60;  // Bin2: 0, Bin1: 1, Ain2: 1, Ain1: 0
 Timer1A_delay(0x7D00);
 GPIO_PORTC_DATA_R_PC4_7 = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
 GPIO_PORTC_DATA_R_PC4_7 = 0xA0;  // Bin2: 1, Bin1: 0, Ain2: 1, Ain1: 0
 Timer1A_delay(0x7D00);
 GPIO_PORTC_DATA_R_PC4_7 = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
 GPIO_PORTC_DATA_R_PC4_7 = 0x90;  // Bin2: 1, Bin1: 0, Ain2: 0, Ain1: 1
 Timer1A_delay(0x7D00);
 GPIO_PORTC_DATA_R_PC4_7 = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
 }
}

void Turn_Motor_Bkwd(int turns)
{
 int i = 0;
 for(i=1; i <= turns; i++) {
     GPIO_PORTC_DATA_R_PC4_7 = 0x90;  // Bin2: 1, Bin1: 0, Ain2: 0, Ain1: 1
     Timer1A_delay(0x7D00);
     GPIO_PORTC_DATA_R_PC4_7 = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
     GPIO_PORTC_DATA_R_PC4_7 = 0xA0;  // Bin2: 1, Bin1: 0, Ain2: 1, Ain1: 0
     Timer1A_delay(0x7D00);
     Timer1A_delay(0x7D00);
     GPIO_PORTC_DATA_R_PC4_7 = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
     GPIO_PORTC_DATA_R_PC4_7 = 0x60;  // Bin2: 0, Bin1: 1, Ain2: 1, Ain1: 0
     Timer1A_delay(0x7D00);
     GPIO_PORTC_DATA_R_PC4_7 = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
     GPIO_PORTC_DATA_R_PC4_7 = 0x50;  // Bin2: 0, Bin1: 1, Ain2: 0, Ain1: 1
     Timer1A_delay(0x7D00);
     GPIO_PORTC_DATA_R_PC4_7 = 0x00;  // Bin2: 0, Bin1: 0, Ain2: 0, Ain1: 0
     }
}

void Timer0A_Handler(void)
{
 Disable_Interrupts();
 brightness = Read_ADC();
       // insert motor control code here ***
 // if brightness > 3280 // (75%)
 //     Turn_Motor_Fwd(NUM_TURNS);
 // }
 // else if brightness < 1025 // (25%)

 //     Turn_Motor_Bkwd(NUM_TURNS);
 // else
     // printf("do nothing\n");


 TIMER0_ICR_R = 0x00000001;  // Clear flags in GPTMRIS and GPTMMIS
 Enable_Interrupts();
}

int Read_ADC(void) {
//      volatile uint32_t result;
//      ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
//      while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
//        // if you have an A0-A3 revision number, you need to add an 8 usec wait here
//      result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
//      ADC0_ISC_R = 0x0008;             // 4) acknowledge completion

      GPIO_PORTF_DATA_R = 0x04;
      Timer0A_delay(0x007A1200);
      GPIO_PORTF_DATA_R = 0x00;
//      return result;
//    }
    return 0;
}
