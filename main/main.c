/* 
TFT驱动IC为  ILI9341
tft
控制线：RS-P5^5;    WR-P4^2;   RD-P4^4;   CS-P5^4;   REST-硬件配置
数据线: P0口低八位数据端口  P2口高八位数据端口（两个数据口都以选用 不可做其他）
TFT显示 GUI函数 显示点 矩形 圆 圆矩形等图形
触摸屏 按下后会显示 
	         				
*/


#include  "stc15f2k60s2.h"		  //STC15头文件
#include  "def.h"	    		  //宏定义 常用函数
#include  "delay.h"				  //延时函数
#include  "tft.h"			      //TFT IC底层驱动
#include  "gui.h"
#include  "xpt2046.h"    //触摸屏驱动
#include  "spi.h"
#include  "key.h"
#include  "pff.h"				  //文件系统调用.h加载
#include  "sram.h"
#include  "flash.h"
#include  "sd.h"
#include  "ov7670.h"
#include <string.h>



FATFS fatfs;	                  //文件系统结构体定义
u8 tbuf[512];
u8 	pic_width;				 //图片宽度
u16 pic_height;				 //图片高度
u8 mode = 0;           //工作模式，0有参考线，1无参考线，2瞄准线，3回看
//针对 IAP15W4K61S4 系列 IO口初始化
//io口初始化 P0 P1 P2 P3 P4 为准双向IO口   
//注意: STC15W4K32S4系列的芯片,上电后所有与PWM相关的IO口均为
//      高阻态,需将这些口设置为准双向口或强推挽模式方可正常使用
//相关IO: P0.6/P0.7/P1.6/P1.7/P2.1/P2.2
//        P2.3/P2.7/P3.7/P4.2/P4.4/P4.5
void IO_init(void)
{
  P0M0 = 0X00;
  P0M1 = 0X00;

  P1M0 = 0X00;
  P1M1 = 0X00;

  P2M0 = 0X00;
  P2M1 = 0X00;

  P3M0 = 0X00;
  P3M1 = 0X00;

  P4M0 = 0X00;
  P4M1 = 0X00;  
}



