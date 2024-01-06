
#include "stc15f2k60s2.h"
#include "sd.h"
#include "spi.h"
#include "delay.h"



u8  SD_Type=0;//SD卡的类型 


///////////////////////////////////////////////////////////////////////////////////


//等待卡准备好
//返回值:0,准备好了;其他,错误代码
u8 SD_WaitReady(void)
{
	u32 t=0;
	do
	{
		if(SPI_SendByte(0XFF)==0XFF)return 0;//OK
		t++;		  	
	}while(t<0XFFFFFF);//等待 
	return 1;
}

//取消选择,释放SPI总线
void SD_DisSelect(void)
{
	SD_CS=1;
 	SPI_SendByte(0xff);//提供额外的8个时钟
}

//选择sd卡,并且等待卡准备OK
//返回值:0,成功;1,失败;
u8 SD_Select(void)
{
	SD_CS=0;
	if(SD_WaitReady()==0)return 0;//等待成功
	SD_DisSelect();
	return 1;//等待失败
}

//向SD卡发送一个命令
//输入: u8 cmd   命令 
//      u32 arg  命令参数
//      u8 crc   crc校验值	   
//返回值:SD卡返回的响应															  
u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
    u8 r1;	
	u8 Retry=0; 

	SD_DisSelect();//取消上次片选
	if(SD_Select())return 0XFF;//片选失效 


	//发送
	SD_CS=1;		                                //片选拉高																		 
	SPI_SendByte(0xff);
	SPI_SendByte(0xff);	 
	SPI_SendByte(0xff);							    
	SD_CS=0; 

    SPI_SendByte(cmd | 0x40);       //分别写入命令	最高2位固定为1  
    SPI_SendByte(arg >> 24);		//命令参数 2-5字节  4个字节 32位
    SPI_SendByte(arg >> 16);
    SPI_SendByte(arg >> 8);
    SPI_SendByte(arg);	  
    SPI_SendByte(crc); 
	                                 //停止传数据命令
	if(cmd==CMD12)SPI_SendByte(0xff);//Skip a stuff byte when stop reading
    //等待响应，或超时退出
	Retry=0X1F;		   //循环32次
	do
	{
		r1=SPI_SendByte(0xFF);
	}while((r1&0X80) && Retry--);	 
	//返回状态值
    return r1;
}







//初始化SD卡
u8 SD_Init(void)
{
    u8 r1;      // 存放SD卡的返回值
    u16 retry;  // 用来进行超时计数
    u8 buf[4];  
	u8 i;
	
 	SPI_Speed(3);		//设置到低速模式 

 	for(i=0;i<10;i++)SPI_SendByte(0XFF);//发送最少74个脉冲 等待SD卡内部供电电压上升时间	  进入SPI模式

	
	retry=20;
	do
	{
		r1=SD_SendCmd(CMD0,0,0x95);//进入IDLE状态 	 即复位sd卡 空闲状态
	}while((r1!=0X01) && retry--);

	 
 	SD_Type=0;//默认无卡
	 
	if(r1==0X01)   //获取版本信息
	{	
		if(SD_SendCmd(CMD8,0x1AA,0x87)==1)//SD V2.0	  发送接口状态命令		                                  
		{							      //如果返回值为1 则是 SDV2.0版本
			for(i=0;i<4;i++)buf[i]=SPI_SendByte(0XFF);	//Get trailing return value of R7 resp
														//提取返回值R7数据
			if(buf[2]==0X01&&buf[3]==0XAA)//卡是否支持2.7~3.6V
			{  
				retry=0XFFFE;
				do
				{
					SD_SendCmd(CMD55,0,0X01);	//发送CMD55
					r1=SD_SendCmd(CMD41,0x40000000,0X01);//发送CMD41
				}while(r1&&retry--);
				if(retry&&SD_SendCmd(CMD58,0,0X01)==0)//鉴别SD2.0卡版本开始
				{	
					for(i=0;i<4;i++)buf[i]=SPI_SendByte(0XFF);//得到OCR值
					if(buf[0]&0x40)SD_Type=SD_TYPE_V2HC;    //检查CCS
					else SD_Type=SD_TYPE_V2;  					     						
				}
				else SD_Type=SD_TYPE_ERR;//错误的卡
			}
		}
	
	}

	SD_DisSelect();//取消片选
	SPI_Speed(0);//高速	
	if(SD_Type)return 0;	//如果没有采集到SD卡版本 则跳出函数
	else if(r1)return r1; 	   
	return 0xaa;//其他错误
}




