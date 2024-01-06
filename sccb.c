//SCCB���� ��I2C���߷ǳ�����  
#include "stc15f2k60s2.h"
#include "sccb.h"
#include "delay.h"


/*
-----------------------------------------------
   ����: start����,SCCB����ʼ�ź�
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void startSCCB()
{
 SCCB_SID=1;
 SCCB_SIC=1;
 delay4us(35); 
 SCCB_SID=0;
 delay4us(35); 
 SCCB_SIC=0;
}
/*
-----------------------------------------------
   ����: stop����,SCCB��ֹͣ�ź�
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void stopSCCB()
{
 SCCB_SID=0;
 delay4us(35); 
 SCCB_SIC=1;
 delay4us(35); 
 SCCB_SID=1;
 delay4us(35);  
}

/*
-----------------------------------------------
   ����: noAck,����������ȡ�е����һ����������
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void noAck(void)
{

 delay4us(2); 
 SCCB_SID=1;

 SCCB_SIC=1;
 delay4us(35); 
 SCCB_SIC=0;
 delay4us(35); 
 SCCB_SID=0;
 delay4us(35);
}

u8 getAck() 
{
 u8  Error;

 SCCB_SID=1;//����SCCB_SIDΪ����
  
 delay4us(35); 
 SCCB_SIC=1;
 delay4us(35); 
 Error=SCCB_SID;
 delay4us(35); 
 SCCB_SIC=0; 
 delay4us(35); 

 SCCB_SID=0;
 return !Error;
}

/*
-----------------------------------------------
   ����: д��һ���ֽڵ����ݵ�SCCB
   ����: д������
 ����ֵ: ���ͳɹ�����1������ʧ�ܷ���0
-----------------------------------------------
*/
u8 SCCBwriteByte(u8 dat)
{
 u8 i;
 for(i=0;i<8;i++)
 {
  SCCB_SID=((dat<<i)&0x80);
  delay4us(35); 
  SCCB_SIC=1;
  delay4us(35); 
  SCCB_SIC=0;
 }
 SCCB_SID=0;
 return getAck();
}

/*
-----------------------------------------------
   ����: һ���ֽ����ݶ�ȡ���ҷ���
   ����: ��
 ����ֵ: ��ȡ��������
-----------------------------------------------
*/
u8 SCCBreadByte(void)
{
 u8 i,rbyte=0;


 SCCB_SID=1;//����SCCB_SIDΪ����

 for(i=0;i<8;i++)
 {
  delay4us(35); 
  SCCB_SIC=1;  
  if(SCCB_SID) rbyte|=(0x80>>i);
  delay4us(35); 
  SCCB_SIC=0;
  delay4us(35); 
 } 


 SCCB_SID=0;
 return rbyte;
}