//将屏幕从（0,0）位置到(240,240)位置 截取240x240图片数据 并转换为BMP格式存入TF卡中
//说明： 默认的文件名字"/OV76/M1.bmp" 别的名字也可以，但必须得在卡上有这个文件存在
//       写入是破坏式写入
//函数思路：因为petit fatfs文件系统只能在原有的文件上更新，而BMP图片前54字节
//          是图片的信息 这一部分是不要更改 保存原有就行
//          所以程序上要先将前54字节取出 然后后在更新54字节以后的图片数据
//注意：	petit fatfs文件系统 在写数据时必须要从 扇区的开头写起 不弄成扇区的中间写数据
//          所以在函数提取了前54字节后 在和后面的颜色数据组成512字节 在重新写回扇区
//*path 保存路径
//返回0 保存成功  1保存失败
u8 Show_Bmp(const char *path)
{
	FRESULT res; 

	u16 br,y=0,zy,height,	  //width,height图片的初始左边
			y1,i1,tmp;		              //tmp 16位解码变量
	u8 x=0,zx,width,x1,
			rgb=0, Bmpcolor;

	res=pf_open(path);		             //打开指定路径文件	 这一步可以打开任何路径下的文件 注意它的功能就是打开文件，不是文件夹
                                     //读文件内数据的前提是必须要打开这个文件

	if(res == FR_OK)
	{
			pf_read(tbuf, 54, &br);		 //取前54字节  前54字节含有bmp 文件大小 文件长宽尺度 像素值 等数据信息   
			if(br!=54) return 1;		 //提取出错

											//实际宽和高都是用4个字节表示的，但图片的大小不能超过屏的尺寸
											//故这里只用一个字节表示宽度,两个字节表示高度
			width  = tbuf[18];				        //计算图片宽度	 
			height = (tbuf[23]<<8)+tbuf[22];	//计算图片高度
		
			pic_width  = width;
			pic_height = height;

			Bmpcolor=tbuf[28]/8;					//获取颜色深度 一般是16位 24位 32位  
			//number(30,280,Bmpcolor,Red,White);
			//将小于屏幕尺寸的图片放到屏幕正中间 顶头显示
			if(width<239)   zx=(240-width)/2;         else zx=0;
			if(height<299)	zy=(320-height);        	else zy=0;
			
			x1=zx; y1=zy;			   //赋值计算后的值

			LCD_scan(2);	  //BMP图片解码的扫描方式为 从左到右   从下到上 否则显示的图片上下颠倒

			Address_set(x1,y1,x1+width-1,y1+height-1);         //设置显示范围 先扫横行 再扫纵行
			LCD_RS=1;    //写数据线拉高	 	  为提高写入速度 主循环前拉高
			while(1)                   //一直到最后一簇
			{
		  		 pf_read(tbuf, 512, &br);		 //从54字节后位置读取512个字节到缓存区  
					 for(i1=0;i1<512;i1++)
						{
							if(Bmpcolor==2)				 //16位BMP
							{			
								switch(rgb)				 //555转565格式
								{
									case 0:
										tmp=((u16)tbuf[i1]&0x1f);			 //R
										tmp+=(((u16)tbuf[i1])&0xe0)<<1;	 //G
									break;
									
									case 1:
										tmp+=(u16)tbuf[i1]<<9;				 //B
									break;		     		 
								} 
							}
							else if(Bmpcolor==3)		//24位BMP解码 RGB分别占8个字节
							{
								switch(rgb)
								{
									case 0:
										tmp=tbuf[i1]>>3;					 //B
									break;
									
									case 1:
										tmp+=((u16)tbuf[i1]<<3)&0x07e0;	 //G
									break;
			
									case 2:
										tmp+=((u16)tbuf[i1]<<8)&0xf800;	 //R
									break; 
								}
							}
						
							rgb++;

							if(rgb==Bmpcolor)
							{ 
								P2=tmp>>8;								  //为了提高显示速度 直接调用IO口本身
								P0=tmp;									  //void Lcd_Write_Data(u16 Data)函数的分解
								tmp=0;
								rgb=0;
								LCD_WR=0;								  //开始写入
								LCD_WR=1;			  	       
	
								x1++;							  //横向自加 加满一行 横向清零 纵向自加 直到扫完整体图片
								if(x1==width+zx)                  
								{	 
									y1--;
									x1=zx;
									if(y1==zy-height)
									{									 //恢复正常扫描方式
										LCD_scan(1);
										return 0;    //显示完成	
									}
								}
							}
						} 
			}
  }
	return 1;   //出错
}		    
void scan(void)//lcd屏幕扫描
{
	u32 j;
	Address_set(0,0,239,239);	  //设置显示范围  显示为240*240 
  if(cur_status==2)			   //判断缓存区是否存好摄像数据
  { 
		
			FIFO_RRST=0;			   //开始复位读指针	读取FIFO数据
      FIFO_RCLK=0;
      FIFO_RCLK=1;
      FIFO_RCLK=0;
      FIFO_RRST=1;			   //复位读指针结束
      FIFO_RCLK=1;
			LCD_RS=1;				   //拉高TFT 写数据端 

	  for(j =0;j<57600;j++)	   //分辨率为240x240   每个颜色点要两个字节 所以 240x240x2=57600  次
	  {		 
			FIFO_RCLK=0;		   //每一次时钟跳变 读一次数据		
			P2=P1;				   //直接将高字节数据给P2
			FIFO_RCLK=1; 

			FIFO_RCLK=0;
			P0=P1;	           	   //直接将低字节数据给P0
			FIFO_RCLK=1; 	


			LCD_WR=0;			   // 显示
      LCD_WR=1;		
	  } 
		EX0 = 1; 				   //显示完成开中断
	  cur_status=0;			   //显示完整个图片以后 将	 cur_status置0  准备接收下一帧
	}
}
u8 Save_Bmp(const char *path)
{
	FRESULT res;
	u16 br,i,j,m=0,n=239,color;

	sram(1);							 //开启外部内存	   
                                     //由于文件路径在外部SRAM中 所以这里要开启外部SRAM才能调用路径
	res=pf_open(path);		             //打开指定路径文件	 这一步可以打开任何路径下的文件 注意它的功能就是打开文件，不是文件夹
                                     //读文件内数据的前提是必须要打开这个文件

	if(res == FR_OK)
	{
			led = 0; //存盘指示灯(P3.5)亮
		
			pf_read(tbuf,54,&br);				 	//提取BMP图片前54字节图片信息
			pf_open(path);		 						//重新打开路径 将扇区指向图片首数据位置
			sram(0);							 			 	//关闭外部内存，开启液晶片选
		
			for(i=27;i<256;i++)				 		//提取512个字节到tbuf中 即256个颜色点
      {
					color=LCD_readpoint(m,n);	//提取LCD每个像元的颜色
					color=((color>>1)&0x7fe0)+(color&0x001f);	//将提取的565格式转换为555格式

					tbuf[i*2]=color;				 	//存入时低字节在前
					tbuf[i*2+1]=(color>>8);
					m++;
      }
					
			pf_write(tbuf,512,&br);			 	//向TF卡内写入512个字节（1个扇区）

			
     for(j=0;j<254;j++)
		 {
	     for(i=0;i<256;i++)				 		//提取512个字节到tbuf中 即256个颜色点
	     {
					color=LCD_readpoint(m,n);	//符合摄像头摄像效果  提取颜色
					color=((color>>1)&0x7fe0)+(color&0x001f); //将提取的565格式转换为555格式

	        tbuf[i*2]=color;				 	//存入时低字节在前
					tbuf[i*2+1]=(color>>8);
	
					m++;
					if(m==240)
					{
						m=0;
						n--;	   		//这里不用判断m 因为循环固定 直接会跳出 
					}
	     }	  
	     pf_write(tbuf,512,&br);			 //向TF卡内写入512个字节		 
		 }

		SD_DisSelect();//取消TF卡片选  在写入函数里加取消片选 会有影响 所以在最后写入完成加取消片选
	 
		delay1ms(2000);    //保持显示延时
		led = 1; //存盘指示灯(P3.5)灭
		return 0;  //写入成功
	}
	else
	{
		return 1;    //错误
	}
}
//
//触摸数据转换屏实际坐标函数体
//返回 result.x坐标 result.y坐标 result.flag=1表示有键按下
//
void GetTouchScreenPos(struct TFT_Pointer * sp)
{
		u16 a,b, flag;				//临时变量

		u8  ax[8];
		u16 x1,y1;
		u16 x2,y2;

		u8 i2t[8];			   //读24c02中 触摸校准参数 临时转换调用数组
		
	//触摸屏校准参数
		struct T_i T_i2c=
		{
			656,			 
			883,
			-13,
			-12,
		};
	
		//读取AD值并转换为X Y坐标
#define ERR_RANGE 5 //误差范围 
  
		if(AD7843_isClick==0)
		{	 
			delay1ms(1);
			if(AD7843_isClick==0)
			{
				LCD_CS=1;			//xpt的片选线在tft上 防止触摸工作时影响tft	这里关掉TFT使能
				AD7843_CS=0; 		//开片选
				SPI_Speed(2);		//降低 SPI通讯速度 使触摸芯片放回数据更稳定
				/*这里采用16时钟周期采集  因为 此触摸功能采用的是SPI总线
					而SPI功能是只能8位传输  XPT2046的AD分辨率为12位  需要读两次
					根据XPT2046手册中 16时钟周期 时序图 可以看出
					发送采集数据  接收一次SPI数据后 在发送空功能的SPI数据  就会把剩下的部分接收到
					这样先接收的 是低字节数据  第二次接收的是高字节数据  移位后 便是12位的AD值   
				*/
				ax[0]=SPI_SendByte(0x90);  //送控制字10010000即用差分方式读X坐标，舍弃读取的数据
				ax[0]=SPI_SendByte(0x00);  //发送任意数据（最高位不能为1，和2046命令冲突），接收X高字节
				ax[1]=SPI_SendByte(0xD0);  //送控制字11010000即用差分方式读Y坐标，接收X低字节
				ax[2]=SPI_SendByte(0x00);  //发送任意数据（同上），接收Y高字节
				ax[3]=SPI_SendByte(0x90);  //送控制字10010000 （第二次）读X坐标，接收Y低字节
				ax[4]=SPI_SendByte(0x00);  //发送任意数据（同上），接收X高字节
				ax[5]=SPI_SendByte(0xD0);  //送控制字11010000  （第二次）读Y坐标，接收X低字节
				ax[6]=SPI_SendByte(0x00);  //发送任意数据（同上)，接收Y高字节
				ax[7]=SPI_SendByte(0x90);  //送控制字10010000  （第三次）读X坐标，接收Y低字节

				//提取两次采集值
				y1=(ax[0]<<5)|(ax[1]>>3);
				y2=(ax[4]<<5)|(ax[5]>>3);
				x1=(ax[2]<<5)|(ax[3]>>3);
				x2=(ax[6]<<5)|(ax[7]>>3);

				if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-ERR_RANGE内
						&&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
				{
					flag=1;			//打开标志位
					a=(x1+x2)/2;
					b=(y1+y2)/2;
				}
				else flag=0;

				SPI_Speed(0);		//调整SPI速度为最高
				AD7843_CS=1; 		//关片选
				LCD_CS=0;
			}
		} 

	
		/* 触摸屏计算公式
   lcdx=xa*tpx+xb;
   lcdy=ya*tpy+yb;
   lcdx,lcdy为屏坐标  tpx,tpy为触屏板值 xa,ya为比例因子  xb,yb为偏移量

   计算方法 
   在屏幕上指定lcdx,lcdy位置画出十字图形 分别画在屏幕上的4个角位置
   用触摸笔分别点击 得到其中的触摸值
   根据上面的公式 计算	xa,ya  xb,yb  这样就能使得触摸板和屏幕校准
		*/
		//无校准功能  
		//		 result.x=0.065894*a-16;			//将得到的AD值带入公式 计算lcd屏的x y坐标 
		//		 result.y=0.084031*b-14;
		
		//加了校准功能  
		 sp->x = ((float)T_i2c.xi/10000)*a+T_i2c.a;			//将得到的AD值带入公式 计算lcd屏的x y坐标 
		 sp->y = ((float)T_i2c.yi/10000)*b+T_i2c.b;
		 sp->flag = flag;
}
// 渲染图片
/*u8 Replay_Show_Bmp(char *path)
{
	FRESULT res; 

	u16 br,y=0,zy,height,	  //width,height图片的初始左边
			y1,i1,tmp;		              //tmp 16位解码变量
	u8 x=0,zx,width,x1,
			rgb=0, Bmpcolor;

	res=pf_open(path);		             //打开指定路径文件	 这一步可以打开任何路径下的文件 注意它的功能就是打开文件，不是文件夹
                                     //读文件内数据的前提是必须要打开这个文件

	if(res == FR_OK)
	{
			pf_read(tbuf, 54, &br);		 //取前54字节  前54字节含有bmp 文件大小 文件长宽尺度 像素值 等数据信息   
			if(br!=54) return 1;		 //提取出错

											//实际宽和高都是用4个字节表示的，但图片的大小不能超过屏的尺寸
											//故这里只用一个字节表示宽度,两个字节表示高度
			width  = tbuf[18];				        //计算图片宽度	 
			height = (tbuf[23]<<8)+tbuf[22];	//计算图片高度
		
			pic_width  = width;
			pic_height = height;

			Bmpcolor=tbuf[28]/8;					//获取颜色深度 一般是16位 24位 32位  
			//number(30,280,Bmpcolor,Red,White);
			//将小于屏幕尺寸的图片放到屏幕正中间 顶头显示
			if(width<239)   zx=(240-width)/2;         else zx=0;
			if(height<299)	zy=(320-height);        	else zy=0;
			
			x1=zx; y1=zy;			   //赋值计算后的值

			LCD_scan(2);	  //BMP图片解码的扫描方式为 从左到右   从下到上 否则显示的图片上下颠倒

			Address_set(x1,y1,x1+width-1,y1+height-1);         //设置显示范围 先扫横行 再扫纵行
			LCD_RS=1;    //写数据线拉高	 	  为提高写入速度 主循环前拉高
			while(1)                   //一直到最后一簇
			{
		  		 pf_read(tbuf, 512, &br);		 //从54字节后位置读取512个字节到缓存区  
					 for(i1=0;i1<512;i1++)
						{
							if(Bmpcolor==2)				 //16位BMP
							{			
								switch(rgb)				 //555转565格式
								{
									case 0:
										tmp=((u16)tbuf[i1]&0x1f);			 //R
										tmp+=(((u16)tbuf[i1])&0xe0)<<1;	 //G
									break;
									
									case 1:
										tmp+=(u16)tbuf[i1]<<9;				 //B
									break;		     		 
								} 
							}
							else if(Bmpcolor==3)		//24位BMP解码 RGB分别占8个字节
							{
								switch(rgb)
								{
									case 0:
										tmp=tbuf[i1]>>3;					 //B
									break;
									
									case 1:
										tmp+=((u16)tbuf[i1]<<3)&0x07e0;	 //G
									break;
			
									case 2:
										tmp+=((u16)tbuf[i1]<<8)&0xf800;	 //R
									break; 
								}
							}
						
							rgb++;

							if(rgb==Bmpcolor)
							{ 
								P2=tmp>>8;								  //为了提高显示速度 直接调用IO口本身
								P0=tmp;									  //void Lcd_Write_Data(u16 Data)函数的分解
								tmp=0;
								rgb=0;
								LCD_WR=0;								  //开始写入
								LCD_WR=1;			  	       
	
								x1++;							  //横向自加 加满一行 横向清零 纵向自加 直到扫完整体图片
								if(x1==width+zx)                  
								{	 
									y1--;
									x1=zx;
									if(y1==zy-height)
									{									 //恢复正常扫描方式
										LCD_scan(1);
										return 0;    //显示完成	
									}
								}
							}
						} 
			}
  }
	return 1;   //出错
}		    
u8 Replay_Next(int *CURRENT, int *TOTAL,char *FILES){

	*CURRENT++;

	if(*CURRENT>=*TOTAL){
		
		*CURRENT = 0;
		
	} 

	Replay_Show_Bmp(&FILES[*CURRENT]);


	return 0;


}
u8 Replay_Prev(int *CURRENT,int *TOTAL,char *FILES){

	*CURRENT--;

	if(CURRENT<0){
		
		CURRENT = TOTAL-1;
		
	} 

	Replay_Show_Bmp(&FILES[*CURRENT]);

	return 0;


}
*/
void main()
{ 
	char* fname[30]=
  {"/OV76/M1.bmp",
	 "/OV76/M2.bmp",
	"/OV76/M3.bmp",
	"/OV76/M4.bmp",
	"/OV76/M5.bmp",
	"/OV76/M6.bmp",
	"/OV76/M7.bmp",
	"/OV76/M8.bmp",
	"/OV76/M9.bmp",
	"/OV76/M10.bmp",
	"/OV76/M11.bmp",
	"/OV76/M12.bmp",
	"/OV76/M13.bmp",
	"/OV76/M14.bmp",
	"/OV76/M15.bmp",
	"/OV76/M16.bmp",
	"/OV76/M17.bmp",
	"/OV76/M18.bmp",
	"/OV76/M19.bmp",
	"/OV76/M20.bmp",
	"/OV76/M21.bmp",
	"/OV76/M22.bmp",
	"/OV76/M23.bmp",
	"/OV76/M24.bmp",
	"/OV76/M25.bmp",
	"/OV76/M26.bmp",
	"/OV76/M27.bmp",
	"/OV76/M28.bmp",
	"/OV76/M29.bmp",
	"/OV76/M30.bmp"};
	int x=0;
	int current = 1;
	int y =300;
	u32  j;					  			//临时变量
	u8  sign=0;			  			//初始标志
	u8 cankao=1;              //参考线开关标志
	struct TFT_Pointer sp;  //定义触摸变量结构体
	struct TFT_Pointer tp; //用来检查触摸屏是否有键按下
	/*  初始化阶段 */
  SP=0X80;				      	// 调整堆栈指向   手册286页 详解
  IO_init();				  	  // 真对 IAP15W4K61S4  IO口初始化
  Lcd_Init();             // TFT液晶初始化
  Init_SPI(); 			      // SPI初始化
  GUI_Clear(Black);			  // 白色清屏 
  SD_Init();			      	// SD卡初始化
  KEY_Init();	
  pf_mount(&fatfs);	      //初始化petit FATFS文件系统  并提取tf卡相应数据
                          //（这句非常重要，它是使用所有Petit Fatfs文件系统功能的前提）

  mem_init();				  		//外部SRAM初始化




	 P1M0=0X00;	  //P1口为仅输入状态
	 P1M1=0Xff;		
	
	/* 开启中断目的为判断VSYNC（帧数据判断）：当有触发的时候为来一帧
	   这时开始往摄像头FIFO缓存芯片AL422B灌入一帧数据。
		 当来第二帧时说明数据灌入完成，此时提取数据进行显示。 */
  IT0=1;			   //边沿触发   
  EX0=1;         //外部中断0   P3.2口    
  EA =1;      //开总中断 
	
  /* 将液晶显示屏用黑色清空 */
  GUI_Clear(Black);
  LCD_scan(1);  //液晶屏设置扫描方式：从上到下 从右到左
  
	

	/* OV7670初始化 */
  if( Ov7670_init_normal()) 
  {															 //初始化不成功
   GUI_sprintf_hzstr16x(60,150,"OV7670初始化",White,Blue);
   delay1ms(2000);
   EX0=0;			   //关闭外部中断0
   EA=0;			   //关闭总中断
   sign=1;
  }

	if (SD_Init()) 							//SD卡初始化，不成功退出
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //初始化失败跳出 （没有插OV7670 或者没插好） 

  FIFO_OE=0;		   //使能摄像头模块

  OV7670_Window_Set(10,176,240,240);	//设置显示窗口尺寸：240*240，场频是10，行频是176

  
	
  IO_init();				   //对 IAP15W4K61S4  IO口初始化
	Lcd_Init();          //tft初始化
	Init_SPI(); 				 //触摸屏SPI接口初始化
while(1)
{	
	GUI_Clear(Black);
 /* 开始扫描 显示摄像头采集数据 */
 while(mode==0)//mode==0,参考线照相
 {
	  
    scan();
		GUI_arc(120,269,25,White,1);	 //120，269位置画一个半径为25的实心圆
		GUI_box(20,245,68,293,White);//左下角画一个48*48的实心白色矩形
		GUI_sprintf_hzstr16x(20,263,"replay", Red, White); //矩形的位置写黑色字

		GUI_line(80,0,80,240,White);//参考线
		GUI_line(160,0,160,240,White);
		GUI_line(0,80,240,80,White);
		GUI_line(0,160,240,160,White);
		
		GUI_tri(180,245,180,293,White);//三角形
		GetTouchScreenPos(&tp); //采集触摸屏
		
		if (tp.flag == 1)           //是否有触摸事件发生 
    { 
			tp.flag = 0;
			if ((tp.x>20) && (tp.x<68) && (tp.y>245) && (tp.y<293)) //点击左下角矩形
			{
				led=0;
				delay1ms(100);//延迟0.1s
				led=1;
				mode=3;
				delay1ms(100);
				break;
			}
			
			if ((tp.x>100) && (tp.x<140) && (tp.y>249) && (tp.y<288)) //点击圆形,再扫描一次（无参考线），并保存
			{   
					for(j = 0;j<5;j++)
					{
						 scan();
		GUI_arc(120,269,25,White,1);	 //120，269位置画一个半径为25的实心圆
		GUI_box(20,245,68,293,White);//左下角画一个48*48的实心白色矩形
		GUI_sprintf_hzstr16x(20,263,"replay", Red, White); //矩形的位置写黑色字

		GUI_line(80,0,80,240,White);//参考线
		GUI_line(160,0,160,240,White);
		GUI_line(0,80,240,80,White);
		GUI_line(0,160,240,160,White);
		
		GUI_tri(180,245,180,293,White);//三角形
						Save_Bmp(fname[current]);
						if(current+1>30)
						{
							current=0;
						}
						else
						{
							current++;
						}
					}		
				}
			if ((tp.x>180) && (tp.x<220) && (tp.y>220) && (tp.y<295)) //点击三角形,改变模式
			{
				led=0;
				delay1ms(100);//延迟0.1s
				led=1;
				if(mode+1<=3)
				{
					mode ++;
					  EX0=1;         //外部中断0   P3.2口    
  EA =1;      //开总中断 
	
  /* 将液晶显示屏用黑色清空 */
  GUI_Clear(Black);
  LCD_scan(1);  //液晶屏设置扫描方式：从上到下 从右到左
					if(Ov7670_init_fupian()) 
  {															 //初始化不成功
   GUI_sprintf_hzstr16x(60,150,"OV7670初始化",White,Blue);
   delay1ms(2000);
   EX0=0;			   //关闭外部中断0
   EA=0;			   //关闭总中断
   sign=1;
  }
	if (SD_Init()) 							//SD卡初始化，不成功退出
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //初始化失败跳出 （没有插OV7670 或者没插好） 

  FIFO_OE=0;		   //使能摄像头模块

  OV7670_Window_Set(10,176,240,240);	//设置显示窗口尺寸：240*240，场频是10，行频是176
				}
				else
				{
					mode = 0;
				}				
					delay1ms(100);
				break;
			}
					
			} 
		}	
 
		while(mode==1)//mode==1,无参考线照相
 {
	 
    scan();
		GUI_arc(120,269,25,White,1);	 //120，269位置画一个半径为25的实心圆
		GUI_box(20,245,68,293,White);//左下角画一个48*48的实心白色矩形
		GUI_sprintf_hzstr16x(20,263,"replay", Red, White); //矩形的位置写黑色字
		GUI_tri(180,245,180,293,White);//三角形
	  //GUI_Target_PSO1(Black);//瞄准
		GetTouchScreenPos(&tp); //采集触摸屏
		
		if (tp.flag == 1)           //是否有触摸事件发生 
    { 
			tp.flag = 0;
			if ((tp.x>20) && (tp.x<68) && (tp.y>245) && (tp.y<293)) //点击左下角矩形
			{
				led=0;
				delay1ms(100);//延迟0.1s
				led=1;
				mode=3;
				delay1ms(100);
				break;
			}
			
			if ((tp.x>100) && (tp.x<140) && (tp.y>249) && (tp.y<288)) //点击圆形,再扫描一次（无参考线），并保存
			{   Save_Bmp(fname[current]);
				  if(current+1>30)
					{
						current=0;
					}
					else
					{
						current++;
					}
				  	
			}
			if ((tp.x>180) && (tp.x<220) && (tp.y>220) && (tp.y<295)) //点击三角形,改变模式
			{
				led=0;
				delay1ms(100);//延迟0.1s
				led=1;
				if(mode+1<=3)
				{
					mode ++;
					  EX0=1;         //外部中断0   P3.2口    
  EA =1;      //开总中断 
	
  /* 将液晶显示屏用黑色清空 */
  GUI_Clear(Black);
  LCD_scan(1);  //液晶屏设置扫描方式：从上到下 从右到左
					if(Ov7670_init_fanzhuan()) 
  {															 //初始化不成功
   GUI_sprintf_hzstr16x(60,150,"OV7670初始化",White,Blue);
   delay1ms(2000);
   EX0=0;			   //关闭外部中断0
   EA=0;			   //关闭总中断
   sign=1;
  }
	if (SD_Init()) 							//SD卡初始化，不成功退出
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //初始化失败跳出 （没有插OV7670 或者没插好） 

  FIFO_OE=0;		   //使能摄像头模块

  OV7670_Window_Set(10,176,240,240);	//设置显示窗口尺寸：240*240，场频是10，行频是176
				}
				else
				{
					mode = 0;
				}				
					delay1ms(100);
				break;
			}
					
			} 
		}	
 
		while(mode==2)//mode==2,瞄准线照相
 {
    scan();
		GUI_arc(120,269,25,White,1);	 //120，269位置画一个半径为25的实心圆
		GUI_box(20,245,68,293,White);//左下角画一个48*48的实心白色矩形
		GUI_sprintf_hzstr16x(20,263,"replay", Red, White); //矩形的位置写黑色字
		GUI_tri(180,245,180,293,White);//三角形
	  GUI_Target_PSO1(Black);//瞄准
		GetTouchScreenPos(&tp); //采集触摸屏
		
		if (tp.flag == 1)           //是否有触摸事件发生 
    { 
			tp.flag = 0;
			if ((tp.x>20) && (tp.x<68) && (tp.y>245) && (tp.y<293)) //点击左下角矩形
			{
				led=0;
				delay1ms(100);//延迟0.1s
				led=1;
				mode=3;
				delay1ms(100);
				break;
			}
			
			if ((tp.x>100) && (tp.x<140) && (tp.y>249) && (tp.y<288)) //点击圆形,再扫描一次（无参考线），并保存
			{   
				Save_Bmp(fname[current]);
				  if(current+1>30)
					{
						current=0;
					}
					else
					{
						current++;
					}
				 
			}
			if ((tp.x>180) && (tp.x<220) && (tp.y>220) && (tp.y<295)) //点击三角形,改变模式
			{
				led=0;
				delay1ms(100);//延迟0.1s
				led=1;
				if(mode+1<=3)
				{
					mode ++;
				}
				else
				{
					mode = 0;
					mode ++;
					  EX0=1;         //外部中断0   P3.2口    
  EA =1;      //开总中断 
	
  /* 将液晶显示屏用黑色清空 */
  GUI_Clear(Black);
  LCD_scan(1);  //液晶屏设置扫描方式：从上到下 从右到左
					if(Ov7670_init_normal()) 
  {															 //初始化不成功
   GUI_sprintf_hzstr16x(60,150,"OV7670初始化",White,Blue);
   delay1ms(2000);
   EX0=0;			   //关闭外部中断0
   EA=0;			   //关闭总中断
   sign=1;
  }
	if (SD_Init()) 							//SD卡初始化，不成功退出
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //初始化失败跳出 （没有插OV7670 或者没插好） 

  FIFO_OE=0;		   //使能摄像头模块

  OV7670_Window_Set(10,176,240,240);	//设置显示窗口尺寸：240*240，场频是10，行频是176
				}				
					delay1ms(100);
				break;
			}
					
			} 
		}

if(mode==3)//mode==3,回看界面
 {
	  GUI_Clear(Black);
		GUI_box(20,245,68,293,White);//左下角画一个48*48的实心白色矩形
		GUI_sprintf_hzstr16x(20,263,"home", Red, White); //矩形的位置写字
		GUI_box(96,245,144,293,White);//
		GUI_sprintf_hzstr16x(96,263,"last", Red, White); //矩形的位置写字
		GUI_box(172,245,220,293,White);	 //120，269位置画一个半径为25的实心圆
	  GUI_sprintf_hzstr16x(172,263,"next", Red, White); //矩形的位置写字
		
	  Show_Bmp(fname[current]); //显示第一张图片
	 GUI_sprintf_hzstr16x(x, y, fname[current], Black, White);
						x += 8*strlen(fname[current])+20;
						number(x, y, pic_width, Black, White); //图片的宽度
						x += 30;
						GUI_sprintf_hzstr16x(x, y, "x", Black, White);
						x += 16;
						number(x, y, pic_height, Black, White); //图片的高度
	while(1){	
		GetTouchScreenPos(&tp); //采集触摸屏
	 if (tp.flag == 1)           //是否有触摸事件发生 
    { 
			tp.flag = 0;
			if ((tp.x>20) && (tp.x<68) && (tp.y>245) && (tp.y<293)) //点击左下角矩形，回到模式0
			{
				led=0;
				delay1ms(100);//延迟0.1s
				led=1;
				mode=0;
				mode ++;
					  EX0=1;         //外部中断0   P3.2口    
  EA =1;      //开总中断 
	
  /* 将液晶显示屏用黑色清空 */
  GUI_Clear(Black);
  LCD_scan(1);  //液晶屏设置扫描方式：从上到下 从右到左
					if(Ov7670_init_normal()) 
  {															 //初始化不成功
   GUI_sprintf_hzstr16x(60,150,"OV7670初始化",White,Blue);
   delay1ms(2000);
   EX0=0;			   //关闭外部中断0
   EA=0;			   //关闭总中断
   sign=1;
  }
	if (SD_Init()) 							//SD卡初始化，不成功退出
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //初始化失败跳出 （没有插OV7670 或者没插好） 

  FIFO_OE=0;		   //使能摄像头模块

  OV7670_Window_Set(10,176,240,240);	//设置显示窗口尺寸：240*240，场频是10，行频是176
				delay1ms(100);
				break;
			}
			if ((tp.x>96) && (tp.x<144) && (tp.y>245) && (tp.y<293)) //点击中间,切换照片
			{
				GUI_Clear(Black);
		GUI_box(20,245,68,293,White);//左下角画一个48*48的实心白色矩形
		GUI_sprintf_hzstr16x(20,263,"home", Red, White); //矩形的位置写字
		GUI_box(96,245,144,293,White);//
		GUI_sprintf_hzstr16x(96,263,"last", Red, White); //矩形的位置写字
		GUI_box(172,245,220,293,White);	 //120，269位置画一个半径为25的实心圆
	  GUI_sprintf_hzstr16x(172,263,"next", Red, White); //矩形的位置写字
				led=0;
				delay1ms(100);//延迟0.1s
				led=1;
				if(current==1)
					{
						current=30;
					}
					else
					{
						current--;
					}		//上一张
				 if(Show_Bmp(fname[current])==0)	  //显示指定路径下的bmp文件	   
                              //这是要已知存入TF卡里的路径 "/ov76/M1.bmp"就是在TF卡根目录下文件名为机器猫.bmp文件
					{
						x=0;
						y=300;
						GUI_sprintf_hzstr16x(x, y, fname[current], Black, White);
						x += 8*strlen(fname[current])+20;
						number(x, y, pic_width, Black, White); //图片的宽度
						x += 30;
						GUI_sprintf_hzstr16x(x, y, "x", Black, White);
						x += 16;
						number(x, y, pic_height, Black, White); //图片的高度
					}
				  
			}
			if ((tp.x>172) && (tp.x<220) && (tp.y>245) && (tp.y<293)) //点击右下,切换照片
			{
				GUI_Clear(Black);
		GUI_box(20,245,68,293,White);//左下角画一个48*48的实心白色矩形
		GUI_sprintf_hzstr16x(20,263,"home", Red, White); //矩形的位置写字
		GUI_box(96,245,144,293,White);//
		GUI_sprintf_hzstr16x(96,263,"last", Red, White); //矩形的位置写字
		GUI_box(172,245,220,293,White);	 //120，269位置画一个半径为25的实心圆
	  GUI_sprintf_hzstr16x(172,263,"next", Red, White); //矩形的位置写字
				led=0;
				delay1ms(100);//延迟0.1s
				led=1;
				 if(current==30)
					{
						current=0;
					}
					else
					{
						current++;
					}		 	//下一张
					if(Show_Bmp(fname[current])==0)	  //显示指定路径下的bmp文件	   
                              //这是要已知存入TF卡里的路径 "/ov76/M1.bmp"就是在TF卡根目录下文件名为机器猫.bmp文件
					{
						x=0;
						y=300;
						GUI_sprintf_hzstr16x(x, y, fname[current], Black, White);
						x += 8*strlen(fname[current])+20;
						number(x, y, pic_width, Black, White); //图片的宽度
						x += 30;
						GUI_sprintf_hzstr16x(x, y, "x", Black, White);
						x += 16;
						number(x, y, pic_height, Black, White); //图片的高度
					}
			}		
		}
	}		
	}			
	
 }
}
 
