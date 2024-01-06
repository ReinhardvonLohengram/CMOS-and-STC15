 
 
#ifndef XPT2046_H
#define XPT2046_H
#include "def.h"


sbit AD7843_CS=P4^4;         //片选	 
sbit AD7843_isClick=P3^7;    //判断触摸屏是否按下，返回值等于1时按下



struct TFT_Pointer			  //定义三个变量的结构体函数  主要应用于 对采集的AD值 X Y  和是否有触摸屏被激发的标志位flag
{
 u16 x;
 u16 y;
 u8 flag;
};


struct T_i
{
 short xi;          //x 比例因子
 short yi;          //y 比例因子
 short a;		  //x 偏移量
 short b;	   	  //y 偏移量
};

struct TFT_Pointer TFT_Cm();     //TFT 触摸屏函数





#endif
