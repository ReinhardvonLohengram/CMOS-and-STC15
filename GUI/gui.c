

#include  "stc15f2k60s2.h"
#include  "gui.h"
#include  "tft.h"
#include  "delay.h" 
#include  "zifu8x16.h"			
#include  "hz16x16.h"		  //汉字16X16
#include  "pff.h"
#include  "sd.h"
#include  "flash.h"




//在指定位置显示 字库里的 GBK汉字   当字体背景颜色 只是0x0001 那么此时不显示背景颜色 背景颜色为默认颜色
//支持横向纵向显示选择功能 mode  但是只有纵向显示的时候才支持 背景颜色保持屏幕本身有的颜色
// x  y  显示的具体坐标
// Disp_char[2] 需要显示的GBK码
// fColor  	 bColor    字体 背景颜色 
// mode    0 纵向显示  1 横向显示  （主要要配合扫描方式）
//返回0 显示成功  返回1 在字库内 没有对应的GBK码
u8 PutGB1616(u16 x, u16  y, u8 Disp_char[2], u16 fColor,u16 bColor,u8 mode)
{

    u8 qh,ql;
	u8 i,j;
    u8  tmp_Char_Model[32];	//GBK12 点阵字库中  提取一个16X16点阵 即32个字节

    qh=Disp_char[0];
	ql=Disp_char[1];

	if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//非 常用汉字
	return 1; //结束访问 
	          
	if(ql<0x7f)ql-=0x40;//根据低字节在两个区域 将高低字节转移到 指定字节位置
	else ql-=0x41;
	qh-=0x81; 	 


    // 读字模 					   //  得到区值    	得到位数值			 一个字 32个字节
    //SD_read_Byte((gbk12_sector<<9) + ((u32)qh*190 + (u32)ql)*32,tmp_Char_Model,32);		//TF卡 读取模式	

	/*
	 TF卡模式
	 gbk12_sector 为字库所在扇区   gbk12_sector<<9相当于X512  转为字节数据 即字库所在字节开始位置
	 每个GBK 码由2 个字节组成，

	 FLASH 模式 
	 flash 中已经将TF卡里的 字库数据完整存入 flash中 只要确定地址即可 这个地址就是存入时的第一个地址 
	 这里是从 第322个扇区开始存入的  flash的一个扇区是4k的字节空间 即 4096个字节 所以首地址是 322*4096

	 第一个字节为0X81~0XFE，
	 第二个字节分为两部分， 一是0X40~0X7E，二是0X80~0XFE 。
	 第一个字节代表为区，那么GBK 里面总共有126 个区（0XFE-0X81+1 ）， 
     每个区内有 190 个汉字（0XFE-0X80+0X7E-0X40+2 ），总共就有 126*190=23940 个汉字
	 点阵库编码规则从 0X8140  开始   所以首先要判断低字节所在区域  然后得到具体字节位置
	 ((u32)qh*190 + (u32)ql)*32 每个区 190个字   一个字32个字节
	*/

	SPI_Flash_Read(tmp_Char_Model,(u32)322*4096+((u32)qh*190 + (u32)ql)*32,32);	   //FLASH读取模式


   if(bColor!=0x0001)							//支持背景颜色设置
    {														 //注意 不管设置哪一显示方向 前提是扫描要对应
	    if(mode==0)Address_set(x,y,  x+16-1, y+16-1);	 // 设置为纵向显示
		if(mode==1)Address_set(x,y,  x+16-1, y+16-1);// 设置为纵向显示
	    //显示出来 
	    for(i=0;i<32;i++)
	    {
	       	        
	        for(j=0;j<8;j++) 
	        {
	            if(tmp_Char_Model[i] & 0x80) 
	                   {
	                	    Lcd_Write_Data(fColor);
	                	}
	            else 
	                    {
	                	    Lcd_Write_Data(bColor);
	                	}
	            tmp_Char_Model[i] <<= 1;
	        } 
	     }
	 }
   else										//不支持背景颜色设置  只显示字体颜色 背景颜色与刷屏时一致
	{
	    qh=x;								//复制x值给qh
		for(i=0;i<32;i++)
		{
	        for(j=0;j<8;j++) 
	        {
			    Address_set(x,y,x,y);	//显示坐标要一个点一个点显示
	            if(tmp_Char_Model[i] & 0x80) 
	                   {
	                	    Lcd_Write_Data(fColor);		//显示汉字的颜色
							x++;						//x自加
							if((x-qh)==16){x=qh;y++;}	//到一行底 清x值
	                	}
	            else 
	                    {								//没有汉字的颜色要显示 即背景 背景不显示使用屏幕自带
	                	    x++;						
							if((x-qh)==16){x=qh;y++;}	//到一行底 清x值
	                	}
	            tmp_Char_Model[i] <<= 1;				//下一个字节
	        } 		
		}
	
	}

return 0;//成功
   
}



