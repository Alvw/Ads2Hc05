#include "PacketUtil.h"
#include "string.h"

enum {
	PCT_BUFF_MAX_SIZE = 26, //1 header + 10 * 2 ch ADS data + 3ch Accelerometer data + 1ch acb voltage data + 1 loff&system
        NUMBER_OF_CHANNELS = 6, //2ch ADS, 3ch accelerometer, 1ch battery
        MAX_DIV = 10
}; 
long buf1[PCT_BUFF_MAX_SIZE];
long buf2[PCT_BUFF_MAX_SIZE];
uint packet_cntr = 0;
long* add_buf = &buf1[0];
long* packet_buf = &buf2[0];
uchar div[NUMBER_OF_CHANNELS] ={1,1,10,10,10,10}; // frequency dividers for each channel. div = 0 if channel is not active.
uchar buffCounter[NUMBER_OF_CHANNELS]; // buffCounter[i]_max = ( MAX_DIV / div[i] )  - number of "long" to buffer data from channel i
uchar sumCounter[NUMBER_OF_CHANNELS]; // sumCounter[i]_max = div[i] - how many times we sum input data from channel i to have average output data

uchar pctAddNewData(long* newData) {
  uchar isAccumulatingFinished = 0;
  uchar j = 0;
  for (uchar i = 0; i < NUMBER_OF_CHANNELS; i++) {
    if (div[i] != 0) {
      if(sumCounter[i] == 0){
        add_buf[1 + j + buffCounter[i]] = 0;
      }
      add_buf[1 + j + buffCounter[i]] += newData[i];
      sumCounter[i]++;
      if (sumCounter[i] == div[i]) {
        buffCounter[i]++;
        if ((sumCounter[i] * buffCounter[i]) == MAX_DIV) {
          isAccumulatingFinished = 1;
          buffCounter[i] = 0;
        }
        sumCounter[i] = 0;
      }
      j += (MAX_DIV / div[i]);      // j += buffCounter[i]_max
    }
  }
  if(isAccumulatingFinished){
    //flip buffers
    long* pBuf = add_buf;
    add_buf = packet_buf;
    packet_buf = pBuf;
  }
  return isAccumulatingFinished;
} 

uchar assemblePacket(){
  uint* intBuff = (uint *) &packet_buf[0];
  intBuff[0] = 0xAAAA;
  intBuff[1] = packet_cntr++; 
  uchar* packetCharBuff = (uchar *) &packet_buf[1];
  uchar charIndex = 0;
  uchar longIndex = 0;
  //Add ADS1292 data
  for (uchar i = 0; i < 2; i++) {// 2 channels
    if (div[i] != 0) {
      uchar numberOfSamplesInChannel = (MAX_DIV/div[i]);
      for(uchar j = 0; j < numberOfSamplesInChannel; j++){
        packet_buf[longIndex + 1]/= div[i];
        packetCharBuff[charIndex++] = packetCharBuff[longIndex*4];
        packetCharBuff[charIndex++] = packetCharBuff[longIndex*4 + 1];
        packetCharBuff[charIndex++] = packetCharBuff[longIndex*4 + 2];
        longIndex++;
      }
    }
  }
  //Add ADC10 data
  for (uchar i = 2; i < 6; i++) {// 4 channels
    if (div[i] != 0) {
      uchar numberOfSamplesInChannel = (MAX_DIV/div[i]);
      for(uchar j = 0; j < numberOfSamplesInChannel; j++){
        packetCharBuff[charIndex++] = packetCharBuff[longIndex*4];
        packetCharBuff[charIndex++] = packetCharBuff[longIndex*4 + 1];
        longIndex++;
      }
    }
  }
  //Add footer value 0x55
  packetCharBuff[charIndex++] = 0x55;
  return 4 + charIndex ;
}














