#ifndef __ADS8341_H
#define __ADS8341_H			    
#include "stm32f10x.h" 
#include "setup.h"  

#define	SPI_AD8341_CS PAout(4)  //选中AD8341
extern u16 pressADvalue;//ADS8341的AD值

void SPI_AD8341_Init(void);
u16 SPI_AD8341_Read_ADVal(u8 ch);	//读取通道ch的ad值
u16 read_16bit_pressAD(u8 ch, u8 times);	//采样10次，求平均值

#endif
















