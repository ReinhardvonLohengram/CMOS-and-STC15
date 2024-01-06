
#include "stc15f2k60s2.h"
#include "sd.h"
#include "spi.h"
#include "delay.h"



u8  SD_Type=0;//SD�������� 


///////////////////////////////////////////////////////////////////////////////////


//�ȴ���׼����
//����ֵ:0,׼������;����,�������
u8 SD_WaitReady(void)
{
	u32 t=0;
	do
	{
		if(SPI_SendByte(0XFF)==0XFF)return 0;//OK
		t++;		  	
	}while(t<0XFFFFFF);//�ȴ� 
	return 1;
}

//ȡ��ѡ��,�ͷ�SPI����
void SD_DisSelect(void)
{
	SD_CS=1;
 	SPI_SendByte(0xff);//�ṩ�����8��ʱ��
}

//ѡ��sd��,���ҵȴ���׼��OK
//����ֵ:0,�ɹ�;1,ʧ��;
u8 SD_Select(void)
{
	SD_CS=0;
	if(SD_WaitReady()==0)return 0;//�ȴ��ɹ�
	SD_DisSelect();
	return 1;//�ȴ�ʧ��
}

//��SD������һ������
//����: u8 cmd   ���� 
//      u32 arg  �������
//      u8 crc   crcУ��ֵ	   
//����ֵ:SD�����ص���Ӧ															  
u8 SD_SendCmd(u8 cmd, u32 arg, u8 crc)
{
    u8 r1;	
	u8 Retry=0; 

	SD_DisSelect();//ȡ���ϴ�Ƭѡ
	if(SD_Select())return 0XFF;//ƬѡʧЧ 


	//����
	SD_CS=1;		                                //Ƭѡ����																		 
	SPI_SendByte(0xff);
	SPI_SendByte(0xff);	 
	SPI_SendByte(0xff);							    
	SD_CS=0; 

    SPI_SendByte(cmd | 0x40);       //�ֱ�д������	���2λ�̶�Ϊ1  
    SPI_SendByte(arg >> 24);		//������� 2-5�ֽ�  4���ֽ� 32λ
    SPI_SendByte(arg >> 16);
    SPI_SendByte(arg >> 8);
    SPI_SendByte(arg);	  
    SPI_SendByte(crc); 
	                                 //ֹͣ����������
	if(cmd==CMD12)SPI_SendByte(0xff);//Skip a stuff byte when stop reading
    //�ȴ���Ӧ����ʱ�˳�
	Retry=0X1F;		   //ѭ��32��
	do
	{
		r1=SPI_SendByte(0xFF);
	}while((r1&0X80) && Retry--);	 
	//����״ֵ̬
    return r1;
}







//��ʼ��SD��
u8 SD_Init(void)
{
    u8 r1;      // ���SD���ķ���ֵ
    u16 retry;  // �������г�ʱ����
    u8 buf[4];  
	u8 i;
	
 	SPI_Speed(3);		//���õ�����ģʽ 

 	for(i=0;i<10;i++)SPI_SendByte(0XFF);//��������74������ �ȴ�SD���ڲ������ѹ����ʱ��	  ����SPIģʽ

	
	retry=20;
	do
	{
		r1=SD_SendCmd(CMD0,0,0x95);//����IDLE״̬ 	 ����λsd�� ����״̬
	}while((r1!=0X01) && retry--);

	 
 	SD_Type=0;//Ĭ���޿�
	 
	if(r1==0X01)   //��ȡ�汾��Ϣ
	{	
		if(SD_SendCmd(CMD8,0x1AA,0x87)==1)//SD V2.0	  ���ͽӿ�״̬����		                                  
		{							      //�������ֵΪ1 ���� SDV2.0�汾
			for(i=0;i<4;i++)buf[i]=SPI_SendByte(0XFF);	//Get trailing return value of R7 resp
														//��ȡ����ֵR7����
			if(buf[2]==0X01&&buf[3]==0XAA)//���Ƿ�֧��2.7~3.6V
			{  
				retry=0XFFFE;
				do
				{
					SD_SendCmd(CMD55,0,0X01);	//����CMD55
					r1=SD_SendCmd(CMD41,0x40000000,0X01);//����CMD41
				}while(r1&&retry--);
				if(retry&&SD_SendCmd(CMD58,0,0X01)==0)//����SD2.0���汾��ʼ
				{	
					for(i=0;i<4;i++)buf[i]=SPI_SendByte(0XFF);//�õ�OCRֵ
					if(buf[0]&0x40)SD_Type=SD_TYPE_V2HC;    //���CCS
					else SD_Type=SD_TYPE_V2;  					     						
				}
				else SD_Type=SD_TYPE_ERR;//����Ŀ�
			}
		}
	
	}

	SD_DisSelect();//ȡ��Ƭѡ
	SPI_Speed(0);//����	
	if(SD_Type)return 0;	//���û�вɼ���SD���汾 ����������
	else if(r1)return r1; 	   
	return 0xaa;//��������
}




