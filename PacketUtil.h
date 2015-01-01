#include "define.h"

uchar pctAddNewData(long* newData);//data size = 2 ch ADS1292 + 4ch ADC10. Returns 1 if buffer is full
uchar assemblePacket(); //returns packet size (number of bytes)
extern long* packet_buf;
