#include <msp430.h>
#include "subroutine.h"
#include "rf.h"
#include "ads1292.h"
#include "ADC10.h"

void assemblePacketAndSend();
void onRF_MessageReceived();
void onRF_MultiByteMessage();
uchar DRDYFG = 0;

int main(void)
{
 //__disable_interrupt();
  sys_init();
  ADC10_Init();
  AFE_Init();
  rf_init();
  // __disable_interrupt();
  
 // __enable_interrupt();
  //rf_prog_and_bind();

 while (1)
 {
   if(rf_rx_data_ready_fg){
     onRF_MessageReceived();
     rf_rx_data_ready_fg = 0;
   }
   if (DRDYFG) {       // if DRDY fired
     ADC10_Measure();              //start ADC10 conversion
     onAFE_DRDY();
     DRDYFG = 0;      // Clear DRDY flag
   }
   if (AFE_Data_Buf_Ready) {       // AFE buffer ready to send
     assemblePacketAndSend();
     AFE_Data_Buf_Ready = 0;
   }
   __bis_SR_register(CPUOFF + GIE); // ”ходим в сп€щий режим 
 }
} 
/*-----------------ќбработка полученного с компьютера сообщени€--------------*/
void onRF_MessageReceived(){
  if(rf_rx_buf[0]>rf_rx_buf_size){
    switch(rf_rx_buf[0]){
    case 255: //stop recording command
      AFE_StopRecording();
      break;
    case 254: //start recording command
      AFE_StartRecording();
      break;
    case 253: //hello command
      //todo send hello command
      break;
    case 252: //верси€ прошивки
      //todo send firmware vercion
      break;
    default:
      //провер€ем два последних байта == маркер конца пакета
      if(((rf_rx_buf[rf_rx_buf[0]-1] == 0x55) && (rf_rx_buf[rf_rx_buf[0]-2] == 0x55))){
        onRF_MultiByteMessage();
      }else{
        //todo send error message
      }
      break;
    }
  }
}

void onRF_MultiByteMessage(){
  uchar msgOffset = 1;
  while (msgOffset < (rf_rx_buf[0]-2)){
    if(rf_rx_buf[msgOffset] == 0x01){//«апись регистров ads1292
     // AFE_Write_Reg(uchar addr, uchar value);
      msgOffset+=rf_rx_buf[msgOffset+2]+3;
    }
    if(rf_rx_buf[msgOffset] == 0x02){//делители частоты дл€ ads1292 и акселерометра
      //todo запись делителей, проверка значений.
      msgOffset+=4;
    }
    if(rf_rx_buf[msgOffset] == 0x03){//–ежим работы акселерометра
      //todo 
      msgOffset+=2;
    }
  }
}

/* ------------------------ ѕрерывание от P1 ----------------------- */

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  P1IFG &= ~AFE_DRDY_PIN;      // Clear DRDY flag
    DRDYFG = 1;
  __bic_SR_register_on_exit(CPUOFF); // Ќе возвращаемс€ в сон при выходе
}
/* -------------------------------------------------------------------------- */

void assemblePacketAndSend(){

}