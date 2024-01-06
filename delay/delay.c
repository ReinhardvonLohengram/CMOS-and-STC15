

#include "stc15f2k60s2.h"
#include "delay.h"

/*
24MHz主频下 
在示波器下看
一个语句的时间大概是 0.8us
*/

//2微秒	延时
//此函数是以2us的倍数增加
//t  2us的倍数
void delay2us(u16 us)
 {
  u16 i;
  u8 m;
  for(i=0;i<us;i++)
  for(m=0;m<3;m++);
 }



//4微秒	延时
//此函数是以4us的倍数增加
//t  4us的倍数
void delay4us(u16 us)
 {
  u16 i;
  u8 m;
  for(i=0;i<us;i++)
  for(m=0;m<5;m++);
 }  



//1毫秒 延时
//ms 1ms的倍数
void delay1ms(u16 ms)
{
 u16 m;
 for(m=0;m<ms;m++)
 delay4us(250);
} 
  


void delaylow(u8 time)		   //红外专用 延时
{
 u8 i,j;
 for(i=0;i<time;i++)
  for(j=0;j<100;j++);
}



