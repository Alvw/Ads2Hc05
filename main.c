//#include <msp430.h>
#include "io430.h"
#include "subroutine.h"
#include "rf.h"
#include "ads1292.h"
#include "ADC10.h"
#include "PacketUtil.h"

void onRF_MessageReceived();
void onRF_MultiByteMessage();
uchar packetDataReady = 0;
uchar helloMsg[] = {0xAA, 0x55, 0x05, 0xA0, 0x55};//todo change to const!!!!!!!!!!!!!!!1
uchar firmwareVersion[] = {0xAA, 0x55, 0x07, 0xA1,0x01,0x00, 0x55};//todo change to const!!!!!!!!!!!!!!!
uchar errMsg[] = {0xAA, 0x55, 0x07, 0xA2,0x00,0x00, 0x55};//todo change to const!!!!!!!!!!!!!!!1

int main(void)
{
  __disable_interrupt();
  sys_init();
  ADC10_Init();
  AFE_Init();
  rf_init();
  Pwr_Indication();
  __enable_interrupt();
 // rf_prog_and_bind();

 while (1)
 {
   if(rf_rx_data_ready_fg) {
     onRF_MessageReceived();
     rf_rx_data_ready_fg = 0;
   }
   if (packetDataReady){       
     uchar packetSize = assemblePacket();
     if(!rf_tx_in_progress){
       rf_send((uchar*)&packet_buf[0], packetSize);
     }
     packetDataReady = 0;      
   }
   if(rf_rx_data_ready_fg || packetDataReady){
  // идем по циклу снова
   }else{
   __bis_SR_register(CPUOFF + GIE); // ”ходим в сп€щий режим 
   }
 }
} 
/*-----------------ќбработка полученного с компьютера сообщени€--------------*/
void onRF_MessageReceived(){
    switch(rf_rx_buf[0]){
    case 0xFF: //stop recording command
      AFE_StopRecording();
      break;
    case 0xFE: //start recording command
      AFE_StartRecording();
      break;
    case 0xFD: //hello command
      rf_send(helloMsg,5);
      break;
    case 0xFC: //верси€ прошивки
      rf_send(firmwareVersion,7);
      break;
    default:
      if(rf_rx_buf[0] <= rf_rx_buf_size){//провер€ем длину команды
        //провер€ем два последних байта == маркер конца пакета
        if(((rf_rx_buf[rf_rx_buf[0]-1] == 0x55) && (rf_rx_buf[rf_rx_buf[0]-2] == 0x55))){
          onRF_MultiByteMessage();
        }else{
          rf_send(errMsg,7);
        }
      }
      break;
    }
}

void onRF_MultiByteMessage(){
  uchar msgOffset = 1;
  while (msgOffset < (rf_rx_buf[0]-2)){
    if(rf_rx_buf[msgOffset] == 0xF0){//команда дл€ ads1292
      AFE_Cmd(rf_rx_buf[msgOffset+1]);
      msgOffset+=2;
    }else if(rf_rx_buf[msgOffset] == 0xF1){//«апись регистров ads1292
      AFE_Write_Reg(rf_rx_buf[msgOffset+1], rf_rx_buf[msgOffset+2], &rf_rx_buf[msgOffset+3]);
      msgOffset+=rf_rx_buf[msgOffset+2]+3;
    }else if(rf_rx_buf[msgOffset] == 0xF2){//делители частоты дл€ ads1292 2 значени€
      for(int i = 0; i<2; i++){
        if((rf_rx_buf[msgOffset+1+i] == 0) || (rf_rx_buf[msgOffset+1+i] == 1) || 
           (rf_rx_buf[msgOffset+1+i] == 2) || (rf_rx_buf[msgOffset+1+i] == 5) || (rf_rx_buf[msgOffset+1+i] == 10)){
          div[i] = rf_rx_buf[msgOffset+1+i]; 
        }
      }
      msgOffset+=3;
    }else if(rf_rx_buf[msgOffset] == 0xF3){//–ежим работы акселерометра
      setAccelerometerMode(rf_rx_buf[msgOffset+1]);
      msgOffset+=2;
    }else if(rf_rx_buf[msgOffset] == 0xF4){//передача данных loff статуса 
      loffStatEnable = rf_rx_buf[msgOffset+1];
      msgOffset+=2;
    }else if(rf_rx_buf[msgOffset] == 0xFF){//stop recording command 
       AFE_StopRecording();
       msgOffset+=1;
    }else if(rf_rx_buf[msgOffset] == 0xFE){//start recording command 
       AFE_StartRecording();
       msgOffset+=1;
    }else{
      rf_send(errMsg,7);
      return;
    }
  }
}

/* ------------------------ ѕрерывание от P1 ----------------------- */

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
  if (P1IFG & AFE_DRDY_PIN) { 
    P1IFG &= ~AFE_DRDY_PIN;      // Clear DRDY flag
    long new_data[6];
    AFE_Read_Data(&new_data[0]);
    loffStat = AFE_getLoffStatus();
    ADC10_Read_Data(&new_data[2]);
    ADC10_Measure();
    if(packetAddNewData(new_data)){
      packetDataReady = 1;
      __bic_SR_register_on_exit(CPUOFF); // Ќе возвращаемс€ в сон при выходе
    }
  }
}
/* -------------------------------------------------------------------------- */
