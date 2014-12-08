#include "define.h"

extern uchar rf_tx_in_progress; //tx in progress 
void rf_init();
void rf_send(uchar* cmd, uchar length);
void rf_prog_and_bind();
