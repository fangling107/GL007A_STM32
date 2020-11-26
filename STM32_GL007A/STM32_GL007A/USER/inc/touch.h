#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "stm32f10x.h"
#include "setup.h"

#define WAIT  	PFin(8)  	//PF8 WAIT
#define INT 	PFin(9)   	//PF9  INT
//#define TP_XY_exchange 
//#define TP_X_swap
//#define TP_Y_swap
#define TP_PRES_DOWN 0x80  		//触屏被按下	  
#define TP_CATH_PRES 0x40  		//有按键按下了 

#define LCD_X_SIZE 480
#define LCD_Y_SIZE 272

extern	float xfac;					
extern	float yfac;
extern 	short xoff;
extern 	short yoff;
extern u8 touch_flag;	
extern u8 keystatus;
extern u8 sta;//按键状态//b7:按下1/松开0; 
	                     //b6:0,没有按键按下;1,有按键按下.
//////////////////////////////////////////////////////////////////////////	 
//保存在EEPROM里面的地址区间基址,占用12个字节(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
#define SAVE_ADDR_BASE 00

extern __IO int16_t OS_TimeMS;
extern u16 LCD_X,LCD_Y;		//实际坐标值
extern u16 ADC_value_X,ADC_value_Y;//物理坐标值
u8 TP_Scan(void);
void Print_Decimal(u16 buffer);     
void Printf_Hex(u8 buf);
void Get_XY_Calibration(void);
void XY_Calibration_Function(void);
u8 TP_Get_Adjdata(void);
void Access_TP_coordinate(u8 tp);//触摸屏扫描
void KeyScan(void) ;
#endif

















