#include "spi.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//SPI驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/9
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 
//以下是SPI模块的初始化代码，配置成主机模式，访问SD Card/W25Q64/NRF24L01						  
//SPI口初始化
//这里针是对SPI2的初始化



void SPI2_Init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB

 	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);  //PB13/14/15上拉

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
	
	SPI2_ReadWriteByte(0xff);//启动传输		 
 

}   
//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频 
  
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1&=0XFFC7;
	SPI2->CR1|=SPI_BaudRatePrescaler;	//设置SPI2速度 
	SPI_Cmd(SPI2,ENABLE); 

} 

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个数据
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(SPI2); //返回通过SPIx最近接收的数据					    
}

//SPI口初始化
//这里针是对SPI1的初始化
void SPI1_Init(void)
{	
	RCC->APB2ENR|=1<<0; 	//复用 
	RCC->APB2ENR|=1<<2;     //PORTA时钟使能 	 
	RCC->APB2ENR|=1<<12;    //SPI1时钟使能 
		   
	//这里只针对SPI口初始化
	GPIOA->CRL&=0X000FFFFF; 
	GPIOA->CRL|=0XBBB00000;//PA5.6.7复用 	    
	GPIOA->ODR|=0X7<<5;    //PA5.6.7上拉

		
	SPI1->CR1|=0<<10;//全双工模式	
	SPI1->CR1|=1<<9; //软件nss管理
	SPI1->CR1|=1<<8;	//内部从设备选择，它决定NSS上的电平

	SPI1->CR1|=1<<2; //SPI主机
	SPI1->CR1|=0<<11;//8bit数据格式		
	SPI1->CR1|=1<<1; //空闲模式下SCK为1 CPOL=1
	SPI1->CR1|=1<<0; //数据采样从第二个时间边沿开始,CPHA=1    
	
	SPI1->CR1|=7<<3; //Fsck=Fcpu/256
	SPI1->CR1|=0<<7; //MSBfirst 
	
	SPI1->CR2|=1<<6;      //接收缓冲区非空中断使能
//	MY_NVIC_Init(1,0,SPI1_IRQChannel,4);   
	  
	SPI1->CR1|=1<<6; //SPI设备使能
	SPI1_ReadWriteByte(0xff);//启动传输		 
}   
//SPI 速度设置函数
//SpeedSet:
//SPI_SPEED_2   2分频   (SPI 12M    --sys 24M)
//SPI_SPEED_8   8分频   (SPI 3M     --sys 24M)
//SPI_SPEED_16  16分频  (SPI 1.5M    --sys 24M)
//SPI_SPEED_256 256分频 (SPI  905.6K --sys 24M)
void SPI1_SetSpeed(u8 SpeedSet)
{
	SPI1->CR1&=0XFFC7;//Fsck=Fcpu/256
	switch(SpeedSet)
	{
		case SPI_SPEED_2:
			SPI1->CR1|=0<<3;//Fsck=Fpclk/2=36Mhz	
			break;
		case SPI_SPEED_4:
			SPI1->CR1|=1<<3;//Fsck=Fpclk/2=18Mhz	
			break;
		case SPI_SPEED_8:
			SPI1->CR1|=2<<3;//Fsck=Fpclk/2=9Mhz	
			break;
		case SPI_SPEED_16:
			SPI1->CR1|=3<<3;//Fsck=Fpclk/2=4.5Mhz	
			break;
		case SPI_SPEED_32:
			SPI1->CR1|=4<<3;//Fsck=Fpclk/2=2.25Mhz	
			break;
		case SPI_SPEED_64:
			SPI1->CR1|=5<<3;//Fsck=Fpclk/2=1.125Mhz	
			break;
		case SPI_SPEED_128:
			SPI1->CR1|=6<<3;//Fsck=Fpclk/2=562.5Khz	
			break;
		case SPI_SPEED_256:
			SPI1->CR1|=7<<3;//Fsck=Fpclk/2=281.25Khz	
			break;
		default:
			break;
	}
	SPI1->CR1|=1<<6; //SPI设备使能	  
} 

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 
	while((SPI1->SR&1<<1)==0)//等待发送区空	
	{
		retry++;
		if(retry>200) return 0;
	}			  
	SPI1->DR=TxData;	 	  //发送一个byte 
	retry=0;
	while((SPI1->SR&1<<0)==0) //等待接收完一个byte  
	{
		retry++;
		if(retry>200) return 0;		
	}	  						    
	return SPI1->DR;          //返回收到的数据				    
}
/*
u8 SPI1_ReadWriteByte(u8 TxData)
{				   			 
	while((SPI1->SR&1<<1)==0);//等待发送区空  
	SPI1->DR=TxData;	 	  //发送一个byte 	   
	while((SPI1->SR&1<<0)==0); //等待接收完一个byte   			    
	return SPI1->DR;          //返回收到的数据				    
}*/





























