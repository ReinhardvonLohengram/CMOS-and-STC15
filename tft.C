


#include "stc15f2k60s2.h"
#include "tft.h"
#include "delay.h"
#include "def.h"




//TFT数据及命令函数
// u8 VH,u8 VL  写入的数据 是16位 用两个8位表示  正好符合P0 P2口所对应
void LCD_Writ_Bus(u8 VH,u8 VL)   //并行数据写入函数
{
    LCD_DataPortH=VH;	
	LCD_DataPortL=VL;		
	LCD_WR=0;
	LCD_WR=1; 
}

//发送命令
void Lcd_WR_Reg(u16 Data)	 
{	
    LCD_RS=0;
	LCD_Writ_Bus(Data>>8,Data);
}

//发送数据
void Lcd_Write_Data(u16 Data) 
{
    LCD_RS=1;
	LCD_Writ_Bus(Data>>8,Data);
}



//写寄存器
//u16 com 要写入的命令 u16 val要写入的数据
void Lcd_WriteReg(u16 com,u16 val)		   
{
	Lcd_WR_Reg(com);
    Lcd_Write_Data(val);
}




//设置坐标范围		
//x1 y1  初始位置
//x2 y2  结束位置
void Address_set(u16 x1,u16 y1,u16 x2,u16 y2)
{	     
      //  xq=0;yq=0;
		Lcd_WR_Reg(0X2A); 
		Lcd_Write_Data(x1>>8); 
		Lcd_Write_Data(x1&0XFF);
		Lcd_Write_Data(x2>>8); 
		Lcd_Write_Data(x2&0XFF);

		Lcd_WR_Reg(0X2B); 
		Lcd_Write_Data(y1>>8); 
		Lcd_Write_Data(y1&0XFF);
		Lcd_Write_Data(y2>>8); 
		Lcd_Write_Data(y2&0XFF);
	    Lcd_WR_Reg(0X2C);      //开始写入GRAM
 
}


//void LCD_SetCursor(u16 Xpos, u16 Ypos)
//{	 
//		    
//		Lcd_WR_Reg(0X2A); 
//		Lcd_Write_Data(Xpos>>8); 
//		Lcd_Write_Data(Xpos&0XFF);	 
//		Lcd_WR_Reg(0X2B); 
//		Lcd_Write_Data(Ypos>>8); 
//		Lcd_Write_Data(Ypos&0XFF);
//	 
//} 
//
//
//void LCD_Clear(u16 color)
//{
//	u16 i,j;      
//	LCD_SetCursor(0x0000,0x0000);	//设置光标位置 
//	Lcd_WR_Reg(0X2C);      //开始写入GRAM
////	LCD_RS=1;	 	    
//    for(i=0;i<320;i++)
//	 {
//	  for (j=0;j<240;j++)
//	   	{
////        	P2=color>>8;	
////	        P0=color;		
////	        LCD_WR=0;
////      	    LCD_WR=1;  
//			Lcd_Write_Data(color);
//	    }
//
//	  }		
//
//
//} 


////画点
////x,y:坐标
////POINT_COLOR:此点的颜色
//void LCD_DrawPoint(u16 x,u16 y,u16 color)
//{
//	LCD_SetCursor(x,y);		//设置光标位置 
//	Lcd_WR_Reg(0X2C);      //开始写入GRAM
//	Lcd_Write_Data(color);
//}




////从ILI93xx读出的数据为BGR格式，而我们写入的时候为RGB格式。	 （实验证明手册里并没有说明）
////通过该函数转换
////c:BGR格式的颜色值
////返回值：RGB格式的颜色值
//u16 LCD_bgr2rgb(u16 c)
//{
//	u16  r,g,b,rgb;   
//	b=(c>>0)&0x1f;
//	g=(c>>5)&0x3f;
//	r=(c>>11)&0x1f;	 
//	rgb=(b<<11)+(g<<5)+(r<<0);		 
//	return(rgb);
//} 


