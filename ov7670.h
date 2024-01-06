#ifndef _OV7670_H_
#define _OV7670_H_
#include "def.h"


#define OV7670_REG_NUM  114

 //===========FIFO PIN============
sbit FIFO_WEN =P3^3;		   //д��FIFOʹ��
sbit FIFO_RCLK=P4^5;		   //������ʱ��
sbit FIFO_WRST=P3^4;		   //дָ�븴λ
sbit FIFO_RRST=P3^5;		   //��ָ�븴λ
sbit FIFO_OE  =P3^6;		   //Ƭѡ�ź�(OE)


extern u8 cur_status;						 //֡��־λ  ��interrupt.c�����е���


u8 wr_Sensor_Reg(u8 regID, u8 regDat);
u8 rd_Sensor_Reg(u8 regID, u8 *regDat);

u8 Ov7670_init_normal(void);						 //OV7670��ʼ��
u8 Ov7670_init_fupian(void);
u8 Ov7670_init_fanzhuan(void);
void OV7670_Window_Set(unsigned int sx,unsigned int sy,unsigned int width,unsigned int height);	   //ov7670��������

#endif



