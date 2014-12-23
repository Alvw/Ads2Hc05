#include "define.h"
extern uchar rf_tx_in_progress; 
extern uchar rf_data_received;
extern uchar rf_rx_buf[30];
extern uchar rf_rx_data_ready_fg;

void rf_init();
void rf_send(uchar* cmd, uchar length);
void rf_prog_and_bind();
void sendAtCommand(uchar* cmd);