//清屏
//color是背景颜色。
//说明：使用背景颜色清除TFT模块屏幕的全部显示内容。
void GUI_Clear(u16 color)
{
	u16 i;
	u8 j;
	Address_set(0,0,239,319);
    for(i=0;i<320;i++)
	 {
	  for (j=0;j<240;j++)
	   	{
        	 Lcd_Write_Data(color);
	    }

	  }
}



//画点
//(x，y)是点的坐标
//color 是点的颜色。
//说明：用指定的颜色在指定的坐标位置上画出一个点。
void GUI_Point(u8 x, u16 y, u16 color)
{  
	Address_set(x,y,x,y);
	Lcd_Write_Data(color);
}

 



//画8点(Bresenham算法)		  
//(rx,ry,a,b):参数
//color:颜色
void gui_circle8(u16 rx,u16 ry,int a,int b,u16 color)
{
	GUI_Point(rx+a,ry-b,color);              
	GUI_Point(rx+b,ry-a,color);                       
	GUI_Point(rx+b,ry+a,color);                           
	GUI_Point(rx+a,ry+b,color);             
	GUI_Point(rx-a,ry+b,color);                  
	GUI_Point(rx-b,ry+a,color);               
	GUI_Point(rx-b,ry-a,color);                      
	GUI_Point(rx-a,ry-b,color);                  
}
void GUI_line(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)//画直线，要求x1,yi为起点坐标，x2,y2终点坐标。必须有一个坐标相等且终点坐标值要更大
{
	int i;
	if(x1 == x2)
	{
		for(i = y1;i < y2; i++)
		{
			GUI_Point(x1, i, color);
		}
	}
	if(y1 == y2)
	{
		for(i = x1;i < x2; i++)
		{
			GUI_Point(i, y1, color);
		}
	}
}

//以某条垂直线为底，在其右侧画等腰三角形
void GUI_tri(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)//画三角形
{
	u16 len;
	int i;
	int tempy = y2;
	int tempx = x1;
	len = y2 - y1;//垂直线段长度
	for(i = len;i >=1; i=i-2)
	{
		
		GUI_line(tempx,tempy-i,tempx,tempy,color);
		tempy = tempy-1;
		tempx = tempx+1;
	}
}



//在指定位置画一个指定大小的圆
//(rx,ry):圆心
//r    :半径
//color:颜色
//mode :0,不填充;1,填充
void GUI_arc(u16 rx,u16 ry,u16 r,u16 color,u8 mode)
{
	int a,b,c;
	int di;	  
	a=0;b=r;	  
	di=3-(r<<1);	//判断下个点位置的标志
	while(a<=b)
	{
		if(mode)
		for(c=a;c<=b;c++)
		gui_circle8(rx,ry,a,c,color);//画实心圆
 		else gui_circle8(rx,ry,a,b,color);			//画空心圆
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 							  
	}
}

//画实心矩形
//(sx,sy)左上角顶点坐标, 
//(ex,ey)右下角顶点坐标, color颜色
//返回: 无
//说明：在指定位置上画出实心矩形。
void GUI_box(u8 sx,u16 sy,u8 ex,u16 ey,u16 color)
{ 
	u16 temp,temp1,m,n;   
	Address_set(sx,sy,ex,ey); 
    n=ex-sx+1;
	m=ey-sy+1;

	for(temp=0;temp<m;temp++)
	{	 	
		for(temp1=0;temp1<n;temp1++)
	 	{	
			Lcd_Write_Data(color);
		}
	}
}


////画颜色递变巨型  可以是横条 竖条等
////(sx,sy)左上角顶点坐标, 
////(ex,ey)右下角顶点坐标, color颜色
//// mode 模式  mode=1  纵向颜色变化 mode=2 横向颜色变化
//void GUI_boxchange(u8 sx,u16 sy,u8 ex,u16 ey,u16 color,u8 mode)
//{ 
//	u16 temp,temp1,m,n,color2;   
//	Address_set(sx,sy,ex,ey); 
//    n=ex-sx+1;
//	m=ey-sy+1;
//	if(mode==2)color2=color;
//	for(temp=0;temp<m;temp++)
//	{	 	
//		for(temp1=0;temp1<n;temp1++)
//	 	{	
//			Lcd_Write_Data(color);
//			if(mode==2)color+=15;
//		}
//		if(mode==1)color+=15;
//		if(mode==2)color=color2;
//	}
//}



//填充矩形
//x0,y0:矩形的左上角坐标
//width,height:矩形的尺寸
//color:颜色
void GUI_fill_box(u16 x0,u16 y0,u16 width,u16 height,u16 color)
{	  							   
	if(width==0||height==0)return;//非法.	 
	GUI_box(x0,y0,x0+width-1,y0+height-1,color);	   	   
}