//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
u16 LCD_readpoint(u16 x,u16 y)
{
	u16 color;
	u8 r=0,g=0,b=0;

//  	Lcd_WR_Reg(0x0020);		  //设置读颜色坐标
//	Lcd_Write_Data(x);	  
//    Lcd_WR_Reg(0x0021);
//	Lcd_Write_Data(y); 

	Lcd_WR_Reg(0X2A); 
	Lcd_Write_Data(x>>8); 
	Lcd_Write_Data(x&0XFF);	 
	Lcd_Write_Data(x>>8); 
	Lcd_Write_Data(x&0XFF);	

	Lcd_WR_Reg(0X2B); 
	Lcd_Write_Data(y>>8); 
    Lcd_Write_Data(y&0XFF);
	Lcd_Write_Data(y>>8); 
    Lcd_Write_Data(y&0XFF);


	Lcd_WR_Reg(0x2e);		           //发送读颜色命令

	P2M1=0XFF;						  //P0 P2口设置为仅输入状态 等待接收颜色数据
	P2M0=0X00;
	P0M1=0XFF;
	P0M0=0X00;

	LCD_RS=1;						  //1为读数据
	//读取数据(读GRAM时,需要读2次)
	LCD_RD=0;					   	  //进行读数据
 	LCD_RD=1;
 	//dummy READ					  //第一次接收为虚拟数据  手册69页 有详解
	delay2us(2);
	LCD_RD=0;					   
 	LCD_RD=1;
	r=P2;
	g=P0;
	delay2us(2);
	LCD_RD=0;					   
 	LCD_RD=1;
	b=P2;
		
//	color=P2;
//	color=(color<<8)|P0;	          //第二次读取接收颜色数据

    P2M1=0X00;						  //P2 P0配置为双向IO口  （恢复状态）
	P2M0=0X00;
	P0M1=0X00;
	P0M0=0X00;

	color=(((u16)r>>3)<<11)|(((u16)g>>2)<<5)|((u16)b>>3);
	return color;
//	return LCD_bgr2rgb(color);		  //对读取的颜色进行 BGR转RGB	  
}



/*
9341扫描特点 如果扫描方向改变后 会直接改变x y起始原点
所以在横屏显示的时候 就不用考虑 x y的变化后的结果 因为原点也发生改变
就是可以一种代码通用

*/

//tft屏设置扫描方式函数
//mode 1 -8  一共8种扫描方式
//     1   左>右  上>下 
//     2   左>右  下>上
//     3   右>左  上>下
//     4   右>左  下>上 
//     5   上>下  左>右  
//     6   上>下  右>左 
//     7   下>上  左>右 
//     8   下>上  右>左 
void LCD_scan(u8 mode)
{
 switch(mode)
 {
   case 1:
	Lcd_WR_Reg(0x36);    // Memory Access Control 	   saomiao
	Lcd_Write_Data(0x08); 	
    Lcd_WR_Reg(0X2C);      //开始写入GRAM			 
	break;
   case 2:
	Lcd_WR_Reg(0x36);    // Memory Access Control 	   saomiao
	Lcd_Write_Data(0x88); 
	Lcd_WR_Reg(0X2C);      //开始写入GRAM					 
	break;
   case 3:
	Lcd_WR_Reg(0x36);    // Memory Access Control 	   saomiao
	Lcd_Write_Data(0x48); 	
	Lcd_WR_Reg(0X2C);      //开始写入GRAM				
	break;
   case 4:
	Lcd_WR_Reg(0x36);    // Memory Access Control 	   saomiao
	Lcd_Write_Data(0xc8); 	
	Lcd_WR_Reg(0X2C);      //开始写入GRAM				 
	break;
   case 5:
	Lcd_WR_Reg(0x36);    // Memory Access Control 	   saomiao
	Lcd_Write_Data(0x28); 
	Lcd_WR_Reg(0X2C);      //开始写入GRAM					 
	break;
   case 6:
	Lcd_WR_Reg(0x36);    // Memory Access Control 	   saomiao
	Lcd_Write_Data(0x68); 	
	Lcd_WR_Reg(0X2C);      //开始写入GRAM				 
	break;
   case 7:
	Lcd_WR_Reg(0x36);    // Memory Access Control 	   saomiao
	Lcd_Write_Data(0xa8); 	
	Lcd_WR_Reg(0X2C);      //开始写入GRAM				 
	break;
   case 8:
	Lcd_WR_Reg(0x36);    // Memory Access Control 	   saomiao
	Lcd_Write_Data(0xe8); 
	Lcd_WR_Reg(0X2C);      //开始写入GRAM					 
	break;
 }
}




