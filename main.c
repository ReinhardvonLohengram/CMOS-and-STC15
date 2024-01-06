/* 
TFT����ICΪ  ILI9341
tft
�����ߣ�RS-P5^5;    WR-P4^2;   RD-P4^4;   CS-P5^4;   REST-Ӳ������
������: P0�ڵͰ�λ���ݶ˿�  P2�ڸ߰�λ���ݶ˿ڣ��������ݿڶ���ѡ�� ������������
TFT��ʾ GUI���� ��ʾ�� ���� Բ Բ���ε�ͼ��
������ ���º����ʾ 
	         				
*/


#include  "stc15f2k60s2.h"		  //STC15ͷ�ļ�
#include  "def.h"	    		  //�궨�� ���ú���
#include  "delay.h"				  //��ʱ����
#include  "tft.h"			      //TFT IC�ײ�����
#include  "gui.h"
#include  "xpt2046.h"    //����������
#include  "spi.h"
#include  "key.h"
#include  "pff.h"				  //�ļ�ϵͳ����.h����
#include  "sram.h"
#include  "flash.h"
#include  "sd.h"
#include  "ov7670.h"
#include <string.h>



FATFS fatfs;	                  //�ļ�ϵͳ�ṹ�嶨��
u8 tbuf[512];
u8 	pic_width;				 //ͼƬ���
u16 pic_height;				 //ͼƬ�߶�
u8 mode = 0;           //����ģʽ��0�вο��ߣ�1�޲ο��ߣ�2��׼�ߣ�3�ؿ�
//��� IAP15W4K61S4 ϵ�� IO�ڳ�ʼ��
//io�ڳ�ʼ�� P0 P1 P2 P3 P4 Ϊ׼˫��IO��   
//ע��: STC15W4K32S4ϵ�е�оƬ,�ϵ��������PWM��ص�IO�ھ�Ϊ
//      ����̬,�轫��Щ������Ϊ׼˫��ڻ�ǿ����ģʽ��������ʹ��
//���IO: P0.6/P0.7/P1.6/P1.7/P2.1/P2.2
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



