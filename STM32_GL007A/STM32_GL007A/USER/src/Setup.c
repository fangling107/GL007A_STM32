/*
********************************************************************************
*  Œƒ º˛ √˚£∫Setup.C
*
*  Œƒº˛√Ë ˆ£∫”≤º˛≥ı ºªØœ‡πÿ∫Ø ˝
*
*  À˘”√–æ∆¨£∫
*
*  ¥¥ Ω® »À£∫
*
*  ∞Ê ±æ ∫≈£∫
*
*  –ﬁ∏ƒº«¬º£∫1.–ﬁ∏ƒ—” ±∫Ø ˝“‘  ”¶≤ªÕ¨ ±÷”°£
********************************************************************************
*/
#include "stm32f10x.h"
#include "flash.h"
#include "SPI.h"
//—” ±us
void delay_mus(u8 time)
{	
	u8 i;
 	for(;time>0;time--)
		for(i=10;i>0;i--);
}
//LCD FSMC≈‰÷√ 
void LCD_FSMCConfig(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  FSMC_NORSRAMTimingInitStructure;	
/*-- FSMC Configuration ------------------------------------------------------*/
/*----------------------- SRAM Bank 4 ----------------------------------------*/
  /* FSMC_Bank1_NORSRAM4 configuration */
  FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 0x02;
  FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0x00;
  FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 0x05;
  FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0x01;
  FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0x00;
  FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0x00;
  FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;
    
  /* Color LCD configuration ------------------------------------
     LCD configured as follow:
        - Data/Address MUX = Disable
        - Memory Type = SRAM
        - Data Width = 16bit
        - Write Operation = Enable
        - Extended Mode = Enable
        - Asynchronous Wait = Disable */
  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;
  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  
  /* BANK 4 (of NOR/SRAM Bank 1~4) is enabled */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
		
}
//RA8870 FSMC¡¨Ω”
void LCD_CtrlLinesConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Set PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
     PD.10(D15), PD.14(D0), PD.15(D1) as alternate function push pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
                                GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | 
                                GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  /* Set PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
     PE.14(D11), PE.15(D12) as alternate function push pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
                                GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
                                GPIO_Pin_15;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	 /*!< SRAM Address lines configuration  PG0(A10) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOG, &GPIO_InitStructure); 	
  
  /* Set PG.12(NE4 (LCD/CS)) as alternate function push pull - NE4(LCD /CS) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	//LCD±≥π‚øÿ÷∆ PF10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOF, &GPIO_InitStructure);  
	GPIO_SetBits(GPIOF, GPIO_Pin_10);	
	//LCDwait–≈∫≈ INT–≈∫≈
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOF, &GPIO_InitStructure); 
	  
}

/* 
********************************************************************************
* ∫Ø  ˝ √˚£∫RCC_Configuration
* π¶    ƒ‹£∫Õ≥Õ‚Œß∆˜º˛ ±÷”≈‰÷√
* ≤Œ     ˝£∫Œﬁ
* ∑µ    ªÿ£∫Œﬁ
* Àµ    √˜£∫Œﬁ
* ≈◊≥ˆ“Ï≥££∫
* ◊˜    ’ﬂ£∫023
* –ﬁ∏ƒº«¬º£∫
********************************************************************************
*/
void RCC_Configuration(void)
{
// 	//ø™∆ÙCRC ±÷” ∆Ù∂ØemWIN
//   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE); 
	/* Enable GPIOA,GPIOB,GPIOC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |RCC_APB2Periph_AFIO, ENABLE);
	/* Enable FSMC */	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	/* Enable spi */	
	RCC_APB1PeriphClockCmd(	RCC_APB2Periph_SPI1|RCC_APB1Periph_SPI2,  ENABLE );//SPI1.SPI12 ±÷” πƒ‹ 	
	
}



