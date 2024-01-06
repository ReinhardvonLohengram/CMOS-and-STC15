
#include "stc15f2k60s2.h"
#include "flash.h" 
#include "spi.h"
#include "delay.h"
#include  "pff.h"
#include  "gui.h"
#include  "tft.h"

extern u8 tbuf[512];

//256bytesΪһҳ
//4KbytesΪһ��Sector ��������
//16������Ϊ1��Block   ���飩
//W25X16
//����Ϊ2M�ֽ�,����32��Block,512��Sector 
//FLASH�ص㣺ÿ��д��ʱ���뱣֤��Χ�ڵ�����ȫΪ0xff Ҳ����FLASHֻ��д0 д1��Ч 
           //����Ƿ�0xff��ô��Ҫ��������д�� ����һ��������4k�ֽ�


//��ȡSPI_FLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
u8 SPI_Flash_ReadSR(void)   
{  
	u8 byte=0;   
	SPI_FLASH_CS=0;                            //ʹ������   
	SPI_SendByte(W25X_ReadStatusReg);          //���Ͷ�ȡ״̬�Ĵ�������    
	byte=SPI_SendByte(0Xff);                   //��ȡһ���ֽ�  
	SPI_FLASH_CS=1;                            //ȡ��Ƭѡ     
	return byte;   
} 




////дSPI_FLASH״̬�Ĵ���
////ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
//void SPI_FLASH_Write_SR(u8 sr)   
//{   
//	SPI_FLASH_CS=0;                            //ʹ������   
//	SPI_SendByte(W25X_WriteStatusReg);   //����дȡ״̬�Ĵ�������    
//	SPI_SendByte(sr);               //д��һ���ֽ�  
//	SPI_FLASH_CS=1;                            //ȡ��Ƭѡ     	      
//}   
//SPI_FLASHдʹ��	
//��WEL��λ   
void SPI_FLASH_Write_Enable(void)   
{
	SPI_FLASH_CS=0;                            //ʹ������   
    SPI_SendByte(W25X_WriteEnable);      //����дʹ��  
	SPI_FLASH_CS=1;                            //ȡ��Ƭѡ     	      
} 			

////SPI_FLASHд��ֹ	
////��WEL����  
//void SPI_FLASH_Write_Disable(void)   
//{  
//	SPI_FLASH_CS=0;                            //ʹ������   
//    SPI_SendByte(W25X_WriteDisable);     //����д��ָֹ��    
//	SPI_FLASH_CS=1;                            //ȡ��Ƭѡ     	      
//} 	
		    
//��ȡоƬID W25X16��ID:0XEF14
u16 SPI_Flash_ReadID(void)
{
	u16 Temp = 0;	  
	SPI_FLASH_CS=0;				    
	SPI_SendByte(0x90);//���Ͷ�ȡID����	    
	SPI_SendByte(0x00); 	    
	SPI_SendByte(0x00); 	    
	SPI_SendByte(0x00); 	 			   
	Temp|=SPI_SendByte(0xFF)<<8;  
	Temp|=SPI_SendByte(0xFF);	 
	SPI_FLASH_CS=1;				    
	return Temp;
}   		    



//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;    												    
	SPI_FLASH_CS=0;                            //ʹ������   
    SPI_SendByte(W25X_ReadData);         //���Ͷ�ȡ����   
    SPI_SendByte((u8)((ReadAddr)>>16));  //����24bit��ַ    
    SPI_SendByte((u8)((ReadAddr)>>8));   
    SPI_SendByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=SPI_SendByte(0XFF);   //ѭ������  
    }
	SPI_FLASH_CS=1;                            //ȡ��Ƭѡ     	      
}  




//�ȴ�����
void SPI_Flash_Wait_Busy(void)   
{   
	while ((SPI_Flash_ReadSR()&0x01)==0x01);   // �ȴ�BUSYλ���
}  



//SPI��һҳ��д������256���ֽڵ�����  (ҳ���ָ��һ�����256���ֽ�)
//��ָ����ַ��ʼд�����256�ֽڵ�����
//buffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//Byte:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void SPI_Flash_Write_Page(u8* buffer,u32 WriteAddr,u16 Byte)
{
 	u16 i;  
    SPI_FLASH_Write_Enable();                  //дʹ��
	SPI_FLASH_CS=0;                            //ʹ������   
    SPI_SendByte(W25X_PageProgram);            //����дҳ����   
    SPI_SendByte((u8)((WriteAddr)>>16));       //����24bit��ַ    
    SPI_SendByte((u8)((WriteAddr)>>8));   
    SPI_SendByte((u8)WriteAddr);   
    for(i=0;i<Byte;i++)SPI_SendByte(buffer[i]);//ѭ��д��  
	SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SPI_Flash_Wait_Busy();					   //�ȴ�д�����
} 



//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//buffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//Byte:Ҫд����ֽ���(���65535)
void SPI_Flash_Write_NoCheck(u8* buffer,u32 WriteAddr,u16 Byte)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���  ҳд���ǰ������ռ���256���ֽ�ƽ�ֵ�	
		 	    				  //�������һ����ַ �ڵ�ǰҳ֮�� ��ôҪ�����ǰҳ����û��д���ֽ��� ��д���ⲿ���ֽ�
	if(Byte<=pageremain)pageremain=Byte;//������256���ֽ�
	while(1)
	{	   
		SPI_Flash_Write_Page(buffer,WriteAddr,pageremain);
		if(Byte==pageremain)break;//д�������
	 	else 
		{
			buffer+=pageremain;
			WriteAddr+=pageremain;	

			Byte-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(Byte>256)pageremain=256;   //һ�ο���д��256���ֽ�
			else pageremain=Byte; 	      //����256���ֽ���
		}
	};	    
} 



