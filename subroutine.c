#include <msp430.h>
#include "subroutine.h"

//Инициализация микроконтроллера
void sys_init(){
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
 // CLOCK
  BCSCTL1 = CALBC1_16MHZ;                    
  DCOCTL = CALDCO_16MHZ;
  
// Неиспользуемые выводы
  P1DIR |= BIT1 + BIT3 + BIT5 + BIT6;
  P1OUT &= ~(BIT1 + BIT3 + BIT5 + BIT6);
  
  P2DIR |= BIT4 + BIT5;
  P2OUT &= ~(BIT4 + BIT5);
  
  P3DIR |= BIT0;
  P3OUT &= ~BIT0;
  
  P4DIR |= BIT0 + BIT1 + BIT2 + BIT3;
  P4OUT &= ~(BIT0 + BIT1 + BIT2 + BIT3);
  
  //Светодиод
  P1DIR |= BIT7;
  P1OUT &=~BIT7;
}

void led(unsigned char state){
  if(state){
    P1OUT |= BIT7;
  }else{
    P1OUT &=~BIT7;
  }
}