/* 
********************************************************************************
* ∫Ø  ˝ √˚£∫NVIC_Configuration
* π¶    ƒ‹£∫œµÕ≥÷–∂œœÚ¡ø”≈œ»º∂≈‰÷√
* ≤Œ     ˝£∫Œﬁ
* ∑µ    ªÿ£∫Œﬁ
* Àµ    √˜£∫
* ≈◊≥ˆ“Ï≥££∫
* ◊˜    ’ﬂ£∫023
* –ﬁ∏ƒº«¬º£∫
********************************************************************************
*/
void NVIC_Configuration(void)
{	

}
//…Ë÷√NVIC∑÷◊È
//NVIC_Group:NVIC∑÷◊È 0~4 ◊‹π≤5◊È 		   
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//»°∫Û»˝Œª
	temp1<<=8;
	temp=SCB->AIRCR;  //∂¡»°œ»«∞µƒ…Ë÷√
	temp&=0X0000F8FF; //«Âø’œ»«∞∑÷◊È
	temp|=0X05FA0000; //–¥»Î‘ø≥◊
	temp|=temp1;	   
	SCB->AIRCR=temp;  //…Ë÷√∑÷◊È	    	  				   
}
//…Ë÷√NVIC 
//NVIC_PreemptionPriority:«¿’º”≈œ»º∂
//NVIC_SubPriority       :œÏ”¶”≈œ»º∂
//NVIC_Channel           :÷–∂œ±‡∫≈
//NVIC_Group             :÷–∂œ∑÷◊È 0~4
//◊¢“‚”≈œ»º∂≤ªƒ‹≥¨π˝…Ë∂®µƒ◊Èµƒ∑∂Œß!∑Ò‘Úª·”–“‚œÎ≤ªµΩµƒ¥ÌŒÛ
//◊ÈªÆ∑÷:
//◊È0:0Œª«¿’º”≈œ»º∂,4ŒªœÏ”¶”≈œ»º∂
//◊È1:1Œª«¿’º”≈œ»º∂,3ŒªœÏ”¶”≈œ»º∂
//◊È2:2Œª«¿’º”≈œ»º∂,2ŒªœÏ”¶”≈œ»º∂
//◊È3:3Œª«¿’º”≈œ»º∂,1ŒªœÏ”¶”≈œ»º∂
//◊È4:4Œª«¿’º”≈œ»º∂,0ŒªœÏ”¶”≈œ»º∂
//NVIC_SubPriority∫ÕNVIC_PreemptionPriorityµƒ‘≠‘Ú «, ˝÷µ‘Ω–°,‘Ω”≈œ»	   
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	MY_NVIC_PriorityGroupConfig(NVIC_Group);//…Ë÷√∑÷◊È
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;//»°µÕÀƒŒª  
	if(NVIC_Channel<32)NVIC->ISER[0]|=1<<NVIC_Channel;// πƒ‹÷–∂œŒª(“™«Â≥˝µƒª∞,œ‡∑¥≤Ÿ◊˜æÕOK)
	else NVIC->ISER[1]|=1<<(NVIC_Channel-32);    
	NVIC->IP[NVIC_Channel]|=temp<<4;//…Ë÷√œÏ”¶”≈œ»º∂∫Õ«¿∂œ”≈œ»º∂   	    	  				   
}

/* 
********************************************************************************
* ∫Ø  ˝ √˚£∫GPIO_Configuration
* π¶    ƒ‹£∫≥ı ºªØIOø⁄
* ≤Œ     ˝£∫Œﬁ
* ∑µ    ªÿ£∫Œﬁ
* Àµ    √˜£∫øÿ÷∆PB6 ∆¯±√µƒ≥ı ºªØ ∑‰√˘∆˜µƒ≥ı ºªØ PB8∑‰√˘∆˜
* ≈◊≥ˆ“Ï≥££∫
* ◊˜    ’ﬂ£∫023
* –ﬁ∏ƒº«¬º£∫
********************************************************************************
*/
void GPIO_Configuration(void)
{

	//PB6∆¯±√µƒ≥ı ºªØ PB8∑‰√˘∆˜	
  GPIO_InitTypeDef  GPIO_InitStructure; 	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_8;	//∂Àø⁄≈‰÷√PB6 PB8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//Õ∆ÕÏ ‰≥ˆ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IOø⁄ÀŸ∂»Œ™50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//∏˘æ›…Ë∂®≤Œ ˝≥ı ºªØGPIOB6
	GPIO_ResetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_8);						 //PB6 PB8 ‰≥ˆµÕ

}


/* 
********************************************************************************
* ∫Ø  ˝ √˚£∫RTC_Configuration
* π¶    ƒ‹£∫ µ ± ±÷”250ms÷–∂œ“ª¥Œ
* ≤Œ     ˝£∫Œﬁ
* ∑µ    ªÿ£∫Œﬁ
* Àµ    √˜£∫
* ≈◊≥ˆ“Ï≥££∫
* ◊˜    ’ﬂ£∫023
* –ﬁ∏ƒº«¬º£∫
********************************************************************************
*/
void RTC_Configuration(void)
{

}



