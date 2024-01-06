#ifndef _OV7670_H_
#define _OV7670_H_
#include "def.h"


#define OV7670_REG_NUM  114

 //===========FIFO PIN============
sbit FIFO_WEN =P3^3;		   //写入FIFO使能
sbit FIFO_RCLK=P4^5;		   //读数据时钟
sbit FIFO_WRST=P3^4;		   //写指针复位
sbit FIFO_RRST=P3^5;		   //读指针复位
sbit FIFO_OE  =P3^6;		   //片选信号(OE)


extern u8 cur_status;						 //帧标志位  在interrupt.c函数中调用


u8 wr_Sensor_Reg(u8 regID, u8 regDat);
u8 rd_Sensor_Reg(u8 regID, u8 *regDat);

u8 Ov7670_init_normal(void);						 //OV7670初始化
u8 Ov7670_init_fupian(void);
u8 Ov7670_init_fanzhuan(void);
void OV7670_Window_Set(unsigned int sx,unsigned int sy,unsigned int width,unsigned int height);	   //ov7670窗体设置

#endif



