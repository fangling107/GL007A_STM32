#include "ADS8341.h" 
#include "spi.h"
#include "ra8870.h"
//////////////////////////////////////////////////////////////////////////////////	 
//ADS8341 采样						  
////////////////////////////////////////////////////////////////////////////////// 


//延时us
void delay_1us(u16 m)
{
	u8 i=10;
	for(;m>0;m--)
		while(i--);
}
//初始化SPI FLASH的IO口
void SPI_AD8341_Init(void)
{	
  	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE);	//PORTA时钟使能 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  // PA4 推挽 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOA,GPIO_Pin_4);

	SPI1_Init();		   	//初始化SPI

}

u16 SPI_AD8341_Read_ADVal(u8 ch)   
{  
	u8   S0PDR_H = 0,S0PDR_L = 0;
	u16  S0PDR_HL=0; 
	switch(ch)
	{
		case 0:
			ch = 0x94;
			break;
		case 1:
			ch = 0xd4;
			break;
		default:break;
	}
	SPI_AD8341_CS = 1;  
	delay_1us(30);
	SPI_AD8341_CS = 0;			//使能器件 
	delay_1us(30);	
	SPI1_ReadWriteByte(ch);    //发送读取状态寄存器命令
   //------判断SPIF是否置位段(发送第1个8位数据)----------
	delay_1us(30);  //??
	S0PDR_H = SPI1_ReadWriteByte(0x00);    //发送读取状态寄存器命令   
	S0PDR_H=S0PDR_H<<1;    		//因只有后7位有效
	//------判断SPIF是否置位段(发送第2个8位数据)----------
	delay_1us(30); 
	S0PDR_L = SPI1_ReadWriteByte(0x00);	//发送任意数据以启动spi时钟	 

	if ((S0PDR_L&0X80)!=0)
	{
		S0PDR_H=S0PDR_H|0X01;  //补足高字节最后一位
	}
	S0PDR_L=S0PDR_L<<1; //因只有后7位就低字节有效位
	//------判断SPIF是否置位段(发送第3个8位数据)---------
	delay_1us(30);
	SPI1_ReadWriteByte(0x00); 

	if ((SPI1->DR & 0X80)!=0)  //读取最低位，如果=1，并补足
	{
		S0PDR_L=S0PDR_L|0X01;  //补足低字节最后一位
	}
	//--取消片选，高低字节组合并返回值段---
	SPI_AD8341_CS = 1;                // 取消片选
	SPI_AD8341_CS = 1;                // 取消片选
	S0PDR_HL = (S0PDR_H<<8) + S0PDR_L;
	return(S0PDR_HL);                   //返回ad值			
}

/****************************************************************************
* 名称:AdSample()
* 功能:10次采样，去掉最大/最小值，剩余8个取平均值
* 入口参数:ch--->选择通道号
* 出口参数:a--->8个数平均值，原始ad值
****************************************************************************/
//int  AdSample (u8 ch)
u16 read_16bit_pressAD(u8 ch, u8 times)
{  
	u8  i, j; 
	u16  a, rcv_data[50];
	u32   rcv_data_float;

  //10次采样
	for(i=0; i<times; i++) 
	{       
		rcv_data[i] = 0;
    //Stop_EA();
		a = SPI_AD8341_Read_ADVal(ch);
    //Start_EA();
		rcv_data[i] = a; 
//  Dely(100);
	}
  
  //10个数排序
	for(j=(times-1); j>0; j--)
	{
		for (i=0; i<j; i++) 
		{
			if (rcv_data[i] > rcv_data[i+1])
			{
				a = rcv_data[i+1];
				rcv_data[i+1] = rcv_data[i];
				rcv_data[i] = a;
			}
		}
	}		
  
   //去掉最大/最小值，剩余8个数平均值     
	rcv_data_float = 0;
	for(i=1; i<(times-1); i++)
	{
		rcv_data_float += rcv_data[i];
	}
  
	a = rcv_data_float / (times-2);  //取平均值        
	return a;                          
}


