#ifndef __PRINTER_H
#define __PRINTER_H	 
#include "stm32f10x.h" 
#include "setup.h"
extern u8 GU8_PrintFinishIcon;//0 繁忙 1空闲

//端口定义
#define P_busy PGin(4)      // 检测打印机是否准备好
#define P_STROBE PGout(5)   //STB
#define P_CLK PAout(12)     //74LS273上升沿有效

#define P_Data0 PCout(10)
#define P_Data1 PCout(11)
#define P_Data2 PCout(9)
#define P_Data3 PCout(8)
#define P_Data4 PCout(7)
#define P_Data5 PGout(6)
#define P_Data6 PGout(7)
#define P_Data7 PCout(6)

void Printer_Init(void);//初始化打印机端口		 		
void PrintStr(char *Str);
void pprint(u8 Data);	 
void PrintStrNnum(char *Str,u8 num);
#endif

















