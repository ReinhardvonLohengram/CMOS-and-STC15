
#ifndef __FLASH_H
#define __FLASH_H			    
#include "def.h"

sbit SPI_FLASH_CS =P3^6;       //FLASHƬѡ					 
////////////////////////////////////////////////////////////////////////////
//W25X16��д
#define FLASH_ID 0XEF14						 //W25X16��IDΪ0XEF14
//ָ���
#define W25X_WriteEnable		0x06 		 //дʹ��
#define W25X_WriteDisable		0x04 		 //д����
#define W25X_ReadStatusReg		0x05 		 //��״̬
#define W25X_WriteStatusReg		0x01 		 //д״̬
#define W25X_ReadData			0x03 		 //д����
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 		 //ҳдָ��
#define W25X_BlockErase			0xD8 		 //�����ָ��
#define W25X_SectorErase		0x20 		 //��������ָ��
#define W25X_ChipErase			0xC7 		 //Ƭ����ָ��
#define W25X_PowerDown			0xB9 		 //����
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 		 //FLASH ID
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 



u16  SPI_Flash_ReadID(void);  	    //��ȡFLASH ID

void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //��ȡflash

void SPI_Flash_Write_NoCheck(u8* buffer,u32 WriteAddr,u16 Byte);   
void SPI_Flash_Erase_Chip(void) ;								   //flash��Ƭ����
void SPI_Flash_Erase_Sector(u32 Dst_Addr);//��������
void copy_sdtoflash(const char *path,u16 sector); //Ҫ���Ƶ��ļ���·��  Ҫ���Ƶ�flash���Ǵӵڼ�������ʼ
#endif
