#include <msp430.h>
#include "subroutine.h"
#include "rf.h"
#include "ads1292.h"
#include "ADC10.h"

void assemblePacketAndSend();
uchar DRDYFG = 0;

int main(void)
{
 __disable_interrupt();
  sys_init();
  ADC10_Init();
  AFE_Init();
  rf_init();
 __enable_interrupt();
  //rf_prog_and_bind();

 while (1)
 {
   if (DRDYFG) {       // if DRDY fired
     DRDYFG = 0;      // Clear DRDY flag
     ADC10_Measure();              //start ADC10 conversion
     onAFE_DRDY();
   }
   if (AFE_Data_Buf_Ready) {       // AFE buffer ready to send
     assemblePacketAndSend();
     AFE_Data_Buf_Ready = 0;
   }
   __bis_SR_register(CPUOFF + GIE); // Уходим в спящий режим 
 }
} 

/* ------------------------ Прерывание от P1 ----------------------- */

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  P1IFG &= ~AFE_DRDY_PIN;      // Clear DRDY flag
    DRDYFG = 1;
  __bic_SR_register_on_exit(CPUOFF); // Не возвращаемся в сон при выходе
}
/* -------------------------------------------------------------------------- */

void assemblePacketAndSend(){

}