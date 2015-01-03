#include "define.h"

uchar packetAddNewData(long* newData);//data size = 2 ch ADS1292 + 4ch ADC10. Returns 1 if buffer is full
uchar assemblePacket(); //returns packet size (number of bytes)
extern long* packet_buf;
extern uchar loffStatEnable;//0 - disable, 1 - enable
extern uchar loffStat;//loff stat value;
void setAccelerometerMode(uchar mode);//0 - disable, 1 - 1channel mode, 3 - 3 channel mode;
 