//画圆角矩形/填充圆角矩形
//x,y,width,height:圆角矩形的位置和尺寸
//r:圆角的半径.
//upcolor:上半部分颜色
//downcolor:下半部分颜色
void GUI_arcrectangle(u16 x,u16 y,u16 width,u16 height,u8 r,u16 upcolor,u16 downcolor)
{
	u16 btnxh=0;
	if(height%2)btnxh=height+1;//基偶数处理
	else btnxh=height;

 		GUI_fill_box(x+r,y,width-2*r,btnxh/2,upcolor);			    //中上
  		GUI_fill_box(x+r,y+btnxh/2,width-2*r,btnxh/2,downcolor);	//中下
		GUI_fill_box(x,y+r,r,btnxh/2-r,upcolor);					//左上
		GUI_fill_box(x,y+btnxh/2,r,btnxh/2-r,downcolor);			//左下
		GUI_fill_box(x+width-r,y+r,r,btnxh/2-r,upcolor);			//右上
		GUI_fill_box(x+width-r,y+btnxh/2,r,btnxh/2-r,downcolor);	//右下

	GUI_arc(x+r,y+r,r,upcolor,1);//左上	
	GUI_arc(x+r,y+btnxh-r-1,r,downcolor,1);//左下	
	GUI_arc(x+width-r-1,y+r,r,upcolor,1);	 //右上
	GUI_arc(x+width-r-1,y+btnxh-r-1,r,downcolor,1);//右下
}







//显示英文或数字字符
//函数支持 字符的纵向和横向显示 （要配合扫描方式）
//x  y  要显示字符的初始坐标
//value 显示字符数据
//dcolor 显示的字符颜色  bgcolor显示字符的背景颜色
//mode   0 字符纵向显示  1 字符横向显示
void GUI_sprintf_char(u16 x,u16 y,u8 value,u16 dcolor,u16 bgcolor,u8 mode)	
{  
	u8 i,j;
	u8 *temp=zifu816;    //temp是*temp的地址  这里temp就是zifu的首地址 
 	                    
	temp+=(value-32)*16;   //确定要显示的值
				           //用ascii表  前32个ascii没有存入zifu库里 所以要减32
	                       //每个字符用16个字节显示 所以在乘以16  就是对应的显示位的首地址

	  if(mode==0)Address_set(x,y,x+7,y+15);    //设置区域		   
	  if(mode==1)Address_set(x,y,x+7,y+15); //设置区域   		    
		for(j=0;j<16;j++)
		{
			for(i=0;i<8;i++)		    //先横扫
			{ 		     
			 	if((*temp&(1<<(7-i)))!=0)		   //将1 左移 然后对应位进行相与 				
				  Lcd_Write_Data(dcolor);		   //显示字符颜色
				 
				else				
				  Lcd_Write_Data(bgcolor);		   //显示背景颜色		
			}
			temp++;								   //下一字节
		 }


}




//程序汉字显示
//支持横向纵向显示选择功能 mode  	 0 汉字纵向显示  1 汉字横向显示
//说明：汉字是用取模软件制作好的 指定的汉字 汉字大小是16x16	 即32个字节存储一个汉字
// 		这部分汉字是存在程序空间 所以汉字的多少直接影响程序空间的剩余量
//      主要方便于就显示几个指定的汉字来说就不用调用字库了 
//x  y  要显示汉字的坐标
//c[2]  汉字的数据 一个汉字两个字节表示
//dcolor 汉字的颜色   bgcolor 背景颜色
void GUI_sprintf_hz1616(u16 x, u16  y, u8 c[2], u16 dcolor,u16 bgcolor,u8 mode)
{
	u8 i,j,k,m;									//定义临时变量
												

	if(mode==0)Address_set(x,y,  x+16-1, y+16-1);	 // 设置为纵向显示
	if(mode==1)Address_set(x,y,  x+16-1, y+16-1);// 设置为纵向显示

	for (k=0;k<64;k++) 		  //64表示自建汉字库中的个数，循环查询内码	这个数并不确定 取决于存如的汉字个数	  
	{                        
	  if ((codeHZ_16[k].Index[0]==c[0])&&(codeHZ_16[k].Index[1]==c[1]))	    //寻找对应汉字	 一个汉字需要两个字节
	  { 
    	for(i=0;i<32;i++) {								    //32个字节 每个字节都要一个点一个点处理 所以是处理了32X8次
		   m=codeHZ_16[k].Msk[i];							//读取对应字节数据
		  for(j=0;j<8;j++) 									//显示一个字节  一个字节8位 也就是8个点
		   {
			 if((m&0x80)==0x80) 							//判断是否是要写入点 	 如果是 给字体颜色
			   Lcd_Write_Data(dcolor);				
			 else 											//如果不是 为背景色  给颜色
			   Lcd_Write_Data(bgcolor);
				
			 m<<=1;										    //左移一位  判断下一点
		   } 
		  }
		}  
	  }	
	}