//����Ļ�ӣ�0,0��λ�õ�(240,240)λ�� ��ȡ240x240ͼƬ���� ��ת��ΪBMP��ʽ����TF����
//˵���� Ĭ�ϵ��ļ�����"/OV76/M1.bmp" �������Ҳ���ԣ���������ڿ���������ļ�����
//       д�����ƻ�ʽд��
//����˼·����Ϊpetit fatfs�ļ�ϵͳֻ����ԭ�е��ļ��ϸ��£���BMPͼƬǰ54�ֽ�
//          ��ͼƬ����Ϣ ��һ�����ǲ�Ҫ���� ����ԭ�о���
//          ���Գ�����Ҫ�Ƚ�ǰ54�ֽ�ȡ�� Ȼ����ڸ���54�ֽ��Ժ��ͼƬ����
//ע�⣺	petit fatfs�ļ�ϵͳ ��д����ʱ����Ҫ�� �����Ŀ�ͷд�� ��Ū���������м�д����
//          �����ں�����ȡ��ǰ54�ֽں� �ںͺ������ɫ�������512�ֽ� ������д������
//*path ����·��
//����0 ����ɹ�  1����ʧ��
u8 Show_Bmp(const char *path)
{
	FRESULT res; 

	u16 br,y=0,zy,height,	  //width,heightͼƬ�ĳ�ʼ���
			y1,i1,tmp;		              //tmp 16λ�������
	u8 x=0,zx,width,x1,
			rgb=0, Bmpcolor;

	res=pf_open(path);		             //��ָ��·���ļ�	 ��һ�����Դ��κ�·���µ��ļ� ע�����Ĺ��ܾ��Ǵ��ļ��������ļ���
                                     //���ļ������ݵ�ǰ���Ǳ���Ҫ������ļ�

	if(res == FR_OK)
	{
			pf_read(tbuf, 54, &br);		 //ȡǰ54�ֽ�  ǰ54�ֽں���bmp �ļ���С �ļ�����߶� ����ֵ ��������Ϣ   
			if(br!=54) return 1;		 //��ȡ����

											//ʵ�ʿ�͸߶�����4���ֽڱ�ʾ�ģ���ͼƬ�Ĵ�С���ܳ������ĳߴ�
											//������ֻ��һ���ֽڱ�ʾ���,�����ֽڱ�ʾ�߶�
			width  = tbuf[18];				        //����ͼƬ���	 
			height = (tbuf[23]<<8)+tbuf[22];	//����ͼƬ�߶�
		
			pic_width  = width;
			pic_height = height;

			Bmpcolor=tbuf[28]/8;					//��ȡ��ɫ��� һ����16λ 24λ 32λ  
			//number(30,280,Bmpcolor,Red,White);
			//��С����Ļ�ߴ��ͼƬ�ŵ���Ļ���м� ��ͷ��ʾ
			if(width<239)   zx=(240-width)/2;         else zx=0;
			if(height<299)	zy=(320-height);        	else zy=0;
			
			x1=zx; y1=zy;			   //��ֵ������ֵ

			LCD_scan(2);	  //BMPͼƬ�����ɨ�跽ʽΪ ������   ���µ��� ������ʾ��ͼƬ���µߵ�

			Address_set(x1,y1,x1+width-1,y1+height-1);         //������ʾ��Χ ��ɨ���� ��ɨ����
			LCD_RS=1;    //д����������	 	  Ϊ���д���ٶ� ��ѭ��ǰ����
			while(1)                   //һֱ�����һ��
			{
		  		 pf_read(tbuf, 512, &br);		 //��54�ֽں�λ�ö�ȡ512���ֽڵ�������  
					 for(i1=0;i1<512;i1++)
						{
							if(Bmpcolor==2)				 //16λBMP
							{			
								switch(rgb)				 //555ת565��ʽ
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
							else if(Bmpcolor==3)		//24λBMP���� RGB�ֱ�ռ8���ֽ�
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
								P2=tmp>>8;								  //Ϊ�������ʾ�ٶ� ֱ�ӵ���IO�ڱ���
								P0=tmp;									  //void Lcd_Write_Data(u16 Data)�����ķֽ�
								tmp=0;
								rgb=0;
								LCD_WR=0;								  //��ʼд��
								LCD_WR=1;			  	       
	
								x1++;							  //�����Լ� ����һ�� �������� �����Լ� ֱ��ɨ������ͼƬ
								if(x1==width+zx)                  
								{	 
									y1--;
									x1=zx;
									if(y1==zy-height)
									{									 //�ָ�����ɨ�跽ʽ
										LCD_scan(1);
										return 0;    //��ʾ���	
									}
								}
							}
						} 
			}
  }
	return 1;   //����
}		    
void scan(void)//lcd��Ļɨ��
{
	u32 j;
	Address_set(0,0,239,239);	  //������ʾ��Χ  ��ʾΪ240*240 
  if(cur_status==2)			   //�жϻ������Ƿ�����������
  { 
		
			FIFO_RRST=0;			   //��ʼ��λ��ָ��	��ȡFIFO����
      FIFO_RCLK=0;
      FIFO_RCLK=1;
      FIFO_RCLK=0;
      FIFO_RRST=1;			   //��λ��ָ�����
      FIFO_RCLK=1;
			LCD_RS=1;				   //����TFT д���ݶ� 

	  for(j =0;j<57600;j++)	   //�ֱ���Ϊ240x240   ÿ����ɫ��Ҫ�����ֽ� ���� 240x240x2=57600  ��
	  {		 
			FIFO_RCLK=0;		   //ÿһ��ʱ������ ��һ������		
			P2=P1;				   //ֱ�ӽ����ֽ����ݸ�P2
			FIFO_RCLK=1; 

			FIFO_RCLK=0;
			P0=P1;	           	   //ֱ�ӽ����ֽ����ݸ�P0
			FIFO_RCLK=1; 	


			LCD_WR=0;			   // ��ʾ
      LCD_WR=1;		
	  } 
		EX0 = 1; 				   //��ʾ��ɿ��ж�
	  cur_status=0;			   //��ʾ������ͼƬ�Ժ� ��	 cur_status��0  ׼��������һ֡
	}
}
u8 Save_Bmp(const char *path)
{
	FRESULT res;
	u16 br,i,j,m=0,n=239,color;

	sram(1);							 //�����ⲿ�ڴ�	   
                                     //�����ļ�·�����ⲿSRAM�� ��������Ҫ�����ⲿSRAM���ܵ���·��
	res=pf_open(path);		             //��ָ��·���ļ�	 ��һ�����Դ��κ�·���µ��ļ� ע�����Ĺ��ܾ��Ǵ��ļ��������ļ���
                                     //���ļ������ݵ�ǰ���Ǳ���Ҫ������ļ�

	if(res == FR_OK)
	{
			led = 0; //����ָʾ��(P3.5)��
		
			pf_read(tbuf,54,&br);				 	//��ȡBMPͼƬǰ54�ֽ�ͼƬ��Ϣ
			pf_open(path);		 						//���´�·�� ������ָ��ͼƬ������λ��
			sram(0);							 			 	//�ر��ⲿ�ڴ棬����Һ��Ƭѡ
		
			for(i=27;i<256;i++)				 		//��ȡ512���ֽڵ�tbuf�� ��256����ɫ��
      {
					color=LCD_readpoint(m,n);	//��ȡLCDÿ����Ԫ����ɫ
					color=((color>>1)&0x7fe0)+(color&0x001f);	//����ȡ��565��ʽת��Ϊ555��ʽ

					tbuf[i*2]=color;				 	//����ʱ���ֽ���ǰ
					tbuf[i*2+1]=(color>>8);
					m++;
      }
					
			pf_write(tbuf,512,&br);			 	//��TF����д��512���ֽڣ�1��������

			
     for(j=0;j<254;j++)
		 {
	     for(i=0;i<256;i++)				 		//��ȡ512���ֽڵ�tbuf�� ��256����ɫ��
	     {
					color=LCD_readpoint(m,n);	//��������ͷ����Ч��  ��ȡ��ɫ
					color=((color>>1)&0x7fe0)+(color&0x001f); //����ȡ��565��ʽת��Ϊ555��ʽ

	        tbuf[i*2]=color;				 	//����ʱ���ֽ���ǰ
					tbuf[i*2+1]=(color>>8);
	
					m++;
					if(m==240)
					{
						m=0;
						n--;	   		//���ﲻ���ж�m ��Ϊѭ���̶� ֱ�ӻ����� 
					}
	     }	  
	     pf_write(tbuf,512,&br);			 //��TF����д��512���ֽ�		 
		 }

		SD_DisSelect();//ȡ��TF��Ƭѡ  ��д�뺯�����ȡ��Ƭѡ ����Ӱ�� ���������д����ɼ�ȡ��Ƭѡ
	 
		delay1ms(2000);    //������ʾ��ʱ
		led = 1; //����ָʾ��(P3.5)��
		return 0;  //д��ɹ�
	}
	else
	{
		return 1;    //����
	}
}
//
//��������ת����ʵ�����꺯����
//���� result.x���� result.y���� result.flag=1��ʾ�м�����
//
void GetTouchScreenPos(struct TFT_Pointer * sp)
{
		u16 a,b, flag;				//��ʱ����

		u8  ax[8];
		u16 x1,y1;
		u16 x2,y2;

		u8 i2t[8];			   //��24c02�� ����У׼���� ��ʱת����������
		
	//������У׼����
		struct T_i T_i2c=
		{
			656,			 
			883,
			-13,
			-12,
		};
	
		//��ȡADֵ��ת��ΪX Y����
#define ERR_RANGE 5 //��Χ 
  
		if(AD7843_isClick==0)
		{	 
			delay1ms(1);
			if(AD7843_isClick==0)
			{
				LCD_CS=1;			//xpt��Ƭѡ����tft�� ��ֹ��������ʱӰ��tft	����ص�TFTʹ��
				AD7843_CS=0; 		//��Ƭѡ
				SPI_Speed(2);		//���� SPIͨѶ�ٶ� ʹ����оƬ�Ż����ݸ��ȶ�
				/*�������16ʱ�����ڲɼ�  ��Ϊ �˴������ܲ��õ���SPI����
					��SPI������ֻ��8λ����  XPT2046��AD�ֱ���Ϊ12λ  ��Ҫ������
					����XPT2046�ֲ��� 16ʱ������ ʱ��ͼ ���Կ���
					���Ͳɼ�����  ����һ��SPI���ݺ� �ڷ��Ϳչ��ܵ�SPI����  �ͻ��ʣ�µĲ��ֽ��յ�
					�����Ƚ��յ� �ǵ��ֽ�����  �ڶ��ν��յ��Ǹ��ֽ�����  ��λ�� ����12λ��ADֵ   
				*/
				ax[0]=SPI_SendByte(0x90);  //�Ϳ�����10010000���ò�ַ�ʽ��X���꣬������ȡ������
				ax[0]=SPI_SendByte(0x00);  //�����������ݣ����λ����Ϊ1����2046�����ͻ��������X���ֽ�
				ax[1]=SPI_SendByte(0xD0);  //�Ϳ�����11010000���ò�ַ�ʽ��Y���꣬����X���ֽ�
				ax[2]=SPI_SendByte(0x00);  //�����������ݣ�ͬ�ϣ�������Y���ֽ�
				ax[3]=SPI_SendByte(0x90);  //�Ϳ�����10010000 ���ڶ��Σ���X���꣬����Y���ֽ�
				ax[4]=SPI_SendByte(0x00);  //�����������ݣ�ͬ�ϣ�������X���ֽ�
				ax[5]=SPI_SendByte(0xD0);  //�Ϳ�����11010000  ���ڶ��Σ���Y���꣬����X���ֽ�
				ax[6]=SPI_SendByte(0x00);  //�����������ݣ�ͬ��)������Y���ֽ�
				ax[7]=SPI_SendByte(0x90);  //�Ϳ�����10010000  �������Σ���X���꣬����Y���ֽ�

				//��ȡ���βɼ�ֵ
				y1=(ax[0]<<5)|(ax[1]>>3);
				y2=(ax[4]<<5)|(ax[5]>>3);
				x1=(ax[2]<<5)|(ax[3]>>3);
				x2=(ax[6]<<5)|(ax[7]>>3);

				if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-ERR_RANGE��
						&&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
				{
					flag=1;			//�򿪱�־λ
					a=(x1+x2)/2;
					b=(y1+y2)/2;
				}
				else flag=0;

				SPI_Speed(0);		//����SPI�ٶ�Ϊ���
				AD7843_CS=1; 		//��Ƭѡ
				LCD_CS=0;
			}
		} 

	
		/* ���������㹫ʽ
   lcdx=xa*tpx+xb;
   lcdy=ya*tpy+yb;
   lcdx,lcdyΪ������  tpx,tpyΪ������ֵ xa,yaΪ��������  xb,ybΪƫ����

   ���㷽�� 
   ����Ļ��ָ��lcdx,lcdyλ�û���ʮ��ͼ�� �ֱ�����Ļ�ϵ�4����λ��
   �ô����ʷֱ��� �õ����еĴ���ֵ
   ��������Ĺ�ʽ ����	xa,ya  xb,yb  ��������ʹ�ô��������ĻУ׼
		*/
		//��У׼����  
		//		 result.x=0.065894*a-16;			//���õ���ADֵ���빫ʽ ����lcd����x y���� 
		//		 result.y=0.084031*b-14;
		
		//����У׼����  
		 sp->x = ((float)T_i2c.xi/10000)*a+T_i2c.a;			//���õ���ADֵ���빫ʽ ����lcd����x y���� 
		 sp->y = ((float)T_i2c.yi/10000)*b+T_i2c.b;
		 sp->flag = flag;
}
// ��ȾͼƬ
/*u8 Replay_Show_Bmp(char *path)
{
	FRESULT res; 

	u16 br,y=0,zy,height,	  //width,heightͼƬ�ĳ�ʼ���
			y1,i1,tmp;		              //tmp 16λ�������
	u8 x=0,zx,width,x1,
			rgb=0, Bmpcolor;

	res=pf_open(path);		             //��ָ��·���ļ�	 ��һ�����Դ��κ�·���µ��ļ� ע�����Ĺ��ܾ��Ǵ��ļ��������ļ���
                                     //���ļ������ݵ�ǰ���Ǳ���Ҫ������ļ�

	if(res == FR_OK)
	{
			pf_read(tbuf, 54, &br);		 //ȡǰ54�ֽ�  ǰ54�ֽں���bmp �ļ���С �ļ�����߶� ����ֵ ��������Ϣ   
			if(br!=54) return 1;		 //��ȡ����

											//ʵ�ʿ�͸߶�����4���ֽڱ�ʾ�ģ���ͼƬ�Ĵ�С���ܳ������ĳߴ�
											//������ֻ��һ���ֽڱ�ʾ���,�����ֽڱ�ʾ�߶�
			width  = tbuf[18];				        //����ͼƬ���	 
			height = (tbuf[23]<<8)+tbuf[22];	//����ͼƬ�߶�
		
			pic_width  = width;
			pic_height = height;

			Bmpcolor=tbuf[28]/8;					//��ȡ��ɫ��� һ����16λ 24λ 32λ  
			//number(30,280,Bmpcolor,Red,White);
			//��С����Ļ�ߴ��ͼƬ�ŵ���Ļ���м� ��ͷ��ʾ
			if(width<239)   zx=(240-width)/2;         else zx=0;
			if(height<299)	zy=(320-height);        	else zy=0;
			
			x1=zx; y1=zy;			   //��ֵ������ֵ

			LCD_scan(2);	  //BMPͼƬ�����ɨ�跽ʽΪ ������   ���µ��� ������ʾ��ͼƬ���µߵ�

			Address_set(x1,y1,x1+width-1,y1+height-1);         //������ʾ��Χ ��ɨ���� ��ɨ����
			LCD_RS=1;    //д����������	 	  Ϊ���д���ٶ� ��ѭ��ǰ����
			while(1)                   //һֱ�����һ��
			{
		  		 pf_read(tbuf, 512, &br);		 //��54�ֽں�λ�ö�ȡ512���ֽڵ�������  
					 for(i1=0;i1<512;i1++)
						{
							if(Bmpcolor==2)				 //16λBMP
							{			
								switch(rgb)				 //555ת565��ʽ
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
							else if(Bmpcolor==3)		//24λBMP���� RGB�ֱ�ռ8���ֽ�
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
								P2=tmp>>8;								  //Ϊ�������ʾ�ٶ� ֱ�ӵ���IO�ڱ���
								P0=tmp;									  //void Lcd_Write_Data(u16 Data)�����ķֽ�
								tmp=0;
								rgb=0;
								LCD_WR=0;								  //��ʼд��
								LCD_WR=1;			  	       
	
								x1++;							  //�����Լ� ����һ�� �������� �����Լ� ֱ��ɨ������ͼƬ
								if(x1==width+zx)                  
								{	 
									y1--;
									x1=zx;
									if(y1==zy-height)
									{									 //�ָ�����ɨ�跽ʽ
										LCD_scan(1);
										return 0;    //��ʾ���	
									}
								}
							}
						} 
			}
  }
	return 1;   //����
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
	u32  j;					  			//��ʱ����
	u8  sign=0;			  			//��ʼ��־
	u8 cankao=1;              //�ο��߿��ر�־
	struct TFT_Pointer sp;  //���崥�������ṹ��
	struct TFT_Pointer tp; //������鴥�����Ƿ��м�����
	/*  ��ʼ���׶� */
  SP=0X80;				      	// ������ջָ��   �ֲ�286ҳ ���
  IO_init();				  	  // ��� IAP15W4K61S4  IO�ڳ�ʼ��
  Lcd_Init();             // TFTҺ����ʼ��
  Init_SPI(); 			      // SPI��ʼ��
  GUI_Clear(Black);			  // ��ɫ���� 
  SD_Init();			      	// SD����ʼ��
  KEY_Init();	
  pf_mount(&fatfs);	      //��ʼ��petit FATFS�ļ�ϵͳ  ����ȡtf����Ӧ����
                          //�����ǳ���Ҫ������ʹ������Petit Fatfs�ļ�ϵͳ���ܵ�ǰ�ᣩ

  mem_init();				  		//�ⲿSRAM��ʼ��




	 P1M0=0X00;	  //P1��Ϊ������״̬
	 P1M1=0Xff;		
	
	/* �����ж�Ŀ��Ϊ�ж�VSYNC��֡�����жϣ������д�����ʱ��Ϊ��һ֡
	   ��ʱ��ʼ������ͷFIFO����оƬAL422B����һ֡���ݡ�
		 �����ڶ�֡ʱ˵�����ݹ�����ɣ���ʱ��ȡ���ݽ�����ʾ�� */
  IT0=1;			   //���ش���   
  EX0=1;         //�ⲿ�ж�0   P3.2��    
  EA =1;      //�����ж� 
	
  /* ��Һ����ʾ���ú�ɫ��� */
  GUI_Clear(Black);
  LCD_scan(1);  //Һ��������ɨ�跽ʽ�����ϵ��� ���ҵ���
  
	

	/* OV7670��ʼ�� */
  if( Ov7670_init_normal()) 
  {															 //��ʼ�����ɹ�
   GUI_sprintf_hzstr16x(60,150,"OV7670��ʼ��",White,Blue);
   delay1ms(2000);
   EX0=0;			   //�ر��ⲿ�ж�0
   EA=0;			   //�ر����ж�
   sign=1;
  }

	if (SD_Init()) 							//SD����ʼ�������ɹ��˳�
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //��ʼ��ʧ������ ��û�в�OV7670 ����û��ã� 

  FIFO_OE=0;		   //ʹ������ͷģ��

  OV7670_Window_Set(10,176,240,240);	//������ʾ���ڳߴ磺240*240����Ƶ��10����Ƶ��176

  
	
  IO_init();				   //�� IAP15W4K61S4  IO�ڳ�ʼ��
	Lcd_Init();          //tft��ʼ��
	Init_SPI(); 				 //������SPI�ӿڳ�ʼ��
