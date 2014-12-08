#include <msp430.h>
#include "rf.h"
#include "string.h"
#include "subroutine.h"

void sendCommand(const char* cmd, unsigned char length);

unsigned char rf_tx_in_progress; 
unsigned char tx_cntr = 0;
unsigned char rx_cntr = 0;
unsigned char tx_buf[30];
unsigned char rx_buf[200];
unsigned char tx_data_size;

const char AT_Command[] = "AT\r\n";
const char AT_Role_Command[] = "AT+ROLE=1\r\n";
const char AT_Name_Command[] = "AT+NAME=BIMETER\r\n";
const char AT_UART_Command[] = "AT+UART=230400,1,0\r\n";
const char AT_RMAAD_Command[] = "AT+RMAAD\r\n";
const char AT_BIND_Command[] = "AT+BIND=14,1,143254\r\n";
const char AT_CMODE_Command[] = "AT+CMODE=0\r\n";
const char AT_Get_ADDR_Command[] = "AT+ADDR?\r\n";

void rf_init(){
  //Reset pin p3.7 and Programming mode pin p3.6
  P3DIR |= BIT6 + BIT7;
  P3OUT |= BIT6 + BIT7;
  
  P3OUT &= ~BIT7;
  __delay_cycles(1600000);
  P3OUT |= BIT7;
  __delay_cycles(1600000);
  
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
  
  sendCommand(AT_Command, sizeof(AT_Command));
  __delay_cycles(1600000);
  rx_cntr = 0;
  sendCommand(AT_Role_Command, sizeof(AT_Role_Command));
  __delay_cycles(1600000);
  rx_cntr = 0;
  sendCommand(AT_Name_Command, sizeof(AT_Name_Command));
  __delay_cycles(1600000);
  rx_cntr = 0;
  sendCommand(AT_UART_Command, sizeof(AT_UART_Command));
  __delay_cycles(1600000);
  rx_cntr = 0;
  sendCommand(AT_RMAAD_Command, sizeof(AT_RMAAD_Command));
  __delay_cycles(1600000);
  rx_cntr = 0;
  sendCommand(AT_BIND_Command, sizeof(AT_BIND_Command));
  __delay_cycles(1600000);
  rx_cntr = 0;
  sendCommand(AT_CMODE_Command, sizeof(AT_CMODE_Command));
  __delay_cycles(1600000);
  rx_cntr = 0;
  sendCommand(AT_Get_ADDR_Command, sizeof(AT_Get_ADDR_Command));
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

void sendCommand(const char* cmd, unsigned char length){
  memcpy(tx_buf, cmd, length);
  tx_data_size = length - 1;
  startRFSending();
}