//����һ������
//Dst_Addr:������ַ 0~511 for w25x16
//����һ��ɽ��������ʱ��:150ms
void SPI_Flash_Erase_Sector(u32 Dst_Addr)   
{   
	Dst_Addr*=4096;
    SPI_FLASH_Write_Enable();                  //SET WEL 	 
    SPI_Flash_Wait_Busy();   
  	SPI_FLASH_CS=0;                            //ʹ������   
    SPI_SendByte(W25X_SectorErase);      //������������ָ�� 
    SPI_SendByte((u8)((Dst_Addr)>>16));  //����24bit��ַ    
    SPI_SendByte((u8)((Dst_Addr)>>8));   
    SPI_SendByte((u8)Dst_Addr); 
	SPI_FLASH_CS=1;                            //ȡ��Ƭѡ     	      
    SPI_Flash_Wait_Busy();   				   //�ȴ��������
} 




//��SD��������� ���Ƶ�ָ��FLASH����λ��  
//flashΪ2M 512������ һ������4096���ֽ�
//��ȡSD������ ������ FATFS�ļ�ϵͳ�� 
//��ȡ��ʽ ֻ����������ʽ���� flash�� 
//д����ʱ���ܵ�ǰλ����û�����ݶ��������һ��������4k�ֽ� 
//Ȼ���ڽ���д�� ��д��ʱ һ��Ҫ���ǵ�ǰ·���Ƿ�Ϊ�գ��Ƿ����� �мǣ�����
// const char *path   sd���ļ�·��
// sector  Ԥ����flash������λ��
void copy_sdtoflash(const char *path,u16 sector) //Ҫ���Ƶ��ļ���·��  Ҫ���Ƶ�flash���Ǵӵڼ�������ʼ
{
  FRESULT res;
  u16 br,num=0;
  u32 add;

  SPI_Flash_Erase_Sector(sector);	 //������ʼ����

  add=(u32)sector*4096;              //������俪ʼ��ַ

  res=pf_open(path);			    //��ָ���ļ�	����ļ���TF����ָ��·��
  GUI_sprintf_hzstr16x(0,0,"FLASH_w25x16  update...",Blue,White);
  GUI_sprintf_hzstr16x(0,16,"The sector:",Blue,White);
  if(res == FR_OK){
    while(1)
	 {
	  pf_read(tbuf, 512, &br);      //һ�ζ�ȡ512���ֽ�
	  num++;						//��ȡ�����ۼ� һ�ζ�ȡ512���ֽ� һ��FLASHд������� ��4k�ֽ� 
	                                //��Ҫд��һ��������FLASHҪ��ȡ8��512���ֽ�

	  if(num>8)				        //�ж� ����һ������ʱ ������һ������ Ϊд����׼��
	   {
	   	 sector++;				    //�����Լ�
		 SPI_Flash_Erase_Sector(sector);	   //����ָ������ ����һ��Ҫ���µ�����
		 num=1;								   //��������
		 number(88,16,sector,Red,White);	   //��ʾҪ���µ�����
	   }

	  SPI_Flash_Write_NoCheck(tbuf,add,br);	   //��ָ��FLASH��ַ��д������

	  add+=br;								   //��ַ����


                                               //��br����512ʱ ˵����TF��������������Ѿ�����һ��TF��������
											   //�Ǿ����Ѿ������ļ��ײ��� ��������һ���־�������ļ��ĸ���	 
	  if(br!=512) {GUI_sprintf_hzstr16x(0,32,"Update complete��",Black,White); delay1ms(1000); break;}	
	  		   
	                                           
	 }
    
   }

}




//��������оƬ
//��Ƭ����ʱ��:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//�ȴ�ʱ�䳬��...
//void SPI_Flash_Erase_Chip(void)   
//{                                             
//    SPI_FLASH_Write_Enable();                  //SET WEL 
//    SPI_Flash_Wait_Busy();   
//  	SPI_FLASH_CS=0;                            //ʹ������   
//    SPI_SendByte(W25X_ChipErase);        //����Ƭ��������  
//	SPI_FLASH_CS=1;                            //ȡ��Ƭѡ     	      
//	SPI_Flash_Wait_Busy();   				   //�ȴ�оƬ��������
//} 


//ָ������ ���ж�ȡ����
// pBuffer���ݻ����� ���ж�ȡ�����ݴ�������
// sector ָ����������ֵ
// NumByteToRead  Ҫ��ȡ���ֽڸ���
//void SPI_Flash_Sector(u8* pBuffer,u16 sector,u16 NumByteToRead)   
//{ 
// 	u16 i;
//	u32 ReadAddr;
//	ReadAddr=(u32)sector*4096;    												    
//	SPI_FLASH_CS=0;                            //ʹ������   
//    SPI_SendByte(W25X_ReadData);         //���Ͷ�ȡ����   
//    SPI_SendByte((u8)((ReadAddr)>>16));  //����24bit��ַ    
//    SPI_SendByte((u8)((ReadAddr)>>8));   
//    SPI_SendByte((u8)ReadAddr);   
//    for(i=0;i<NumByteToRead;i++)
//	{ 
//        pBuffer[i]=SPI_SendByte(0XFF);   //ѭ������  
//    }
//	SPI_FLASH_CS=1;                            //ȡ��Ƭѡ     	      
//}  