/* 
********************************************************************************
* ∫Ø  ˝ √˚£∫
* π¶    ƒ‹£∫
* ≤Œ     ˝£∫
* ∑µ    ªÿ£∫ 
* Àµ    √˜£∫
* ≈◊≥ˆ“Ï≥££∫
* ◊˜    ’ﬂ£∫023
* –ﬁ∏ƒº«¬º£∫
********************************************************************************
*/
/*
void ADC_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    //PB0--TBT2--ºÏ≤‚µÁ≥ÿ2(4.8V)
    //PB1--TBT1--ºÏ≤‚µÁ≥ÿ1(3.6V)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    DMA_DeInit(DMA1_Channel1);
    //DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    //DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCConvertedValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 2;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    //Enable DMA1 channel1 
    DMA_Cmd(DMA1_Channel1, ENABLE);

    //ADC1 configuration 
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 2;
    ADC_Init(ADC1, &ADC_InitStructure);

    //ADC1 regular channel,8,9 configuration 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_71Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_71Cycles5);

    //Enable ADC1 DMA 
    ADC_DMACmd(ADC1, ENABLE);

    //Enable ADC1 
    ADC_Cmd(ADC1, ENABLE);

    //Enable ADC1 reset calibaration register 
    ADC_ResetCalibration(ADC1);
    //Check the end of ADC1 reset calibration register 
    while(ADC_GetResetCalibrationStatus(ADC1));

    //Start ADC1 calibaration 
    ADC_StartCalibration(ADC1);
    //Check the end of ADC1 calibration 
    while(ADC_GetCalibrationStatus(ADC1));

    //Start ADC1 Software Conversion 
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


*/
//≥ı ºªØADC
//’‚¿ÔŒ“√«Ωˆ“‘πÊ‘ÚÕ®µ¿Œ™¿˝
//Œ“√«ƒ¨»œΩˆø™∆ÙÕ®µ¿1																	   
void  Adc_Init(void)
{    
	//œ»≥ı ºªØIOø⁄
 	RCC->APB2ENR|=1<<2;    // πƒ‹PORTAø⁄ ±÷” 
	GPIOA->CRL&=0XFFFFFF0F;//PA1 anolog ‰»Î
	RCC->APB2ENR|=1<<4;    // πƒ‹PORTCø⁄ ±÷” 
	GPIOC->CRL&=0XFFFFFFF0;//PC0 anolog ‰»Î
	//Õ®µ¿10/11…Ë÷√			 
	RCC->APB2ENR|=1<<9;    //ADC1 ±÷” πƒ‹	  
	RCC->APB2RSTR|=1<<9;   //ADC1∏¥Œª
	RCC->APB2RSTR&=~(1<<9);//∏¥ŒªΩ· ¯	    
	RCC->CFGR&=~(3<<14);   //∑÷∆µ“Ú◊”«Â¡„	
	//SYSCLK/DIV2=12M ADC ±÷”…Ë÷√Œ™12M,ADC◊Ó¥Û ±÷”≤ªƒ‹≥¨π˝14M!
	//∑Ò‘ÚΩ´µº÷¬ADC◊º»∑∂»œ¬Ωµ! 
	RCC->CFGR|=2<<14;      	 
	ADC1->CR1&=0XF0FFFF;   //π§◊˜ƒ£ Ω«Â¡„
	ADC1->CR1|=0<<16;      //∂¿¡¢π§◊˜ƒ£ Ω  
	ADC1->CR1&=~(1<<8);    //∑«…®√Ëƒ£ Ω	  
	ADC1->CR2&=~(1<<1);    //µ•¥Œ◊™ªªƒ£ Ω
	ADC1->CR2&=~(7<<17);	   
	ADC1->CR2|=7<<17;	   //»Ìº˛øÿ÷∆◊™ªª  
	ADC1->CR2|=1<<20;      // π”√”√Õ‚≤ø¥•∑¢(SWSTART)!!!	±ÿ–Î π”√“ª∏ˆ ¬º˛¿¥¥•∑¢
	ADC1->CR2&=~(1<<11);   //”“∂‘∆Î	 
	ADC1->SQR1&=~(0XF<<20);
	ADC1->SQR1|=0<<20;     //1∏ˆ◊™ªª‘⁄πÊ‘Ú–Ú¡–÷– “≤æÕ «÷ª◊™ªªπÊ‘Ú–Ú¡–1 			   
	//…Ë÷√Õ®µ¿1µƒ≤…—˘ ±º‰
	ADC1->SMPR1&=~(7<<0);  //Õ®µ¿10≤…—˘ ±º‰«Âø’	  
 	ADC1->SMPR1|=7<<0;     //Õ®µ¿10  239.5÷‹∆⁄,Ã·∏ﬂ≤…—˘ ±º‰ø…“‘Ã·∏ﬂæ´»∑∂»
	ADC1->SMPR2&=~(7<<3);  //Õ®µ¿1≤…—˘ ±º‰«Âø’	  
 	ADC1->SMPR2|=7<<3;     //Õ®µ¿1  239.5÷‹∆⁄,Ã·∏ﬂ≤…—˘ ±º‰ø…“‘Ã·∏ﬂæ´»∑∂»	 
	ADC1->CR2|=1<<0;	   //ø™∆ÙAD◊™ªª∆˜	 
	ADC1->CR2|=1<<3;       // πƒ‹∏¥Œª–£◊º  
	while(ADC1->CR2&1<<3); //µ»¥˝–£◊ºΩ· ¯ 			 
    //∏√Œª”…»Ìº˛…Ë÷√≤¢”…”≤º˛«Â≥˝°£‘⁄–£◊ººƒ¥Ê∆˜±ª≥ı ºªØ∫Û∏√ŒªΩ´±ª«Â≥˝°£ 		 
	ADC1->CR2|=1<<2;        //ø™∆ÙAD–£◊º	   
	while(ADC1->CR2&1<<2);  //µ»¥˝–£◊ºΩ· ¯
	//∏√Œª”…»Ìº˛…Ë÷√“‘ø™ º–£◊º£¨≤¢‘⁄–£◊ºΩ· ¯ ±”…”≤º˛«Â≥˝  
}	
//ªÒµ√ADC÷µ
//ch:Õ®µ¿÷µ 0~16
//∑µªÿ÷µ:◊™ªªΩ·π˚
u16 Get_Adc(u8 ch)   
{
	//…Ë÷√◊™ªª–Ú¡–	  		 
	ADC1->SQR3&=0XFFFFFFE0;//πÊ‘Ú–Ú¡–1 Õ®µ¿ch
	ADC1->SQR3|=ch;		  			    
	ADC1->CR2|=1<<22;       //∆Ù∂ØπÊ‘Ú◊™ªªÕ®µ¿ 
	while(!(ADC1->SR&1<<1));//µ»¥˝◊™ªªΩ· ¯	 	   
	return ADC1->DR;		//∑µªÿadc÷µ	
}
u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{		
		temp_val+=Get_Adc(ch);
		delay_mus(50);		
}
	return temp_val/times;
} 
void TIM4_Int_Init(void)	//∂® ±1ms
{
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

	TIM_TimeBaseInitStructure.TIM_Prescaler=71;   
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;	  
	TIM_TimeBaseInitStructure.TIM_Period=999;		 
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM4,ENABLE);	

}
//PB7øÿ÷∆∆¯±√µƒºÃµÁ∆˜–≈∫≈
void starttest(void)
{	
 	GPIO_SetBits(GPIOB,GPIO_Pin_6);						 //PB6  ‰≥ˆ∏ﬂ
}
//PB7øÿ÷∆∆¯±√µƒºÃµÁ∆˜–≈∫≈
void stoptest(void)
{	
	GPIO_ResetBits(GPIOB,GPIO_Pin_6);						 //PB6  ‰≥ˆµÕ
}


/* 
********************************************************************************
* ∫Ø  ˝ √˚£∫InitHardware
* π¶    ƒ‹£∫≥ı ºªØœµÕ≥”≤º˛
* ≤Œ     ˝£∫Œﬁ
* ∑µ    ªÿ£∫Œﬁ
* Àµ    √˜£∫
* ≈◊≥ˆ“Ï≥££∫
* ◊˜    ’ﬂ£∫023
* –ﬁ∏ƒº«¬º£∫
********************************************************************************
*/
void InitHardware(void)
{  		
		//SystemInit();			//œµÕ≥ ±÷”≥ı ºªØ
		//NVIC_Configuration();
		RCC_Configuration();		
		GPIO_Configuration();
		LCD_FSMCConfig();	//LCD fsmc≈‰÷√
		LCD_CtrlLinesConfig();//LCD GPIO≈‰÷	
		TIM4_Int_Init();
		SPI_Flash_Init();
}