//��ָ��������SD��һ�������� ��512�ֽ�
//buf:���ݻ�����
//sector:����
//����ֵ:0,ok;����,ʧ��. 
u8 SD_read_sector(u32 sector,u8 *buf) 
{ 
    u16 i;
	u8 r1;
	SD_DisSelect();//ȡ���ϴ�Ƭѡ
 	if(SD_Select())return 0XFF;//ƬѡʧЧ 

    if(SD_Type!=SD_TYPE_V2HC)sector <<= 9;//ת��Ϊ�ֽڵ�ַ


    r1=SD_SendCmd(CMD17,sector,0X01);//������
		if(r1==0)//ָ��ͳɹ�
		{
		    while(SPI_SendByte(0xff)!=0xfe);
			 for(i=0;i<512;i++) 
			    {
			   //��������һ�������� 
			    buf[i]=SPI_SendByte(0xff);  
			    }
			 //������2��αCRC��dummy CRC��
			  SPI_SendByte(0xff); 
              SPI_SendByte(0xff); 		   
		}


	SD_DisSelect();//ȡ��Ƭѡ
	return r1;//

} 


////дSD��д��512�ֽ�����
////buf:���ݻ�����
////sector:��ʼ����
////����ֵ:0,ok;����,ʧ��.
//u8 SD_WriteDisk(u8*buf,u32 sector)
//{
//	u8 r1;
//	u16 t;
//    sector *= 512;//ת��Ϊ�ֽڵ�ַ
//
//	r1=SD_SendCmd(CMD24,sector,0X01);//������
//	if(r1==0)//ָ��ͳɹ�
//	{
//
//     if(SD_WaitReady()){SD_DisSelect();return 1;}//�ȴ�׼��ʧЧ
//	 SPI_SendByte(0xFE); //��������
//
//	 for(t=0;t<512;t++)SPI_SendByte(buf[t]);//д�ֽ�
//	 SPI_SendByte(0xFF);    //����crc
//	 SPI_SendByte(0xFF);
//	 t=SPI_SendByte(0xFF);  //������Ӧ
//	 if((t&0x1F)!=0x05)r1=2;//��Ӧ����									  					    
//	 	 	   
//	}
//
//
//	SD_DisSelect();//ȡ��Ƭѡ
//	return r1;
//}



//��ָ������дSD��һ�������� ��512�ֽ�
//buf:���ݻ�����
//sector:��ʼ����
//cnt:������
//����ֵ:0,ok;����,ʧ��.
//u8 SD_write_sector(u32 sector,u8*buf)
//{
//	u8 r1;
//	u16 t;
//	u32 m=0;
//	if(SD_Type!=SD_TYPE_V2HC)sector *= 512;//ת��Ϊ�ֽڵ�ַ
//
//	 r1=SD_SendCmd(CMD24,sector,0X01);//������
//		if(r1==0)//ָ��ͳɹ�
//		{
//			//r1=SD_SendBlock(buf,0xFE);//д512���ֽ�	 
//			 //�ȴ���׼������	
//	          do
//	          {
//		        if(SPI_SendByte(0XFF)==0XFF)break;//OK
//		          m++;		  	
//	           }while(m<0XFFFFFF);//�ȴ� 
//
//	          SPI_SendByte(0XFE);
//			  
//				for(t=0;t<512;t++)SPI_SendByte(buf[t]);//����ٶ�,���ٺ�������ʱ��
//			    SPI_SendByte(0xFF);//����crc
//			    SPI_SendByte(0xFF);
//				t=SPI_SendByte(0xFF);//������Ӧ
//				if((t&0x1F)!=0x05)return 2;//��Ӧ����									  					    
//		}
//	SD_DisSelect();//ȡ��Ƭѡ
//	return 0;  //д��ɹ�
//
//					
//}



//��ָ����ַ ��ȡ ָ���ֽ���
//addr:��ַ
//buf:���ݻ�����
//Bytes��Ҫ��ȡ���ֽ���
//����ֵ:0,ok;����,ʧ��.   
u8 SD_read_Byte(u32 addr,u8 *buf,u32 Bytes) 
{ 
    u16 i;
    u8 temp=0,retry;

    // ����16���ÿ鳤�� 
    SD_SendCmd(16,Bytes,0xff); 

    retry = 0;

    do
    {
    // д��CMD17 	��ȡ����
    temp=SD_SendCmd(17,addr,0xff);
    retry++;
    }
		
	while((temp!=0x00) && (retry < TRY_TIME)); 

	
    if (retry >= TRY_TIME) 
    {
    return READ_BLOCK_ERROR; 
    }

    // ����0xfe��ͷ  
    while(SPI_SendByte(0xff)!=0xfe);

     for(i=0;i<Bytes;i++) 
    {

        //��������ָ�� �����ֽ����ݵ�buf
        buf[i] = SPI_SendByte(0xff);  
    }

   //������2��αCRC��dummy CRC��
    SPI_SendByte(0xff); 
    SPI_SendByte(0xff);  


    //* ����16�ָ��鳤�� Ϊ512���ֽ�
    SD_SendCmd(16,512,0xff);
	SD_DisSelect();//ȡ��Ƭѡ
    
    return 0; 

} 


