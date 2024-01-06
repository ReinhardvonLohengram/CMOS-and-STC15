
#include "stc15f2k60s2.h"
#include "flash.h" 
#include "spi.h"
#include "delay.h"
#include  "pff.h"
#include  "gui.h"
#include  "tft.h"

extern u8 tbuf[512];

//256bytes为一页
//4Kbytes为一个Sector （扇区）
//16个扇区为1个Block   （块）
//W25X16
//容量为2M字节,共有32个Block,512个Sector 
//FLASH特点：每次写入时必须保证范围内的数据全为0xff 也就是FLASH只能写0 写1无效 
           //如果是非0xff那么就要擦除后在写入 擦除一次最少是4k字节


//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8 SPI_Flash_ReadSR(void)   
{  
	u8 byte=0;   
	SPI_FLASH_CS=0;                            //使能器件   
	SPI_SendByte(W25X_ReadStatusReg);          //发送读取状态寄存器命令    
	byte=SPI_SendByte(0Xff);                   //读取一个字节  
	SPI_FLASH_CS=1;                            //取消片选     
	return byte;   
} 




////写SPI_FLASH状态寄存器
////只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
//void SPI_FLASH_Write_SR(u8 sr)   
//{   
//	SPI_FLASH_CS=0;                            //使能器件   
//	SPI_SendByte(W25X_WriteStatusReg);   //发送写取状态寄存器命令    
//	SPI_SendByte(sr);               //写入一个字节  
//	SPI_FLASH_CS=1;                            //取消片选     	      
//}   
//SPI_FLASH写使能	
//将WEL置位   
void SPI_FLASH_Write_Enable(void)   
{
	SPI_FLASH_CS=0;                            //使能器件   
    SPI_SendByte(W25X_WriteEnable);      //发送写使能  
	SPI_FLASH_CS=1;                            //取消片选     	      
} 			

////SPI_FLASH写禁止	
////将WEL清零  
//void SPI_FLASH_Write_Disable(void)   
//{  
//	SPI_FLASH_CS=0;                            //使能器件   
//    SPI_SendByte(W25X_WriteDisable);     //发送写禁止指令    
//	SPI_FLASH_CS=1;                            //取消片选     	      
//} 	
		    
//读取芯片ID W25X16的ID:0XEF14
u16 SPI_Flash_ReadID(void)
{
	u16 Temp = 0;	  
	SPI_FLASH_CS=0;				    
	SPI_SendByte(0x90);//发送读取ID命令	    
	SPI_SendByte(0x00); 	    
	SPI_SendByte(0x00); 	    
	SPI_SendByte(0x00); 	 			   
	Temp|=SPI_SendByte(0xFF)<<8;  
	Temp|=SPI_SendByte(0xFF);	 
	SPI_FLASH_CS=1;				    
	return Temp;
}   		    



//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;    												    
	SPI_FLASH_CS=0;                            //使能器件   
    SPI_SendByte(W25X_ReadData);         //发送读取命令   
    SPI_SendByte((u8)((ReadAddr)>>16));  //发送24bit地址    
    SPI_SendByte((u8)((ReadAddr)>>8));   
    SPI_SendByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=SPI_SendByte(0XFF);   //循环读数  
    }
	SPI_FLASH_CS=1;                            //取消片选     	      
}  




//等待空闲
void SPI_Flash_Wait_Busy(void)   
{   
	while ((SPI_Flash_ReadSR()&0x01)==0x01);   // 等待BUSY位清空
}  



//SPI在一页内写入少于256个字节的数据  (页编程指令一次最多256个字节)
//在指定地址开始写入最大256字节的数据
//buffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//Byte:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void SPI_Flash_Write_Page(u8* buffer,u32 WriteAddr,u16 Byte)
{
 	u16 i;  
    SPI_FLASH_Write_Enable();                  //写使能
	SPI_FLASH_CS=0;                            //使能器件   
    SPI_SendByte(W25X_PageProgram);            //发送写页命令   
    SPI_SendByte((u8)((WriteAddr)>>16));       //发送24bit地址    
    SPI_SendByte((u8)((WriteAddr)>>8));   
    SPI_SendByte((u8)WriteAddr);   
    for(i=0;i<Byte;i++)SPI_SendByte(buffer[i]);//循环写数  
	SPI_FLASH_CS=1;                            //取消片选 
	SPI_Flash_Wait_Busy();					   //等待写入结束
} 



//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//buffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//Byte:要写入的字节数(最大65535)
void SPI_Flash_Write_NoCheck(u8* buffer,u32 WriteAddr,u16 Byte)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数  页写入是把整个空间用256个字节平分的	
		 	    				  //如果给出一个地址 在当前页之中 那么要算出当前页后面没有写的字节数 先写入这部分字节
	if(Byte<=pageremain)pageremain=Byte;//不大于256个字节
	while(1)
	{	   
		SPI_Flash_Write_Page(buffer,WriteAddr,pageremain);
		if(Byte==pageremain)break;//写入结束了
	 	else 
		{
			buffer+=pageremain;
			WriteAddr+=pageremain;	

			Byte-=pageremain;			  //减去已经写入了的字节数
			if(Byte>256)pageremain=256;   //一次可以写入256个字节
			else pageremain=Byte; 	      //不够256个字节了
		}
	};	    
} 



