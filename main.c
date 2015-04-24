//#include <msp430.h>
#include "io430.h"
#include "subroutine.h"
#include "rf.h"
#include "ads1292.h"
#include "ADC10.h"
#include "PacketUtil.h"

void onRF_MessageReceived();
void onRF_MultiByteMessage();
void startRecording();
uchar packetDataReady = 0;
uchar helloMsg[] = {0xAA, 0x55, 0x05, 0xA0, 0x55};//todo change to const!!!!!!!!!!!!!!!1
uchar firmwareVersion[] = {0xAA, 0x55, 0x07, 0xA1,0x01,0x00, 0x55};//todo change to const!!!!!!!!!!!!!!!
uchar errMsg[] = {0xAA, 0x55, 0x07, 0xA2,0x00,0x00, 0x55};//todo change to const!!!!!!!!!!!!!!!1
uchar pingCntr = 0; 
uchar timerTask;
//������� �� ������������ RF ������ � ���������� ������ �������. 1 ���� ������� ~ 0.25 �������.
// 0 - ������������ ���������
uint resetTimeout = 0; 

int main(void)
{
  __disable_interrupt();
  sys_init();
  ADC10_Init();
  AFE_Init();
  rf_init();
  Pwr_Indication();
  __enable_interrupt();

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
  // ���� �� ����� �����
   }else{
   __bis_SR_register(CPUOFF + GIE); // ������ � ������ ����� 
   }
 }
} 
/*-----------------��������� ����������� � ���������� ���������--------------*/
void onRF_MessageReceived(){
    switch(rf_rx_buf[0]){
    case 0xFF: //stop recording command
      TACCR0 = 0x00; //timer stop
      AFE_StopRecording();
      break;
    case 0xFE: //start recording command
      startRecording();
      break;
    case 0xFD: //hello command
      rf_send(helloMsg,5);
      break;
    case 0xFC: //������ ��������
      rf_send(firmwareVersion,7);
      break;
    case 0xFB: //ping command
      pingCntr = 0;
      break;
    default:
      if(rf_rx_buf[0] <= rf_rx_buf_size){//��������� ����� �������
        //��������� ��� ��������� ����� == ������ ����� ������
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
    if(rf_rx_buf[msgOffset] == 0xF0){//������� ��� ads1292
      AFE_Cmd(rf_rx_buf[msgOffset+1]);
      msgOffset+=2;
    }else if(rf_rx_buf[msgOffset] == 0xF1){//������ ��������� ads1292
      AFE_Write_Reg(rf_rx_buf[msgOffset+1], rf_rx_buf[msgOffset+2], &rf_rx_buf[msgOffset+3]);
      msgOffset+=rf_rx_buf[msgOffset+2]+3;
    }else if(rf_rx_buf[msgOffset] == 0xF2){//�������� ������� ��� ads1292 2 ��������
      for(int i = 0; i<2; i++){
        if((rf_rx_buf[msgOffset+1+i] == 0) || (rf_rx_buf[msgOffset+1+i] == 1) || 
           (rf_rx_buf[msgOffset+1+i] == 2) || (rf_rx_buf[msgOffset+1+i] == 5) || (rf_rx_buf[msgOffset+1+i] == 10)){
          div[i] = rf_rx_buf[msgOffset+1+i]; 
        }
      }
      msgOffset+=3;
    }else if(rf_rx_buf[msgOffset] == 0xF3){//����� ������ �������������
      setAccelerometerMode(rf_rx_buf[msgOffset+1]);
      msgOffset+=2;
    }else if(rf_rx_buf[msgOffset] == 0xF4){//�������� ������ loff ������� 
      loffStatEnable = rf_rx_buf[msgOffset+1];
      msgOffset+=2;
    }else if(rf_rx_buf[msgOffset] == 0xF5){//RF reset timeout ��� ���������� Ping ������� � ����������. 
      resetTimeout = rf_rx_buf[msgOffset+1] * 4;
      msgOffset+=2;
    }else if(rf_rx_buf[msgOffset] == 0xFF){//stop recording command 
       TACCR0 = 0x00;
       AFE_StopRecording();
       msgOffset+=1;
    }else if(rf_rx_buf[msgOffset] == 0xFE){//start recording command 
       startRecording();
       msgOffset+=1;
    }else{
      rf_send(errMsg,7);
      return;
    }
  }
}

void startRecording(){
       packetUtilResetCounters();
       if(resetTimeout){
        TACCR0 = 0xFFFF;
        pingCntr = 0;
       }
       AFE_StartRecording();
}

/* ------------------------ ���������� �� P1 ----------------------- */

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
      __bic_SR_register_on_exit(CPUOFF); // �� ������������ � ��� ��� ������
    }
  }
  if (P1IFG & BIT0) { 
    P1IFG &= ~BIT0;      // Clear BT connection status flag
  }
}
/* -------------------------------------------------------------------------- */
/* ------------------------- ���������� �� ������� -------------------------- */
/* -------------------------------------------------------------------------- */
#pragma vector = TIMERA1_VECTOR
__interrupt void TimerA_ISR(void)
{ 
  TACTL &= ~TAIFG;
  if(timerTask == 0x01){
    P3OUT |= BIT7;//BT reset pin hi
    timerTask = 0;
  }
  pingCntr++;
  if(pingCntr > resetTimeout){//no signal from host for ~ resetTimeout * 4 seconds
      P3OUT &= ~BIT7; //BT reset pin lo
      timerTask = 0x01;
      pingCntr = 0;
  }
}
/* -------------------------------------------------------------------------- */ 