while(1)
{	
	GUI_Clear(Black);
 /* ��ʼɨ�� ��ʾ����ͷ�ɼ����� */
 while(mode==0)//mode==0,�ο�������
 {
	  
    scan();
		GUI_arc(120,269,25,White,1);	 //120��269λ�û�һ���뾶Ϊ25��ʵ��Բ
		GUI_box(20,245,68,293,White);//���½ǻ�һ��48*48��ʵ�İ�ɫ����
		GUI_sprintf_hzstr16x(20,263,"replay", Red, White); //���ε�λ��д��ɫ��

		GUI_line(80,0,80,240,White);//�ο���
		GUI_line(160,0,160,240,White);
		GUI_line(0,80,240,80,White);
		GUI_line(0,160,240,160,White);
		
		GUI_tri(180,245,180,293,White);//������
		GetTouchScreenPos(&tp); //�ɼ�������
		
		if (tp.flag == 1)           //�Ƿ��д����¼����� 
    { 
			tp.flag = 0;
			if ((tp.x>20) && (tp.x<68) && (tp.y>245) && (tp.y<293)) //������½Ǿ���
			{
				led=0;
				delay1ms(100);//�ӳ�0.1s
				led=1;
				mode=3;
				delay1ms(100);
				break;
			}
			
			if ((tp.x>100) && (tp.x<140) && (tp.y>249) && (tp.y<288)) //���Բ��,��ɨ��һ�Σ��޲ο��ߣ���������
			{   
					for(j = 0;j<5;j++)
					{
						 scan();
		GUI_arc(120,269,25,White,1);	 //120��269λ�û�һ���뾶Ϊ25��ʵ��Բ
		GUI_box(20,245,68,293,White);//���½ǻ�һ��48*48��ʵ�İ�ɫ����
		GUI_sprintf_hzstr16x(20,263,"replay", Red, White); //���ε�λ��д��ɫ��

		GUI_line(80,0,80,240,White);//�ο���
		GUI_line(160,0,160,240,White);
		GUI_line(0,80,240,80,White);
		GUI_line(0,160,240,160,White);
		
		GUI_tri(180,245,180,293,White);//������
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
			if ((tp.x>180) && (tp.x<220) && (tp.y>220) && (tp.y<295)) //���������,�ı�ģʽ
			{
				led=0;
				delay1ms(100);//�ӳ�0.1s
				led=1;
				if(mode+1<=3)
				{
					mode ++;
					  EX0=1;         //�ⲿ�ж�0   P3.2��    
  EA =1;      //�����ж� 
	
  /* ��Һ����ʾ���ú�ɫ��� */
  GUI_Clear(Black);
  LCD_scan(1);  //Һ��������ɨ�跽ʽ�����ϵ��� ���ҵ���
					if(Ov7670_init_fupian()) 
  {															 //��ʼ�����ɹ�
   GUI_sprintf_hzstr16x(60,150,"OV7670��ʼ��",White,Blue);
   delay1ms(2000);
   EX0=0;			   //�ر��ⲿ�ж�0
   EA=0;			   //�ر����ж�
   sign=1;
  }
	if (SD_Init()) 							//SD����ʼ�������ɹ��˳�
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //��ʼ��ʧ������ ��û�в�OV7670 ����û��ã� 

  FIFO_OE=0;		   //ʹ������ͷģ��

  OV7670_Window_Set(10,176,240,240);	//������ʾ���ڳߴ磺240*240����Ƶ��10����Ƶ��176
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
 
		while(mode==1)//mode==1,�޲ο�������
 {
	 
    scan();
		GUI_arc(120,269,25,White,1);	 //120��269λ�û�һ���뾶Ϊ25��ʵ��Բ
		GUI_box(20,245,68,293,White);//���½ǻ�һ��48*48��ʵ�İ�ɫ����
		GUI_sprintf_hzstr16x(20,263,"replay", Red, White); //���ε�λ��д��ɫ��
		GUI_tri(180,245,180,293,White);//������
	  //GUI_Target_PSO1(Black);//��׼
		GetTouchScreenPos(&tp); //�ɼ�������
		
		if (tp.flag == 1)           //�Ƿ��д����¼����� 
    { 
			tp.flag = 0;
			if ((tp.x>20) && (tp.x<68) && (tp.y>245) && (tp.y<293)) //������½Ǿ���
			{
				led=0;
				delay1ms(100);//�ӳ�0.1s
				led=1;
				mode=3;
				delay1ms(100);
				break;
			}
			
			if ((tp.x>100) && (tp.x<140) && (tp.y>249) && (tp.y<288)) //���Բ��,��ɨ��һ�Σ��޲ο��ߣ���������
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
			if ((tp.x>180) && (tp.x<220) && (tp.y>220) && (tp.y<295)) //���������,�ı�ģʽ
			{
				led=0;
				delay1ms(100);//�ӳ�0.1s
				led=1;
				if(mode+1<=3)
				{
					mode ++;
					  EX0=1;         //�ⲿ�ж�0   P3.2��    
  EA =1;      //�����ж� 
	
  /* ��Һ����ʾ���ú�ɫ��� */
  GUI_Clear(Black);
  LCD_scan(1);  //Һ��������ɨ�跽ʽ�����ϵ��� ���ҵ���
					if(Ov7670_init_fanzhuan()) 
  {															 //��ʼ�����ɹ�
   GUI_sprintf_hzstr16x(60,150,"OV7670��ʼ��",White,Blue);
   delay1ms(2000);
   EX0=0;			   //�ر��ⲿ�ж�0
   EA=0;			   //�ر����ж�
   sign=1;
  }
	if (SD_Init()) 							//SD����ʼ�������ɹ��˳�
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //��ʼ��ʧ������ ��û�в�OV7670 ����û��ã� 

  FIFO_OE=0;		   //ʹ������ͷģ��

  OV7670_Window_Set(10,176,240,240);	//������ʾ���ڳߴ磺240*240����Ƶ��10����Ƶ��176
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
 
		while(mode==2)//mode==2,��׼������
 {
    scan();
		GUI_arc(120,269,25,White,1);	 //120��269λ�û�һ���뾶Ϊ25��ʵ��Բ
		GUI_box(20,245,68,293,White);//���½ǻ�һ��48*48��ʵ�İ�ɫ����
		GUI_sprintf_hzstr16x(20,263,"replay", Red, White); //���ε�λ��д��ɫ��
		GUI_tri(180,245,180,293,White);//������
	  GUI_Target_PSO1(Black);//��׼
		GetTouchScreenPos(&tp); //�ɼ�������
		
		if (tp.flag == 1)           //�Ƿ��д����¼����� 
    { 
			tp.flag = 0;
			if ((tp.x>20) && (tp.x<68) && (tp.y>245) && (tp.y<293)) //������½Ǿ���
			{
				led=0;
				delay1ms(100);//�ӳ�0.1s
				led=1;
				mode=3;
				delay1ms(100);
				break;
			}
			
			if ((tp.x>100) && (tp.x<140) && (tp.y>249) && (tp.y<288)) //���Բ��,��ɨ��һ�Σ��޲ο��ߣ���������
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
			if ((tp.x>180) && (tp.x<220) && (tp.y>220) && (tp.y<295)) //���������,�ı�ģʽ
			{
				led=0;
				delay1ms(100);//�ӳ�0.1s
				led=1;
				if(mode+1<=3)
				{
					mode ++;
				}
				else
				{
					mode = 0;
					mode ++;
					  EX0=1;         //�ⲿ�ж�0   P3.2��    
  EA =1;      //�����ж� 
	
  /* ��Һ����ʾ���ú�ɫ��� */
  GUI_Clear(Black);
  LCD_scan(1);  //Һ��������ɨ�跽ʽ�����ϵ��� ���ҵ���
					if(Ov7670_init_normal()) 
  {															 //��ʼ�����ɹ�
   GUI_sprintf_hzstr16x(60,150,"OV7670��ʼ��",White,Blue);
   delay1ms(2000);
   EX0=0;			   //�ر��ⲿ�ж�0
   EA=0;			   //�ر����ж�
   sign=1;
  }
	if (SD_Init()) 							//SD����ʼ�������ɹ��˳�
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //��ʼ��ʧ������ ��û�в�OV7670 ����û��ã� 

  FIFO_OE=0;		   //ʹ������ͷģ��

  OV7670_Window_Set(10,176,240,240);	//������ʾ���ڳߴ磺240*240����Ƶ��10����Ƶ��176
				}				
					delay1ms(100);
				break;
			}
					
			} 
		}

if(mode==3)//mode==3,�ؿ�����
 {
	  GUI_Clear(Black);
		GUI_box(20,245,68,293,White);//���½ǻ�һ��48*48��ʵ�İ�ɫ����
		GUI_sprintf_hzstr16x(20,263,"home", Red, White); //���ε�λ��д��
		GUI_box(96,245,144,293,White);//
		GUI_sprintf_hzstr16x(96,263,"last", Red, White); //���ε�λ��д��
		GUI_box(172,245,220,293,White);	 //120��269λ�û�һ���뾶Ϊ25��ʵ��Բ
	  GUI_sprintf_hzstr16x(172,263,"next", Red, White); //���ε�λ��д��
		
	  Show_Bmp(fname[current]); //��ʾ��һ��ͼƬ
	 GUI_sprintf_hzstr16x(x, y, fname[current], Black, White);
						x += 8*strlen(fname[current])+20;
						number(x, y, pic_width, Black, White); //ͼƬ�Ŀ��
						x += 30;
						GUI_sprintf_hzstr16x(x, y, "x", Black, White);
						x += 16;
						number(x, y, pic_height, Black, White); //ͼƬ�ĸ߶�
	while(1){	
		GetTouchScreenPos(&tp); //�ɼ�������
	 if (tp.flag == 1)           //�Ƿ��д����¼����� 
    { 
			tp.flag = 0;
			if ((tp.x>20) && (tp.x<68) && (tp.y>245) && (tp.y<293)) //������½Ǿ��Σ��ص�ģʽ0
			{
				led=0;
				delay1ms(100);//�ӳ�0.1s
				led=1;
				mode=0;
				mode ++;
					  EX0=1;         //�ⲿ�ж�0   P3.2��    
  EA =1;      //�����ж� 
	
  /* ��Һ����ʾ���ú�ɫ��� */
  GUI_Clear(Black);
  LCD_scan(1);  //Һ��������ɨ�跽ʽ�����ϵ��� ���ҵ���
					if(Ov7670_init_normal()) 
  {															 //��ʼ�����ɹ�
   GUI_sprintf_hzstr16x(60,150,"OV7670��ʼ��",White,Blue);
   delay1ms(2000);
   EX0=0;			   //�ر��ⲿ�ж�0
   EA=0;			   //�ر����ж�
   sign=1;
  }
	if (SD_Init()) 							//SD����ʼ�������ɹ��˳�
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card Error!",Red, White); 
		return;
	}			      
	else
	{
		GUI_sprintf_hzstr16x(90,20,"TF Card OK!",Black, White);
	}
	
  if(sign==1) return;		   //��ʼ��ʧ������ ��û�в�OV7670 ����û��ã� 

  FIFO_OE=0;		   //ʹ������ͷģ��

  OV7670_Window_Set(10,176,240,240);	//������ʾ���ڳߴ磺240*240����Ƶ��10����Ƶ��176
				delay1ms(100);
				break;
			}
			if ((tp.x>96) && (tp.x<144) && (tp.y>245) && (tp.y<293)) //����м�,�л���Ƭ
			{
				GUI_Clear(Black);
		GUI_box(20,245,68,293,White);//���½ǻ�һ��48*48��ʵ�İ�ɫ����
		GUI_sprintf_hzstr16x(20,263,"home", Red, White); //���ε�λ��д��
		GUI_box(96,245,144,293,White);//
		GUI_sprintf_hzstr16x(96,263,"last", Red, White); //���ε�λ��д��
		GUI_box(172,245,220,293,White);	 //120��269λ�û�һ���뾶Ϊ25��ʵ��Բ
	  GUI_sprintf_hzstr16x(172,263,"next", Red, White); //���ε�λ��д��
				led=0;
				delay1ms(100);//�ӳ�0.1s
				led=1;
				if(current==1)
					{
						current=30;
					}
					else
					{
						current--;
					}		//��һ��
				 if(Show_Bmp(fname[current])==0)	  //��ʾָ��·���µ�bmp�ļ�	   
                              //����Ҫ��֪����TF�����·�� "/ov76/M1.bmp"������TF����Ŀ¼���ļ���Ϊ����è.bmp�ļ�
					{
						x=0;
						y=300;
						GUI_sprintf_hzstr16x(x, y, fname[current], Black, White);
						x += 8*strlen(fname[current])+20;
						number(x, y, pic_width, Black, White); //ͼƬ�Ŀ��
						x += 30;
						GUI_sprintf_hzstr16x(x, y, "x", Black, White);
						x += 16;
						number(x, y, pic_height, Black, White); //ͼƬ�ĸ߶�
					}
				  
			}
			if ((tp.x>172) && (tp.x<220) && (tp.y>245) && (tp.y<293)) //�������,�л���Ƭ
			{
				GUI_Clear(Black);
		GUI_box(20,245,68,293,White);//���½ǻ�һ��48*48��ʵ�İ�ɫ����
		GUI_sprintf_hzstr16x(20,263,"home", Red, White); //���ε�λ��д��
		GUI_box(96,245,144,293,White);//
		GUI_sprintf_hzstr16x(96,263,"last", Red, White); //���ε�λ��д��
		GUI_box(172,245,220,293,White);	 //120��269λ�û�һ���뾶Ϊ25��ʵ��Բ
	  GUI_sprintf_hzstr16x(172,263,"next", Red, White); //���ε�λ��д��
				led=0;
				delay1ms(100);//�ӳ�0.1s
				led=1;
				 if(current==30)
					{
						current=0;
					}
					else
					{
						current++;
					}		 	//��һ��
					if(Show_Bmp(fname[current])==0)	  //��ʾָ��·���µ�bmp�ļ�	   
                              //����Ҫ��֪����TF�����·�� "/ov76/M1.bmp"������TF����Ŀ¼���ļ���Ϊ����è.bmp�ļ�
					{
						x=0;
						y=300;
						GUI_sprintf_hzstr16x(x, y, fname[current], Black, White);
						x += 8*strlen(fname[current])+20;
						number(x, y, pic_width, Black, White); //ͼƬ�Ŀ��
						x += 30;
						GUI_sprintf_hzstr16x(x, y, "x", Black, White);
						x += 16;
						number(x, y, pic_height, Black, White); //ͼƬ�ĸ߶�
					}
			}		
		}
	}		
	}			
	
 }
}
 
