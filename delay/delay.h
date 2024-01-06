#ifndef DELAY_H
#define EDLAY_H
#include  "def.h"


void delaylow(u8 time);	 //红外专用 延时
void delay2us(u16 us);   //2us倍数延时
void delay4us(u16 us);   //4us倍数延时
void delay1ms(u16 ms);   //毫秒延时
#endif
