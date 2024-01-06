#ifndef _SCCB_H_
#define _SCCB_H_
#include "def.h"


sbit SCCB_SIC=P3^0;		  //Ê±ÖÓ
sbit SCCB_SID=P3^1;		  //Êý¾Ý


#define DELAYTIME 200

void startSCCB(void);
void stopSCCB(void);
void noAck(void);
unsigned char getAck();
unsigned char SCCBwriteByte(unsigned char dat);
unsigned char SCCBreadByte(void);

#endif


