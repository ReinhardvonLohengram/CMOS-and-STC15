
#ifndef  SPI_H
#define  SPI_H

#include "def.h"


void Init_SPI();		             //SPI初始化
u8 SPI_SendByte(u8 SPI_SendData);	 //SPI为全双工通讯
void SPI_Speed(u8 speed);			 //SPI速度调节
#endif




