//显示汉字字符串 纵向显示
//x1 y1 显示的初始位置
//*str 要显示的数据
//dcolor 显示字符的颜色
//bgcolor 显示字符的背景颜色  
void GUI_sprintf_hzstr16x(u16 x1,u16 y1,u8 *str,u16 dcolor,u16 bgcolor)	  
{  
	 u8 l=0;
	while(*str)
	{	
	  if(*str<0x80)		  //小于128   ascii 都在数组内
	  {	  
		GUI_sprintf_char(x1+l*8,y1,*str,dcolor,bgcolor,0);
		l+=1;
		str++;
		}
	  else
	  {	  
	   PutGB1616(x1+l*8,y1,(u8*)str,dcolor, bgcolor,0);
			str+=2;l+=2;
	  }
	}	
}


//显示汉字及字符  横向显示
//x1 y1 显示的初始位置
//*str 要显示的数据
//dcolor 显示汉字 字符的颜色
//bgcolor 显示汉字 字符的背景颜色  不支持背景屏幕自有色
void GUI_sprintf_hzstr16h(u8 x1,u16 y1,u8 *str,u16 dcolor,u16 bgcolor)	  
{  
	 u8 l=0;
	while(*str)
	{	
	  if(*str<0x80)		  //小于128   ascii 都在数组内
	  {	  
		GUI_sprintf_char(x1,y1+l*8,*str,dcolor,bgcolor,1);
		l+=1;
		str++;
		}
	  else
	  {	  
	   GUI_sprintf_hz1616(x1,y1+l*8,(u8*)str,dcolor, bgcolor,1);
			str+=2;l+=2;
	  }
	}	
}



//显示最大5位的数据  如果非5位状态 其他位不显示 只显示有效值
// x y 显示数值坐标  
//dat 数值
//dcolor  显示数值颜色  bgcolor背景颜色
void number(u8 x,u16 y,u32 dat,u16 dcolor,u16 bgcolor)				//坐标 要显示的数据	   跟踪程序数据用
{
 u8 take[5];
 u8 i,m;

 take[0]=dat/10000%10;
 take[1]=dat/1000%10;
 take[2]=dat/100%10;
 take[3]=dat/10%10;
 take[4]=dat%10;

							  //显示位数
 if(dat/10000)m=5;			  //万位
 else if(dat/1000)m=4;		  //千位
 else if(dat/100)m=3;		  //百位
 else if(dat/10)m=2;		  //十位
 else if(dat/1)m=1;			  //个位
 else if(dat==0)m=1;		  //如果读出来的值就是0 直接用一位表示

 for(i=0;i<m;i++)			  //显示
  GUI_sprintf_char(x+i*8,y,take[5-m+i]+'0',dcolor,bgcolor,0);			  //+'0'表示转换为ACCII码

}





//只显示10位数据
void number10(u8 x,u16 y,u16 dat,u16 dcolor,u16 bgcolor)	//坐标 要显示的数据	 
{

   GUI_sprintf_char(x,y,dat/10%10+'0',dcolor,bgcolor,0);
   GUI_sprintf_char(x+8,y,dat%10+'0',dcolor,bgcolor,0); 
}
	
void GUI_Target_PSO1(u16 color)    // PSO-1形式瞄准线
{

	int i,j,k;

	for (j=100;j<102;j++)  //水平准线
	{
		for (i = 25; i < 80; i++)    //水平准线 左
		{
			GUI_Point(i,j,color);
		}
		for ( i = 160; i < 215; i++)     //水平准线 右
		{
			GUI_Point(i,j,color);
		}
	}	

	for (i=90;i<=150;i=i+10)   //横向竖直分划线
	{
		if (i!=120)
		{
			for (j=100;j<105;j++)
			{
				GUI_Point(i,j,color);
			}
		}
	}

	for (i=117;i<123;i++)   //三角准心
	{
		if (i<120)
		{
			j=219-i;
			for (k=0;k<3;k++)
			{
				GUI_Point(i,j,color);
				j=j+10;
			}
		}
		else
		{
			j=i-20;
			for (k=0;k<3;k++)
			{
				GUI_Point(i,j,color);
				j=j+10;
			}
		}
	}

	for (i=119;i<121;i++)  //竖直准线 下
	{
		for (j=140;j<200;j++)
		{
			GUI_Point(i,j,color);
		}
	} 
}
