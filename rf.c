#include <msp430.h>
#include "rf.h"
#include "string.h"
#include "subroutine.h"


void sendAtCommand(uchar* cmd);
uchar getStrSize(uchar* str);

//extern
uchar rf_tx_in_progress; 
uchar rf_data_received;
uchar rf_rx_buf[30];

uchar* rf_tx_buf;
uchar rf_tx_cntr = 0;
uchar rf_rx_cntr = 0;
uchar rf_tx_data_size;

void rf_init(){
  //Reset pin p3.7 and Programming mode pin p3.6
  P3DIR |= BIT6 + BIT7;
  P3OUT &= ~BIT6;
  P3OUT &= ~BIT7;
  __delay_cycles(1600000);
  P3OUT |= BIT7;
  __delay_cycles(1600000);
  
  //configure UART 230400
    P3SEL |= 0x30;                            // P3.4,5 = USCI_A0 TXD/RXD
    UCA0CTL1 |= UCSSEL_2;                    // SMCLC
    UCA0BR0 = 69;                            // 16,000 MHz  230400
    UCA0BR1 = 0;                             
    UCA0MCTL = UCBRS2;               	 // Modulation UCBRSx = 4
    UCA0CTL1 &= ~UCSWRST;                    // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                         // Enable USCI_A0 RX interrupt
    
  //configure UART 38400
//  P3SEL |= 0x30;                            // P3.4,5 = USCI_A0 TXD/RXD
//  UCA0CTL1 |= UCSSEL_2;                    // SMCLC
//  UCA0BR0 = 160;                            // 16 MHz  38400
//  UCA0BR1 = 1;                              // 16 MHz  38400
//  UCA0MCTL = UCBRS2 + UCBRS1;               	 // Modulation UCBRSx = 6
//  UCA0CTL1 &= ~UCSWRST;                    // **Initialize USCI state machine**
//  IE2 |= UCA0RXIE;                         // Enable USCI_A0 RX interrupt
}

void rf_prog_and_bind(){
  __delay_cycles(16000000);
  sendAtCommand("AT+ROLE=1\r\n");
  sendAtCommand("AT+NAME=BIMETER\r\n");
  sendAtCommand("AT+UART=230400,1,0\r\n");
  sendAtCommand("AT+RMAAD\r\n");
  sendAtCommand("AT+BIND=14,1,211704\r\n");
  sendAtCommand("AT+CMODE=0\r\n");
  sendAtCommand("AT+ADDR?\r\n");
  led(1);
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
  rf_rx_buf[rf_rx_cntr++] = UCA0RXBUF;

  
}

//if(UART_RX_DR){
//    rx_lost_interrupts++;
//    return;
//  }
//  UART_RX_Buf[rx_cntr] = UCA0RXBUF;
//  if((UART_RX_Buf[0]!=0x80) && (UART_RX_Buf[0]!=0x00)){
//    rx_lost_frames++;
//    rx_cntr = 0;
//    return;
//  }
//  if(rx_cntr == 1){
//    UART_RX_length = UART_RX_Buf[rx_cntr] + 4;
//  }
//  rx_cntr++;
//  if(rx_cntr > (UART_RX_length - 1)){
//    UART_RX_DR = 1;
//    rx_cntr = 0;
//    __bic_SR_register_on_exit(CPUOFF + GIE); // Не возвращаемся в сон при выходе
//  }
//


void startRFSending() {
  rf_tx_in_progress = 1;
  rf_tx_cntr = 0;
  while (!(IFG2 & UCA0TXIFG));
  IFG2 &= ~UCA0TXIFG;                     //tx flag reset!!!!!!!!!
  IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
  UCA0TXBUF = rf_tx_buf[rf_tx_cntr++];	
} 


#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {
  UCA0TXBUF = rf_tx_buf[rf_tx_cntr++];           // TX next character
  if (rf_tx_cntr > (rf_tx_data_size - 1)) {                 // TX over?
    IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
    rf_tx_in_progress = 0;
  }
}

void rf_send(uchar* cmd, uchar length){
  //memcpy(rf_tx_buf, cmd, length);
  rf_tx_buf = cmd;
  rf_tx_data_size = length;
  startRFSending();
}

void sendAtCommand(uchar* cmd){
  rf_rx_cntr = 0;
  rf_send(cmd, getStrSize(cmd));
    __delay_cycles(1600000);
}

uchar getStrSize(uchar* str){
  uchar size = 0;
  while(str[size]){
    size++;
  }
  return size;
}

