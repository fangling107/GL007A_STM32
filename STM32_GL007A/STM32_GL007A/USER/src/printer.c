#include "printer.h"
u8 GU8_PrintFinishIcon;//0 繁忙 1空闲
//初始化打印机端口.并使能
void Printer_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure; 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOG, ENABLE);	 //使能PA,PC,PG端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	//端口配置PA.12--CLK
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//根据设定参数初始化GPIOA.12
	GPIO_ResetBits(GPIOA,GPIO_Pin_12);						 //PA.12 输出低	

	/***PC6~11--data2~7*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);				//根据设定参数初始化GPIOC.4~11
	GPIO_ResetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11);//PC.6,7,8,9,10,11 输出低	

	/***PG6,7--data0,1***/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	
	GPIO_Init(GPIOG, &GPIO_InitStructure);	  				//推挽输出 ，IO口速度为50MHz
	GPIO_ResetBits(GPIOG,GPIO_Pin_6|GPIO_Pin_7);			//PG.6,7 输出低	
	
	/***PG5--STB***/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    	//STB
	GPIO_Init(GPIOG, &GPIO_InitStructure);	  			//推挽输出 ，IO口速度为50MHz
	GPIO_SetBits(GPIOG,GPIO_Pin_5); 					//PG.5 输出高
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;	  	//Busy 检测
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 	//设置成输入，默认下拉	  
	GPIO_Init(GPIOG, &GPIO_InitStructure);			//初始化GPIOG.4
}

void P_WriteData(u8 Data)
{
	P_Data0 = (Data>>0) & 0x01;
	P_Data1 = (Data>>1) & 0x01;
	P_Data2 = (Data>>2) & 0x01;
	P_Data3 = (Data>>3) & 0x01;
	P_Data4 = (Data>>4) & 0x01;
	P_Data5 = (Data>>5) & 0x01;
	P_Data6 = (Data>>6) & 0x01;
	P_Data7 = (Data>>7) & 0x01;	
}
//延时1ms子程序
void delayms(u16 x)
{
	u16 i,j;
	for(i=x;i>0;i--)
		for(j=12000;j>0;j--);	
}

/************************************************************
** 函数名称: print(uint8 Data)
** 功能描述: 访问打印机
** 输　入: 打印命令或数据
*************************************************************/
void pprint(u8 Data)
{  	

    P_STROBE = 1; //STB=0
		P_CLK = 0;
    while (P_busy != 0)
	{
		if(GU8_PrintFinishIcon == 1) 
		{
			GU8_PrintFinishIcon = 0;
			break;
		}
	}; 
	P_WriteData(Data);
	delayms(2);
	P_CLK = 1;
	delayms(2);
	P_CLK = 0;																//74ls273上升沿 锁存数据
	P_STROBE = 0;                              //STB=1;
	delayms(2);
	P_STROBE = 1;                              //STB=0;
	delayms(2); 
}

/**********************************************************
** 函数名称: PrintStr(char *Str)
** 功能描述: 打印所需内容
** 输　入: 要打印的字符或汉字
** 输　出: 无
** 全局变量: 无
** 调用模块: 无
***********************************************************/
void PrintStr(char *Str)
{
	while(*Str != 0)
	{
		pprint(*Str++);
		delayms(2);
	}
}

/**********************************************************
** 函数名称: PrintStr(char *Str)
** 功能描述: 打印所需内容
** 输　入: 要打印的字符或汉字
** 输　出: 无
** 全局变量: 无
** 调用模块: 无
***********************************************************/
void PrintStrNnum(char *Str,u8 num)
{
	u8 i=0;
	while((*Str != 0)&&(i<num))
	{
		pprint(*Str++);
		delayms(2);
		i++;
	}
}

 