//擦除一个扇区
//Dst_Addr:扇区地址 0~511 for w25x16
//擦除一个山区的最少时间:150ms
void SPI_Flash_Erase_Sector(u32 Dst_Addr)   
{   
	Dst_Addr*=4096;
    SPI_FLASH_Write_Enable();                  //SET WEL 	 
    SPI_Flash_Wait_Busy();   
  	SPI_FLASH_CS=0;                            //使能器件   
    SPI_SendByte(W25X_SectorErase);      //发送扇区擦除指令 
    SPI_SendByte((u8)((Dst_Addr)>>16));  //发送24bit地址    
    SPI_SendByte((u8)((Dst_Addr)>>8));   
    SPI_SendByte((u8)Dst_Addr); 
	SPI_FLASH_CS=1;                            //取消片选     	      
    SPI_Flash_Wait_Busy();   				   //等待擦除完成
} 




//将SD卡里的数据 复制到指定FLASH扇区位置  
//flash为2M 512个扇区 一个扇区4096个字节
//读取SD卡数据 建立在 FATFS文件系统上 
//存取方式 只能以扇区形式存入 flash中 
//写数据时不管当前位置有没有数据都会先清除一个扇区即4k字节 
//然后在进行写入 （写入时 一定要考虑当前路径是否为空，是否有用 切记！！）
// const char *path   sd卡文件路径
// sector  预存入flash的扇区位置
void copy_sdtoflash(const char *path,u16 sector) //要复制的文件名路径  要复制到flash的是从第几扇区开始
{
  FRESULT res;
  u16 br,num=0;
  u32 add;

  SPI_Flash_Erase_Sector(sector);	 //擦除开始扇区

  add=(u32)sector*4096;              //计算出其开始地址

  res=pf_open(path);			    //打开指定文件	这个文件是TF卡中指定路径
  GUI_sprintf_hzstr16x(0,0,"FLASH_w25x16  update...",Blue,White);
  GUI_sprintf_hzstr16x(0,16,"The sector:",Blue,White);
  if(res == FR_OK){
    while(1)
	 {
	  pf_read(tbuf, 512, &br);      //一次读取512个字节
	  num++;						//读取次数累加 一次读取512个字节 一个FLASH写入的扇区 是4k字节 
	                                //即要写入一个扇区的FLASH要读取8次512个字节

	  if(num>8)				        //判断 超过一个扇区时 擦除下一个扇区 为写入做准备
	   {
	   	 sector++;				    //扇区自加
		 SPI_Flash_Erase_Sector(sector);	   //擦除指定扇区 即下一个要更新的扇区
		 num=1;								   //清计算变量
		 number(88,16,sector,Red,White);	   //显示要更新的扇区
	   }

	  SPI_Flash_Write_NoCheck(tbuf,add,br);	   //在指定FLASH地址内写入数据

	  add+=br;								   //地址增加


                                               //当br不是512时 说明从TF卡里读出的数据已经不足一个TF卡扇区了
											   //那就是已经到达文件底部了 更新完这一部分就完成了文件的复制	 
	  if(br!=512) {GUI_sprintf_hzstr16x(0,32,"Update complete！",Black,White); delay1ms(1000); break;}	
	  		   
	                                           
	 }
    
   }

}




//擦除整个芯片
//整片擦除时间:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//等待时间超长...
//void SPI_Flash_Erase_Chip(void)   
//{                                             
//    SPI_FLASH_Write_Enable();                  //SET WEL 
//    SPI_Flash_Wait_Busy();   
//  	SPI_FLASH_CS=0;                            //使能器件   
//    SPI_SendByte(W25X_ChipErase);        //发送片擦除命令  
//	SPI_FLASH_CS=1;                            //取消片选     	      
//	SPI_Flash_Wait_Busy();   				   //等待芯片擦除结束
//} 


//指定扇区 进行读取数据
// pBuffer数据缓存区 所有读取的数据存入其中
// sector 指定的扇区数值
// NumByteToRead  要读取的字节个数
//void SPI_Flash_Sector(u8* pBuffer,u16 sector,u16 NumByteToRead)   
//{ 
// 	u16 i;
//	u32 ReadAddr;
//	ReadAddr=(u32)sector*4096;    												    
//	SPI_FLASH_CS=0;                            //使能器件   
//    SPI_SendByte(W25X_ReadData);         //发送读取命令   
//    SPI_SendByte((u8)((ReadAddr)>>16));  //发送24bit地址    
//    SPI_SendByte((u8)((ReadAddr)>>8));   
//    SPI_SendByte((u8)ReadAddr);   
//    for(i=0;i<NumByteToRead;i++)
//	{ 
//        pBuffer[i]=SPI_SendByte(0XFF);   //循环读数  
//    }
//	SPI_FLASH_CS=1;                            //取消片选     	      
//}  








