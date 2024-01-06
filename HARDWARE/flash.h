
#ifndef __FLASH_H
#define __FLASH_H			    
#include "def.h"

sbit SPI_FLASH_CS =P3^6;       //FLASH片选					 
////////////////////////////////////////////////////////////////////////////
//W25X16读写
#define FLASH_ID 0XEF14						 //W25X16的ID为0XEF14
//指令表
#define W25X_WriteEnable		0x06 		 //写使能
#define W25X_WriteDisable		0x04 		 //写禁能
#define W25X_ReadStatusReg		0x05 		 //读状态
#define W25X_WriteStatusReg		0x01 		 //写状态
#define W25X_ReadData			0x03 		 //写数据
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 		 //页写指令
#define W25X_BlockErase			0xD8 		 //块擦除指令
#define W25X_SectorErase		0x20 		 //扇区擦除指令
#define W25X_ChipErase			0xC7 		 //片擦除指令
#define W25X_PowerDown			0xB9 		 //掉电
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 		 //FLASH ID
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 



u16  SPI_Flash_ReadID(void);  	    //读取FLASH ID

void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //读取flash

void SPI_Flash_Write_NoCheck(u8* buffer,u32 WriteAddr,u16 Byte);   
void SPI_Flash_Erase_Chip(void) ;								   //flash整片擦除
void SPI_Flash_Erase_Sector(u32 Dst_Addr);//扇区擦除
void copy_sdtoflash(const char *path,u16 sector); //要复制的文件名路径  要复制到flash的是从第几扇区开始
#endif