//液晶初始化
void Lcd_Init(void)
{


	LCD_CS=1;
	LCD_RD=1;
	LCD_WR=1;
	delay1ms(5);

	LCD_CS =0;  //打开片选使能		从这一步开始 始终开启屏片选使能	
	
		Lcd_WR_Reg(0xCF);  
		Lcd_Write_Data(0x00); 
		Lcd_Write_Data(0xC1); 
		Lcd_Write_Data(0X30); 
		Lcd_WR_Reg(0xED);  
		Lcd_Write_Data(0x64); 
		Lcd_Write_Data(0x03); 
		Lcd_Write_Data(0X12); 
		Lcd_Write_Data(0X81); 
		Lcd_WR_Reg(0xE8);  
		Lcd_Write_Data(0x85); 
		Lcd_Write_Data(0x10); 
		Lcd_Write_Data(0x7A); 
		Lcd_WR_Reg(0xCB);  
		Lcd_Write_Data(0x39); 
		Lcd_Write_Data(0x2C); 
		Lcd_Write_Data(0x00); 
		Lcd_Write_Data(0x34); 
		Lcd_Write_Data(0x02); 
		Lcd_WR_Reg(0xF7);  
		Lcd_Write_Data(0x20); 
		Lcd_WR_Reg(0xEA);  
		Lcd_Write_Data(0x00); 
		Lcd_Write_Data(0x00); 
		Lcd_WR_Reg(0xC0);    //Power control 
		Lcd_Write_Data(0x1B);   //VRH[5:0] 
		Lcd_WR_Reg(0xC1);    //Power control 
		Lcd_Write_Data(0x01);   //SAP[2:0];BT[3:0] 
		Lcd_WR_Reg(0xC5);    //VCM control 
		Lcd_Write_Data(0x30); 	 //3F
		Lcd_Write_Data(0x30); 	 //3C
		Lcd_WR_Reg(0xC7);    //VCM control2 
		Lcd_Write_Data(0XB7); 
		Lcd_WR_Reg(0x36);    // Memory Access Control 
		Lcd_Write_Data(0x08); 
		Lcd_WR_Reg(0x3A);   
		Lcd_Write_Data(0x55); 
		Lcd_WR_Reg(0xB1);   
		Lcd_Write_Data(0x00);   
		Lcd_Write_Data(0x1A); 
		Lcd_WR_Reg(0xB6);    // Display Function Control 
		Lcd_Write_Data(0x0A); 
		Lcd_Write_Data(0xA2); 
		Lcd_WR_Reg(0xF2);    // 3Gamma Function Disable 
		Lcd_Write_Data(0x00); 
		Lcd_WR_Reg(0x26);    //Gamma curve selected 
		Lcd_Write_Data(0x01); 
		Lcd_WR_Reg(0xE0);    //Set Gamma 
		Lcd_Write_Data(0x0F); 
		Lcd_Write_Data(0x2A); 
		Lcd_Write_Data(0x28); 
		Lcd_Write_Data(0x08); 
		Lcd_Write_Data(0x0E); 
		Lcd_Write_Data(0x08); 
		Lcd_Write_Data(0x54); 
		Lcd_Write_Data(0XA9); 
		Lcd_Write_Data(0x43); 
		Lcd_Write_Data(0x0A); 
		Lcd_Write_Data(0x0F); 
		Lcd_Write_Data(0x00); 
		Lcd_Write_Data(0x00); 
		Lcd_Write_Data(0x00); 
		Lcd_Write_Data(0x00); 		 
		Lcd_WR_Reg(0XE1);    //Set Gamma 
		Lcd_Write_Data(0x00); 
		Lcd_Write_Data(0x15); 
		Lcd_Write_Data(0x17); 
		Lcd_Write_Data(0x07); 
		Lcd_Write_Data(0x11); 
		Lcd_Write_Data(0x06); 
		Lcd_Write_Data(0x2B); 
		Lcd_Write_Data(0x56); 
		Lcd_Write_Data(0x3C); 
		Lcd_Write_Data(0x05); 
		Lcd_Write_Data(0x10); 
		Lcd_Write_Data(0x0F); 
		Lcd_Write_Data(0x3F); 
		Lcd_Write_Data(0x3F); 
		Lcd_Write_Data(0x0F); 
		Lcd_WR_Reg(0x2B); 
		Lcd_Write_Data(0x00);
		Lcd_Write_Data(0x00);
		Lcd_Write_Data(0x01);
		Lcd_Write_Data(0x3f);
		Lcd_WR_Reg(0x2A); 
		Lcd_Write_Data(0x00);
		Lcd_Write_Data(0x00);
		Lcd_Write_Data(0x00);
		Lcd_Write_Data(0xef);	 
		Lcd_WR_Reg(0x11); //Exit Sleep
		delay1ms(120);
		Lcd_WR_Reg(0x29); //display on	

}	   
