#include <msp430.h>
#include "subroutine.h"
#include "rf.h"


volatile unsigned int i;                    // volatile to prevent optimization

int main(void)
{
 //__disable_interrupt();
  sys_init();
 __enable_interrupt();
  rf_init();
  while (1)
  {
   __bis_SR_register(CPUOFF + GIE); // ������ � ������ ����� 

  }
} 

//AFE_Read_Data(&spiRxBuf[0], 9);