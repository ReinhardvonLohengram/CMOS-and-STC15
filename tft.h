#ifndef TFT_H
#define TFT_H
#include "def.h"


#define  LCD_DataPortH P2     //高8位数据口
#define  LCD_DataPortL P0     //低8位数据口
sbit LCD_RS = P5^5;  	  //数据/命令切换  （0：读写命令 1：读写数据）
sbit LCD_WR = P4^2;		  //写入数据
sbit LCD_RD =P4^4;		  //读取数据
sbit LCD_CS=P5^4;		  //片选	
//sbit LCD_REST	          //硬件复位   

void Lcd_Init(void);   //初始化

void Lcd_WR_Reg(u16 Data);   //发送命令
void Lcd_Write_Data(u16 Data); //发送数据
void Lcd_WriteReg(u16 com,u16 val); //写寄存器
void Address_set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标范围
u16 LCD_readpoint(u16 x,u16 y);     //读取点颜色

void LCD_scan(u8 mode);

#endif
