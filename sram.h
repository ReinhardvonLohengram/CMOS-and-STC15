

#ifndef __SRAM_H
#define __SRAM_H
#include "def.h"

  
#ifndef NULL
#define NULL 0
#endif


sbit SRAM=P4^7;		           //�ⲿ��չ�ڴ�ƬѡIO
void sram(u8 m);			   //�ڴ�Ƭѡ���ⲿ�ڴ�Ƭѡ��ر�TFTƬѡ ��ֹ���ţ�


#define SRAM_BLOCK_SIZE			32  	  						//�ڴ���СΪ32�ֽ�
#define SRAM_BASE_SIZE			26*1024 						//�������ڴ�26*1024
#define SRAM_MAP_SIZE	        SRAM_BASE_SIZE/SRAM_BLOCK_SIZE 	//�ڴ���С
	 


u8 mem_perused();				 //�ڴ�ʹ����
void *mymalloc(u16 size)  ;		 //�����ڴ�
void myfree(void *ptr)  ;		 //�ͷ��ڴ�
void mem_init() ;				 //�ڴ��ʼ��

void sramtest();		  //�ڴ����� �ͷŲ���
#endif













