#include <msp430.h>
#include "rf.h"
#include "string.h"

void sendAtCommand();

unsigned char rf_tx_in_progress; 
unsigned char tx_cntr = 0;
unsigned char rx_cntr = 0;
unsigned char tx_buf[20];
unsigned char rx_buf[20];
unsigned char tx_data_size;

const unsigned char AT_Command[] = {'A','T','\r','\n'};


void rf_init(){
  //Reset pin p3.7 and Programming mode pin p3.6
  P3DIR |= BIT6 + BIT7;
  P3OUT |= BIT6 + BIT7;
//  __delay_cycles(16000000);
//  P3OUT &=~ BIT7;
//   __delay_cycles(16000000);
//  P3OUT |= BIT7;
  
  //configure UART 230400
  //  P3SEL = 0x30;                            // P3.4,5 = USCI_A0 TXD/RXD
  //  UCA0CTL1 |= UCSSEL_2;                    // SMCLC
  //  UCA0BR0 = 69;                            // 16,000 MHz  230400
  //  UCA0BR1 = 0;                             
  //  UCA0MCTL = UCBRS2;               	 // Modulation UCBRSx = 4
  //  UCA0CTL1 &= ~UCSWRST;                    // **Initialize USCI state machine**
  //  IE2 |= UCA0RXIE;                         // Enable USCI_A0 RX interrupt
  //configure UART 38400
  P3SEL = 0x30;                            // P3.4,5 = USCI_A0 TXD/RXD
  UCA0CTL1 |= UCSSEL_2;                    // SMCLC
  UCA0BR0 = 160;                            // 16 MHz  38400
  UCA0BR1 = 1;                              // 16 MHz  38400
  UCA0MCTL = UCBRS2 + UCBRS1;               	 // Modulation UCBRSx = 6
  UCA0CTL1 &= ~UCSWRST;                    // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                         // Enable USCI_A0 RX interrupt
  
  
  //send AT command
  sendAtCommand();
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

void sendAtCommand(){
  memcpy(tx_buf, AT_Command, 4);
  tx_data_size = 4;
  startRFSending();
}


