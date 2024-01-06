

#ifndef __SRAM_H
#define __SRAM_H
#include "def.h"

  
#ifndef NULL
#define NULL 0
#endif


sbit SRAM=P4^7;		           //外部扩展内存片选IO
void sram(u8 m);			   //内存片选（外部内存片选则关闭TFT片选 防止干扰）


#define SRAM_BLOCK_SIZE			32  	  						//内存块大小为32字节
#define SRAM_BASE_SIZE			26*1024 						//最大管理内存26*1024
#define SRAM_MAP_SIZE	        SRAM_BASE_SIZE/SRAM_BLOCK_SIZE 	//内存表大小
	 


u8 mem_perused();				 //内存使用率
void *mymalloc(u16 size)  ;		 //申请内存
void myfree(void *ptr)  ;		 //释放内存
void mem_init() ;				 //内存初始化

void sramtest();		  //内存申请 释放测试
#endif













