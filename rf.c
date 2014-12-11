#include <msp430.h>
#include "rf.h"
#include "string.h"
#include "subroutine.h"


void sendAtCommand(uchar* cmd, uchar length);

unsigned char rf_tx_in_progress; 
unsigned char tx_cntr = 0;
unsigned char rx_cntr = 0;
unsigned char tx_buf[30];
unsigned char rx_buf[20];
unsigned char tx_data_size;

void rf_init(){
  //Reset pin p3.7 and Programming mode pin p3.6
  P3DIR |= BIT6 + BIT7;
  P3OUT |= BIT6 + BIT7;
  
  P3OUT &= ~BIT6;
  
  P3OUT &= ~BIT7;
  __delay_cycles(1600000);
  P3OUT |= BIT7;
  __delay_cycles(1600000);
  
  //configure UART 230400
    P3SEL = 0x30;                            // P3.4,5 = USCI_A0 TXD/RXD
    UCA0CTL1 |= UCSSEL_2;                    // SMCLC
    UCA0BR0 = 69;                            // 16,000 MHz  230400
    UCA0BR1 = 0;                             
    UCA0MCTL = UCBRS2;               	 // Modulation UCBRSx = 4
    UCA0CTL1 &= ~UCSWRST;                    // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                         // Enable USCI_A0 RX interrupt
    
  //configure UART 38400
//  P3SEL = 0x30;                            // P3.4,5 = USCI_A0 TXD/RXD
//  UCA0CTL1 |= UCSSEL_2;                    // SMCLC
//  UCA0BR0 = 160;                            // 16 MHz  38400
//  UCA0BR1 = 1;                              // 16 MHz  38400
//  UCA0MCTL = UCBRS2 + UCBRS1;               	 // Modulation UCBRSx = 6
//  UCA0CTL1 &= ~UCSWRST;                    // **Initialize USCI state machine**
//  IE2 |= UCA0RXIE;                         // Enable USCI_A0 RX interrupt
}

void rf_prog_and_bind(){
  sendAtCommand("AT+ROLE=1\r\n", sizeof("AT+ROLE=1\r\n"));
  sendAtCommand("AT+NAME=BIMETER\r\n", sizeof("AT+NAME=BIMETER\r\n"));
  sendAtCommand("AT+UART=230400,1,0\r\n", sizeof("AT+UART=230400,1,0\r\n"));
  sendAtCommand("AT+RMAAD\r\n", sizeof("AT+RMAAD\r\n"));
  sendAtCommand("AT+BIND=14,1,211704\r\n", sizeof("AT+BIND=14,1,211704\r\n"));
  sendAtCommand("AT+CMODE=0\r\n", sizeof("AT+CMODE=0\r\n"));
  sendAtCommand("AT+ADDR?\r\n", sizeof("AT+ADDR?\r\n"));
  led(1);
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
  rx_buf[rx_cntr++] = UCA0RXBUF;
}

void startRFSending() {
  rf_tx_in_progress = 1;
  tx_cntr = 0;
  while (!(IFG2 & UCA0TXIFG));
  IFG2 &= ~UCA0TXIFG;                     //tx flag reset!!!!!!!!!
  IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt
  UCA0TXBUF = tx_buf[tx_cntr++];	
} 


#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {
  UCA0TXBUF = tx_buf[tx_cntr++];           // TX next character
  if (tx_cntr > (tx_data_size - 1)) {                 // TX over?
    IE2 &= ~UCA0TXIE;                       // Disable USCI_A0 TX interrupt
    rf_tx_in_progress = 0;
  }
}

void rf_send(uchar* cmd, uchar length){
  memcpy(tx_buf, cmd, length);
  tx_data_size = length - 1;
  startRFSending();
}

void sendAtCommand(uchar* cmd, uchar length){
  rx_cntr = 0;
  rf_send(cmd, length);
    __delay_cycles(1600000);
}

