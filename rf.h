extern unsigned char rf_tx_in_progress; //tx in progress 
extern unsigned char rf_rx_data; //0 - Нет данных, >0 были получены данные
void rf_init();
void rf_send(int *buf, unsigned char length);