//在指定扇区读SD卡一扇区数据 即512字节
//buf:数据缓存区
//sector:扇区
//返回值:0,ok;其他,失败. 
u8 SD_read_sector(u32 sector,u8 *buf) 
{ 
    u16 i;
	u8 r1;
	SD_DisSelect();//取消上次片选
 	if(SD_Select())return 0XFF;//片选失效 

    if(SD_Type!=SD_TYPE_V2HC)sector <<= 9;//转换为字节地址


    r1=SD_SendCmd(CMD17,sector,0X01);//读命令
		if(r1==0)//指令发送成功
		{
		    while(SPI_SendByte(0xff)!=0xfe);
			 for(i=0;i<512;i++) 
			    {
			   //连续读出一扇区数据 
			    buf[i]=SPI_SendByte(0xff);  
			    }
			 //下面是2个伪CRC（dummy CRC）
			  SPI_SendByte(0xff); 
              SPI_SendByte(0xff); 		   
		}


	SD_DisSelect();//取消片选
	return r1;//

} 


////写SD卡写入512字节数据
////buf:数据缓存区
////sector:起始扇区
////返回值:0,ok;其他,失败.
//u8 SD_WriteDisk(u8*buf,u32 sector)
//{
//	u8 r1;
//	u16 t;
//    sector *= 512;//转换为字节地址
//
//	r1=SD_SendCmd(CMD24,sector,0X01);//读命令
//	if(r1==0)//指令发送成功
//	{
//
//     if(SD_WaitReady()){SD_DisSelect();return 1;}//等待准备失效
//	 SPI_SendByte(0xFE); //发开启符
//
//	 for(t=0;t<512;t++)SPI_SendByte(buf[t]);//写字节
//	 SPI_SendByte(0xFF);    //忽略crc
//	 SPI_SendByte(0xFF);
//	 t=SPI_SendByte(0xFF);  //接收响应
//	 if((t&0x1F)!=0x05)r1=2;//响应错误									  					    
//	 	 	   
//	}
//
//
//	SD_DisSelect();//取消片选
//	return r1;
//}



//在指定扇区写SD卡一扇区数据 即512字节
//buf:数据缓存区
//sector:起始扇区
//cnt:扇区数
//返回值:0,ok;其他,失败.
//u8 SD_write_sector(u32 sector,u8*buf)
//{
//	u8 r1;
//	u16 t;
//	u32 m=0;
//	if(SD_Type!=SD_TYPE_V2HC)sector *= 512;//转换为字节地址
//
//	 r1=SD_SendCmd(CMD24,sector,0X01);//读命令
//		if(r1==0)//指令发送成功
//		{
//			//r1=SD_SendBlock(buf,0xFE);//写512个字节	 
//			 //等待卡准备就绪	
//	          do
//	          {
//		        if(SPI_SendByte(0XFF)==0XFF)break;//OK
//		          m++;		  	
//	           }while(m<0XFFFFFF);//等待 
//
//	          SPI_SendByte(0XFE);
//			  
//				for(t=0;t<512;t++)SPI_SendByte(buf[t]);//提高速度,减少函数传参时间
//			    SPI_SendByte(0xFF);//忽略crc
//			    SPI_SendByte(0xFF);
//				t=SPI_SendByte(0xFF);//接收响应
//				if((t&0x1F)!=0x05)return 2;//响应错误									  					    
//		}
//	SD_DisSelect();//取消片选
//	return 0;  //写入成功
//
//					
//}



//在指定地址 读取 指定字节数
//addr:地址
//buf:数据缓存区
//Bytes：要读取的字节数
//返回值:0,ok;其他,失败.   
u8 SD_read_Byte(u32 addr,u8 *buf,u32 Bytes) 
{ 
    u16 i;
    u8 temp=0,retry;

    // 命令16设置块长度 
    SD_SendCmd(16,Bytes,0xff); 

    retry = 0;

    do
    {
    // 写入CMD17 	读取命令
    temp=SD_SendCmd(17,addr,0xff);
    retry++;
    }
		
	while((temp!=0x00) && (retry < TRY_TIME)); 

	
    if (retry >= TRY_TIME) 
    {
    return READ_BLOCK_ERROR; 
    }

    // 读到0xfe报头  
    while(SPI_SendByte(0xff)!=0xfe);

     for(i=0;i<Bytes;i++) 
    {

        //连续读出指定 长度字节数据到buf
        buf[i] = SPI_SendByte(0xff);  
    }

   //下面是2个伪CRC（dummy CRC）
    SPI_SendByte(0xff); 
    SPI_SendByte(0xff);  


    //* 命令16恢复块长度 为512个字节
    SD_SendCmd(16,512,0xff);
	SD_DisSelect();//取消片选
    
    return 0; 

} 


