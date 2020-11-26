#include "rtc.h" 
#include "ra8870.h"
#include "touch.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "picture.h"
#include "stm32f10x.h"
#include "stm32f10x_fsmc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//4.3寸 TFT液晶驱动	  	
//********************************************************************************	 

char Data[10];//日期
char Time[8];//时间

//延时1us子程序
void Delay1us(void)
{
	u8 i;
	for(i=10;i>0;i--);	
}	
//延时1ms子程序
void Delay1ms(u16 x)
{
	u16 i,j;
	for(i=x;i>0;i--)
		for(j=12000;j>0;j--);	
}		   
//写寄存器函数
//regval:寄存器值
void LCD_WR_REG(u8 regval)
{ 
	LCD_REG=regval;//写入要写的寄存器序号	 
}
//写LCD数据
//data:要写入的值
void LCD_WR_DATA(u8 data)
{										    	   
	LCD_RAM=data;		 
}
//读LCD数据
//返回值:读到的值
u8 LCD_RD_DATA(void)
{	
	u8 ram=0;	
	ram=LCD_RAM;
	return ram;		 
}	
u8 LCD_ReadStatus(void)
{
	u8 stat=0;
	stat=LCD_REG;
	return stat;

}  
//读寄存器
//LCD_Reg:寄存器地址
//返回值:读到的数据
u8 LCD_ReadReg(u8 LCD_Reg)
{										   
	LCD_WR_REG(LCD_Reg);		
	Delay1us();		  
	return LCD_RD_DATA();	
}   


//系统时钟设置
void RA8870_PLL_ini(void)
{
		LCD_WR_REG(0x88);
		//Delay1us();
		LCD_WR_DATA(0x0B);      
    Delay1ms(100); //After REG[88h] or REG[89h] is programmed, a lock time (< 30us) must be kept to guarantee the stability of the PLL output. After the lock time period, a software reset must be asserted and user must re-program the RA8870 to complete the procedure
    LCD_WR_REG(0x89);
		//Delay1us();
		LCD_WR_DATA(0x03); 
    Delay1ms(100); //After REG[88h] or REG[89h] is programmed, a lock time (< 30us) must be kept to guarantee the stability of the PLL output. After the lock time period, a software reset must be asserted and user must re-program the RA8870 to complete the procedure
    LCD_WR_REG(0x01);
		//Delay1us();
		LCD_WR_DATA(0x01);	
    LCD_WR_DATA(0x00);
    Delay1ms(100);
}
//初始化lcd
void LCDRA8870_Init(void)
{ 	
	RA8870_PLL_ini();

	LCD_WR_REG(0x04); //PCLK    
	LCD_WR_DATA(0x80);
	Delay1ms(100);
	LCD_WR_REG(0x10);	
	LCD_WR_DATA(0x3c);//64K 外部SRAM
	Delay1ms(100);
	LCD_WR_REG(0x11);
	LCD_WR_DATA(0x00);//RGB
	Delay1ms(100);	 
	
	LCD_WR_REG(0x12);	 //IOCR  GPIO& analog TFT interface
	LCD_WR_DATA(0x00); //

	LCD_WR_REG(0x13);	 //IODR  GPIO
	LCD_WR_DATA(0x05);
	
//Horizontal set
	LCD_WR_REG(0x14);	
	LCD_WR_DATA(0x3B);
	
	LCD_WR_REG(0x15);	
	LCD_WR_DATA(0x00); 
	
	LCD_WR_REG(0x16);	
	LCD_WR_DATA(0x00);
	
	LCD_WR_REG(0x17);
	LCD_WR_DATA(0x00);
	
	LCD_WR_REG(0x18);	
	LCD_WR_DATA(0x03);

	LCD_WR_REG(0x19);
	LCD_WR_DATA(0x0f);	
	LCD_WR_REG(0x1a);
	LCD_WR_DATA(0x01);	
	LCD_WR_REG(0x1b);
	LCD_WR_DATA(0x00);	
	LCD_WR_REG(0x1c);
	LCD_WR_DATA(0x00);	
	LCD_WR_REG(0x1d);
	LCD_WR_DATA(0x00);	
	LCD_WR_REG(0x1e);
	LCD_WR_DATA(0x00);	
	LCD_WR_REG(0x1f);	
	LCD_WR_DATA(0x01);	
	LCD_WR_REG(0x28);  //if use font ROM Speed setting
	LCD_WR_DATA(0x04);	
	LCD_WR_REG(0x40);  //绘图模式
	LCD_WR_DATA(0x00);
	LCD_WR_REG(0x41);  //绘图模式
	LCD_WR_DATA(0x00);
	Display_ON();
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(0xFF);//	
	Memory_Clear();	
	Delay1ms(100);
}
//STATUS 
void Chk_Busy(void)
{
	u8 temp; 	
	do
	{
	temp=LCD_StatusRead();
	}while((temp&0x80)==0x80);		   
}
void Chk_Busy_BTE(void)
{
	u8 temp;
   	
	do
	{
     temp=LCD_StatusRead();
	}while((temp&0x40)==0x40);	   
}	
void Display_ON(void)
{
	u8 temp;

	LCD_WR_REG(0x01);//PWRR
	temp = LCD_RD_DATA();
	temp |= 0x80;
	LCD_WR_REG(0x01);//PWRR
	LCD_WR_DATA(temp);
}


void Display_OFF(void)
{
	u8 temp;

	LCD_WR_REG(0x01);//PWRR
	temp = LCD_RD_DATA();
	temp &= 0x7f;
	LCD_WR_REG(0x01);//PWRR
	LCD_WR_DATA(temp);
}

//工作窗口的设定
void Active_Window(u16 XL,u16 XR ,u16 YT ,u16 YB)
{
	
    //setting active window X
	
  LCD_REG=(0x30);//HSAW0
	LCD_RAM=(XL);
	  
  LCD_REG=(0x31);//HSAW1	   
	LCD_RAM=(XL>>8);

	
  LCD_REG=(0x34);//HEAW0
	LCD_RAM=(XR);
	  
  LCD_REG=(0x35);//HEAW1	   
	LCD_RAM=(XR>>8);

    //setting active window Y
	 
  LCD_REG=(0x32);//VSAW0
	LCD_RAM=(YT);
	
  LCD_REG=(0x33);//VSAW1	   
	LCD_RAM=(YT>>8);


  LCD_REG=(0x36);//VEAW0
	LCD_RAM=(YB);
	
  LCD_REG=(0x37);//VEAW1	   
	LCD_RAM=(YB>>8);
}
//绘制矩形
void Geometric_Coordinate(u16 XL,u16 XR ,u16 YT ,u16 YB)
{ 
  LCD_REG=(0x91);
	LCD_RAM=(XL);
 
  LCD_REG=(0x92);	   
	LCD_RAM=(XL>>8);

 
  LCD_REG=(0x95);
	LCD_RAM=(XR);
   
  LCD_REG=(0x96);	   
	LCD_RAM=(XR>>8);
	   
  
  LCD_REG=(0x93);
	LCD_RAM=(YT);
	  
  LCD_REG=(0x94);	   
	LCD_RAM=(YT>>8);

	 
  LCD_REG=(0x97);
	LCD_RAM=(YB);
	 
  LCD_REG=(0x98);	   
	LCD_RAM=(YB>>8);
}
//设定全屏刷屏
void Clear_Full_Window(void)
{
	u8 temp;
	LCD_WR_REG(0x8e);//MCLR
	temp = LCD_RD_DATA();
	temp &= 0xbf;
	LCD_WR_REG(0x8e);//MCLR
	LCD_WR_DATA(temp);  
}
//选择文字背景刷屏
void Memory_Clear_with_Font_BgColor(void)
{
	u8 temp;
	LCD_WR_REG(0x8e);//MCLR
	temp = LCD_RD_DATA();
	temp |= 0x01;
	LCD_WR_REG(0x8e);//MCLR
	LCD_WR_DATA(temp);  
}
//设置文字背景色
void Text_Background_Color(u8 color)
{	
  LCD_REG=(0x43);//TBCR
	LCD_RAM=(color);
}
//设置文字显示位置
void XY_Coordinate(u16 X,u16 Y)
{
	u8 temp;

	temp=X;   
    LCD_REG=(0x46);
	LCD_RAM=(temp);
	temp=X>>8;   
    LCD_REG=(0x47);	   
	LCD_RAM=(temp);

	temp=Y;   
    LCD_REG=(0x48);
	LCD_RAM=(temp);
	temp=Y>>8;   
    LCD_REG=(0x49);	   
	LCD_RAM=(temp);
}
//REG[40h]
void Graphic_Mode(void)
{	u8 temp;
	LCD_REG=(0x40);//MWCR0
	temp = LCD_RAM;
	temp &= 0X7F ;
	LCD_RAM=(temp);
}
//
void Text_Foreground_Color(u8 color)
{	 
  LCD_REG=(0x42);//TFCR
	LCD_RAM=(color);
}

void BTE_Source_Destination	(u16 SX,u16 DX ,u16 SY ,u16 DY)
{
	u8 temp,temp1;
    
	temp=SX;   
    LCD_REG=(0x54);//HSBE0
	LCD_RAM=(temp);
	temp=SX>>8;   
    LCD_REG=(0x55);//HSBE1	   
	LCD_RAM=(temp);

	temp=DX;   
    LCD_REG=(0x58);//HDBE0
	LCD_RAM=(temp);
	temp=DX>>8;   
    LCD_REG=(0x59);//HDBE1	   
	LCD_RAM=(temp); 
    
	temp=SY;   
    LCD_REG=(0x56);//VSBE0
	LCD_RAM=(temp);
	temp=SY>>8;   
    LCD_REG=(0x57);//VSBE1
	temp1 = LCD_RAM;
    temp=temp|temp1; 
	LCD_REG=(0x57);//VSBE1  
	LCD_RAM=(temp);


	temp=DY;   
    LCD_REG=(0x5a);//VDBE0
	LCD_RAM=(temp);
	temp=DY>>8;   
    LCD_REG=(0x5b);//VDBE1
	temp1 = LCD_RAM;
	temp=temp|temp1;	
	LCD_REG=(0x5b);//VDBE1   
	LCD_RAM=(temp);
}				

void BTE_Size_setting(u8 width,u8 height)
{	u8 temp;
	temp=width;   
    LCD_REG=(0x5c);//BEWR0
	LCD_RAM=(temp);
	temp=width>>8;   
    LCD_REG=(0x5d);//BEWR1	   
	LCD_RAM=(temp);

	temp=height;   
    LCD_REG=(0x5e);//BEHR0
	LCD_RAM=(temp);
	temp=height>>8;   
    LCD_REG=(0x5f);//BEHR1	   
	LCD_RAM=(temp);
}		
								 
//---------------------------------------------//
//---------------------------------------------//
void BTE_Background_red(u16 color)
{	u8 temp;   
	temp=color;   
    LCD_REG=(0x60);//BGCR0
	LCD_RAM=(temp);
}							
//---------------------------------------------//
//---------------------------------------------//
void BTE_Background_green(u16 color)
{	u8 temp;   
	temp=color;   
    LCD_REG=(0x61);//BGCR1
	LCD_RAM=(temp);
}							  
//---------------------------------------------//
//---------------------------------------------//
void BTE_Background_blue(u16 color)
{	u8 temp;  
	temp=color;   
    LCD_REG=(0x62);//BGCR2
	LCD_RAM=(temp);
} 							

//---------------------------------------------//
//---------------------------------------------//
void BTE_Foreground_red(u16 color)
{	u8 temp; 
	temp=color;   
    LCD_REG=(0x63);//FGCR0
	LCD_RAM=(temp);
}								 
//---------------------------------------------//
//---------------------------------------------//
void BTE_Foreground_green(u16 color)
{	u8 temp;
	temp=color;   
    LCD_REG=(0x64);//FGCR1
	LCD_RAM=(temp);
}							 
//---------------------------------------------//

//---------------------------------------------//
void BTE_Foreground_blue(u16 color)
{
	u8 temp;
	temp=color;   
    LCD_REG=(0x65);//FGCR2
	LCD_RAM=(temp);
} 		
//清除显存
void Memory_Clear(void)
{
	u8 temp;
	LCD_REG=(0x8e);//MCLR
	temp = LCD_RAM;
	temp |= 0x80;
	LCD_REG=(0x8e);//MCLR
	LCD_RAM=(temp);
	Chk_Busy();
}
//启动画直线
void Draw_line(void)
{ 
  LCD_WR_REG(0x90);//DCR
  LCD_WR_DATA(0x80);
}
//启动画长方形
void Draw_square(void)
{
  LCD_REG=(0x90);//DCR
  LCD_RAM=(0x90);
}

//启动方形填充
void Draw_square_fill(void)
{
  LCD_REG=(0x90);//DCR
  LCD_RAM=(0xb0);
}
void Draw_circle(void)
{
 LCD_REG=(0x90);//DCR
 LCD_RAM=(0x40);
 Chk_Busy();
}
void Draw_circle_fill(void)
{
  LCD_REG=(0x90);//DCR
  LCD_RAM=(0x60);
  Chk_Busy();
}
//文本模式设定
void Text_Mode(void)
{
	u8 temp;

	LCD_REG=(0x40);//MWCR0
	temp = LCD_RAM;
	temp |= 0X80 ;
	LCD_REG=(0x40);//MWCR0
	LCD_RAM=(temp);	 
}
void Show_Cursor(void)
{
	LCD_REG=(0x02);
	LCD_RAM=(0x10);
	Chk_Busy();
}

void Del_Cursor(void)
{
	LCD_REG=(0x02);
	LCD_RAM=(0x00);
	Chk_Busy();
}
/**************************************************************************************
函数功能：画矩形
输入：    矩形的左上角和右下角坐标，和目标颜色
		  x1,y1  左上角坐标
		  x2,y2  右上角坐标
		  fill	 0  画矩形框
		  		 1  矩形填充
输出：	  无
**************************************************************************************/
void LcdFillRec(u16 x1, u16 y1, u16 x2, u16 y2, u8 fill, u8 color)
{
	Active_Window(0,480,0,272);	
	Text_Foreground_Color(color); // 设定颜色
	Geometric_Coordinate(x1,x2,y1,y2); // 设定矩形坐标
	LCD_REG=(0x90);
	if (fill)
	{
		LCD_RAM=(0xB0);
	}
	else
	{
		LCD_RAM=(0x90);
	}
	Chk_Busy();
}

/**************************************************************************************
函数功能：画矩形方框,立体感的组合形式
输入：    矩形的左上角和右下角坐标，和目标颜色
输出：	  无
(x1,y1)左上角坐标 (x2,y2)右下角坐标
color:前景色 bcolor:背景色
mode:1 字体加粗 0 不加粗
max:放大倍数
**************************************************************************************/
void LcdPrintRecZuHe(u16 x1, u16 y1, u16 x2, u16 y2, char *str1,u8 color,u8 bcolor,u8 mode,u8 max)
{
	LcdFillRec(x1+1, y1+8+1, x2+1, y2+1, 0, color);
	LcdFillRec(x1,   y1+8,   x2,   y2,   0, color);	
	Text_Mode();
	Text_Foreground_Color(color);
	Text_Background_Color(bcolor);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();	
	XY_Coordinate(x1+10,y1);
	if(mode) Bold_Font();//mode=1 字体加粗
	if(max==2) 
	{
		Horizontal_FontEnlarge_x2();
    Vertical_FontEnlarge_x2();
	}
	else if(max==3)
	{
		Horizontal_FontEnlarge_x3();
    Vertical_FontEnlarge_x3();
	}
	else
	{
		Horizontal_FontEnlarge_x1();
    Vertical_FontEnlarge_x1();
	}
	Show_String(str1);
	NoBold_Font();//取消字体加粗	
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();//取消放大倍数
	Chk_Busy();
}
//选择外部CGROM
void External_CGROM(void)
{
	u8 temp;

	LCD_REG=(0x21);//FNCR0
	temp = LCD_RAM;
	temp |= 0x20 ;
	LCD_REG=(0x21);//FNCR0
	LCD_RAM=(temp);
}
void  ASCII_Mode_disable(void)
{
	u8 temp;

	LCD_REG=(0x21);//FNCR0
	temp = LCD_RAM;
	temp &= 0xef ;
	LCD_REG=(0x21);//FNCR0
	LCD_RAM=(temp);
}

void ASCII_Mode_enable(void)
{
	u8 temp;

	LCD_REG=(0x21);//FNCR0
	temp = LCD_RAM;
	temp |= 0X10 ;
	LCD_REG=(0x21);//FNCR0
	LCD_RAM=(temp);
}
void External_CGROM_GB(void)
{
	u8 temp;

	LCD_REG=(0x21);//FNCR0
	temp = LCD_RAM;
	temp &= 0xf3 ;
	LCD_REG=(0x21);//FNCR0
	LCD_RAM=(temp);
}
//显示字符串
void Show_String(char *str)
{  
	 Text_Mode();
	LCD_REG=(0x02);
	while(*str != '\0')
	{
	 LCD_RAM=(*str);
	 ++str;	 	
	 Chk_Busy();		
	}
 Chk_Busy();
}

//显示指定长度的字符串
void Show_StringNum(char *str,u8 len)
{  
	u8 len1=0;
	 Text_Mode();
	LCD_REG=(0x02);
	while((*str != '\0')&&(len1!=len))
	{
	 LCD_RAM=(*str);
	 ++str;
	 len1++;			
	 Chk_Busy();		
	}
 Chk_Busy();
}
void Horizontal_FontEnlarge_x1(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp &= 0xf3 ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void Horizontal_FontEnlarge_x2(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp &= 0xf3 ;
	temp |= 0x04 ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void Horizontal_FontEnlarge_x3(void)
{
	u8 temp;
	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp &= 0xf3 ;
	temp |= 0x08 ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void Vertical_FontEnlarge_x1(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp &= 0xfc ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void Vertical_FontEnlarge_x2(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp &= 0xfc ;
	temp |= 0x01 ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void Vertical_FontEnlarge_x3(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp &= 0xfc ;
	temp |= 0x02 ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void Font_with_BackgroundTransparency(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp =LCD_RAM;
	temp |=0X40 ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void Font_with_BackgroundColor(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp &= 0xbf ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void NoBackgroundColor(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp |= 0x40;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void BackgroundColor(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp &= 0xBF;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void NoBold_Font(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp &= 0xdf ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}

void Bold_Font(void)
{
	u8 temp;

	LCD_REG=(0x22);//FNCR1
	temp = LCD_RAM;
	temp |= 0x20 ;
	LCD_REG=(0x22);//FNCR1
	LCD_RAM=(temp);
}
void Circle_Coordinate_Radius(u16 X,u16 Y,u16 R)
{
      
   LCD_REG=(0x99);
	LCD_RAM=(X);  
    LCD_REG=(0x9a);	   
	LCD_RAM=(X>>8);  
	   
    LCD_REG=(0x9b);
	LCD_RAM=(Y); 
    LCD_REG=(0x9c);	   
	LCD_RAM=(Y>>8);
 
    LCD_REG=(0x9d);
	LCD_RAM=(R);
}
/*
*********************************************************************************************************
*	函 数 名: RA8870_SetCursor
*	功能说明: 设置读显存的光标位置。 很多其他的控制器写光标和读光标是相同的寄存器，但是RA8875是不同的。
*	形    参:  _usX : X坐标; _usY: Y坐标
*	返 回 值: 无
*********************************************************************************************************
*/
static void RA8870_SetReadCursor(uint16_t _usX, uint16_t _usY)
{
	/* 设置内存读光标的坐标 */	
	LCD_REG=0x4A;
	LCD_RAM=_usX;	
	LCD_REG=0x4B;
	LCD_RAM=(_usX>>8);
	

	LCD_REG=0x4C;
	LCD_RAM=_usY;	
	LCD_REG=0x4D;
	LCD_RAM=(_usY >> 8);
}
/*
*********************************************************************************************************
*	函 数 名: RA8870_PutPixel
*	功能说明: 画1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  :像素颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8870_PutPixel(u16 X, u16 Y, u16 _usColor)
{		
	u8 temp;
	LCD_REG=0x46;
	LCD_RAM=X;	
  LCD_REG=0x47;	   
	LCD_RAM=X>>8;

	
  LCD_REG=0x48;
	LCD_RAM=Y;	 
  LCD_REG=0x49;	   
	LCD_RAM=(Y>>8);
	
	LCD_REG = 0x02;	
	temp=_usColor>>8;
	LCD_RAM=temp;
 	temp=_usColor;
 	LCD_RAM=temp;
}
/*
*********************************************************************************************************
*	函 数 名: RA8870_setpoint
*	功能说明: 设置显示区域
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  :像素颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void RA8870_setpoint(u16 X, u16 Y)
{	
	LCD_REG=0x46;
	LCD_RAM=X;	
  LCD_REG=0x47;	   
	LCD_RAM=X>>8;

	
  LCD_REG=0x48;
	LCD_RAM=Y;	 
  LCD_REG=0x49;	   
	LCD_RAM=(Y>>8);

	LCD_REG = 0x02;	
		
}

/*
*********************************************************************************************************
*	函 数 名: RA8875_GetPixel
*	功能说明: 读取1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  :像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
uint16_t RA8870_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;

	//RA8875_WriteReg(0x40, (1 << 0));	/* 设置为绘图模式，读取光标不自动加1 */
	LCD_REG=0x40; 
	LCD_RAM=(1 << 0);
	RA8870_SetReadCursor(_usX, _usY);	/* 设置读取光标位置 */	

	LCD_REG=0x02;
	usRGB = LCD_RAM;	/* 第1次读取数据丢弃 */
	usRGB = LCD_RAM;
	usRGB = LCD_RAM;
	return usRGB;
}
/*
*********************************************************************************************************
*	函 数 名: BTE_SetTarBlock
*	功能说明: 设置RA8875 BTE目标区块以及目标图层
*	形    参:
*			uint16_t _usX : 水平起点坐标
*			uint16_t _usY : 垂直起点坐标
*			uint16_t _usHeight : 区块高度
*			uint16_t _usWidth : 区块宽度
*			uint8_t _ucLayer : 0 图层1； 1 图层2
*	返 回 值: 无
*********************************************************************************************************
*/
void BTE_SetTarBlock(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t _ucLayer)
{
	/* 设置起点坐标 */
	LCD_REG=0X58; LCD_RAM= _usX;
	LCD_REG=0X59; LCD_RAM= _usX>>8;
	LCD_REG=0X5A; LCD_RAM= _usY;
	
	if (_ucLayer == 0)	/* 图层2 */
	{
		LCD_REG=0X5B; LCD_RAM= _usY>>8;
	}
	else
	{
		LCD_REG=0X5B; LCD_RAM= (1 << 7) | (_usY >> 8);
	}

	/* 设置区块宽度 */
	LCD_REG=0X5C; LCD_RAM= _usWidth;
	LCD_REG=0X5D; LCD_RAM= _usWidth>>8;
	/* 设置区块高度 */
	LCD_REG=0X5E; LCD_RAM= _usHeight;
	LCD_REG=0X5F; LCD_RAM= _usHeight>>8;
	
}
/*
*********************************************************************************************************
*	函 数 名: BTE_SetOperateCode
*	功能说明: 设定BTE 操作码和光栅运算码
*	形    参: _ucOperate : 操作码
*	返 回 值: 无
*********************************************************************************************************
*/
void BTE_SetOperateCode(uint8_t _ucOperate)
{
	/*  设定BTE 操作码和光栅运算码  */
	LCD_REG=0X51; LCD_RAM= _ucOperate;
}
/*
*********************************************************************************************************
*	函 数 名: BTE_Start
*	功能说明: 启动BTE操作
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void BTE_Start(void)
{		
	LCD_REG=0X50; LCD_RAM= 0X80;
}
uint8_t RA8870_ReadStatus(void)
{
	uint8_t value = 0;	
	value = LCD_REG;
	return value;	
}
/*
*********************************************************************************************************
*	函 数 名: BTE_Wait
*	功能说明: 等待BTE操作结束
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void BTE_Wait(void)
{
 	while ((RA8870_ReadStatus() & 0x40) == 0x40);	
}
void BTE_ROP_Code(u8 CODE)
{
	 u8 temp;  
//	temp=CODE>>4;   
//    LCD_REG=(0x51);//BECR1
//	LCD_RAM=(temp);
	temp=CODE;   
  LCD_REG=(0x51);//BECR1	   
	LCD_RAM=(temp); 
}	
void BTE_enable(void)
{
	u8 temp;

	LCD_REG=(0x50);//BECR0
	temp = LCD_RAM;
	temp |= 0X80 ;
	LCD_REG=(0x50);//BECR0
	LCD_RAM=(temp);  
}
void Enable_TP(void)
{
// 	u8 temp;
// 	u8 a,b;
// 	a=0x40;
// 	b=0x04;
// 	LCD_REG=(0x70);
// 	temp=LCD_RAM;
// 	temp|=0x80;
// 	temp|=a;
// 	temp|=b;
// 	LCD_REG=(0x70);
// 	LCD_RAM=(temp);

	LCD_REG=(0x70);
	LCD_RAM=0x82;
}
void Set_4wire_TP(void)
{	u8 temp;
  LCD_REG=(0x71);//TPCR1	
	temp = LCD_RAM;
	temp |= 0x80;
	LCD_REG=(0x71);;//TPCR1
	LCD_RAM=(temp);
}
void TP_auto_mode(void)
{	u8 temp;
  	LCD_REG=(0x71);//TPCR1	
	temp = LCD_RAM;
	temp &= 0xbf;
	LCD_REG=(0x71);//TPCR1
	LCD_RAM=(temp);
}
void enable_TP_interrupt(void)
{ u8 temp;
  LCD_REG=(0x8F);//INTC
  temp=LCD_RAM; 
  temp |= 0x40;
  LCD_RAM=(temp);

 }
//清除中断标志位
void clear_TP_interrupt(void)
{ u8 temp;
  LCD_REG=0x8f;//INTC
  temp=LCD_RAM; 
  temp |=0X04;
  LCD_RAM=temp;

}
//触摸屏初始化
void RA8870_TouchInit(void)
{	

	Enable_TP();
	Set_4wire_TP();
	TP_auto_mode();
	Chk_Busy();
	enable_TP_interrupt();
	Memory_Clear();
	Chk_Busy();
	clear_TP_interrupt();
}

//==============================================================================
//Subroutine:	TP status
//==============================================================================
u8 Touch_Status(void)
{	u8 temp; 
	LCD_REG=(0x8f);//INTC	
	//Chk_Busy();
	temp = LCD_RAM;	
	if ((temp&0x04)==0x04)
		return 1;
	else 
		return 0;
}
void LcdPrint8bitBmp(const u8* image,u16 x,u16 y,u16 widht,u16 height)
{
	//u8  color8;
	u16 w,h;
	u8 temp;
	//u16 r,g,b;
   LCD_REG=(0x10);  //SYSR   u8[4:3]=00 256 color  u8[2:1]=  00 8u8 MPU interface
 	temp=LCD_RAM;
	temp&=0x03;
	temp|=0x30;
	LCD_REG=(0x10); 
	LCD_RAM=temp;     //256色(RRRGGGBB) 
	Delay1ms(10);
	Active_Window(x,x+widht,y,y+height);
	for(h=0;h<height;h++)
	{
		XY_Coordinate(x,y+h);
		LCD_REG=(0x02);		//MRWC  REG[02h] -- Memory Read/Write Command
		for(w = 0; w< widht; w++)
		{
			LCD_RAM=(*image++);		
			Chk_Busy();
		}
	}
 	LCD_REG=(0x10);  //SYSR   u8[4:3]=00 256 color  u8[2:1]=  00 8u8 MPU interface
	temp=LCD_RAM;
	temp|=0x0F;
	LCD_REG=(0x10);
	LCD_RAM=temp;     //65535色 
	Delay1ms(1);
}
/**************************************************************************************
函数功能：	画水平直线
输入	：  WORD  x    		起点的横坐标
	  		WORD  y  	    起点的纵坐标
	  		WORD  width		直线长度
	  		WORD  color		颜色
输出	：	无
**************************************************************************************/
void LcdPrintHorz(u16 x, u16 y, u16 width, u16 color)
{
	Active_Window(0,480,0,272);
	Text_Foreground_Color(color); // 设定颜色 
	Geometric_Coordinate(x,x+width-1,y,y); // 设置水平线起始点
	LCD_REG=(0x90);//写0x90寄存器
	LCD_RAM=(0x80);   //向0x90寄存器写数据
	Chk_Busy();
}
/**************************************************************************************
函数功能：  画垂直直线
输入	：  u16  x    	    起点的横坐标
	  		u16  y  	    起点的纵坐标
	  		u16  height		直线高度
	  		u16  color		颜色
输出	：	无
**************************************************************************************/
void LcdPrintVert(u16 x, u16 y, u16 height, u16 color)
{
	Active_Window(0,480,0,272);
	Text_Foreground_Color(color); // 设定颜色 
	Geometric_Coordinate(x,x,y,y+height-1); // 设置起始点坐标
	LCD_REG=(0x90);//写0x90寄存器
	LCD_RAM=(0x80);   //向0x90寄存器写数据
	Chk_Busy();
}



/**************************************************************************************
功能描述: 在屏幕显示一输入框,有立体感的
输    入: u16 x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void DisplayEdit(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	LcdPrintHorz(x1,  y1,  x2 - x1+1, color_grey2);
	LcdPrintHorz(x1+1,y1+1,x2 - x1-1, color_grey1);
	LcdPrintVert(x1,  y1,  y2 - y1+1, color_grey2);
	LcdPrintVert(x1+1,y1+1,y2 - y1-1, color_grey1);
	LcdPrintHorz(x1,  y2,  x2 - x1+1, color_white);
	LcdPrintVert(x2,  y1,  y2 - y1+1, color_white);
	LcdFillRec(  x1+2,y1+2,x2 - 1, y2-1,1,color);
}
/**************************************************************************************
功能描述: 在屏幕显示一凹下的按钮框
输    入: u16 x1,y1,x2,y2 按钮框左上角和右下角坐标
输    出: 无
**************************************************************************************/
void DisplayButtonDown(u16 x1,u16 y1,u16 x2,u16 y2)
{
	LcdPrintHorz(x1,  y1,  x2 - x1+1, color_grey2);
	LcdPrintHorz(x1+1,y1+1,x2 - x1-1, color_grey1);
	LcdPrintVert(x1,  y1,  y2 - y1+1, color_grey2);
	LcdPrintVert(x1+1,y1+1,y2 - y1-1, color_grey1);
	LcdPrintHorz(x1,  y2,  x2 - x1+1, color_grey2);
	LcdPrintVert(x2,  y1,  y2 - y1+1, color_grey1);
}
void DisplayButtonUp(u16 x1,u16 y1,u16 x2,u16 y2)
{
	LcdPrintHorz(x1,  y1,  x2 - x1+1, color_grey1);
	LcdPrintVert(x1,  y1,  y2 - y1+1, color_grey2);
	
	LcdPrintHorz(x1+1,y2-1,x2 - x1-1, color_grey1);
	LcdPrintHorz(x1,  y2,  x2 - x1+1, color_grey2);
	LcdPrintVert(x2-1,y1+1,y2 - y1-1, color_grey1);
  LcdPrintVert(x2  ,y1  ,y2 - y1+1, color_grey2);
}
//键盘的按钮
void DisplayButton(u16 x1,u16 y1,u16 x2,u16 y2)
{
	LcdPrintHorz(x1,  y1,  x2 - x1+1, color_grey1);
	LcdPrintVert(x1,  y1,  y2 - y1+1, color_grey2);
	
	LcdPrintHorz(x1+1,y2-1,x2 - x1-1, color_grey1);
	LcdPrintHorz(x1,  y2,  x2 - x1+1, color_grey2);
	LcdPrintVert(x2-1,y1+1,y2 - y1-1, color_grey1);
  LcdPrintVert(x2  ,y1  ,y2 - y1+1, color_grey2);
	LcdFillRec(x1+2,y1+2,x2-2,y2-2,1,color_cyan);
	
}
/********************************************************************/
//函数功能：放置按键
//(x,y)矩形按键中心坐标
//len:按键长度
//wid:按键宽度
/********************************************************************/
void Put_AnJian(u16 x, u16 y, u16 len, u16 wid)
{
	u16 lx_inside, ly_inside, rx_inside, ry_inside;
	Active_Window(0,479,0,271);	
	//显示图片
	Graphic_Mode();
	lx_inside = x - len/2;
	ly_inside = y - wid/2;
	rx_inside = x + len/2;
	ry_inside = y + wid/2;	
	LcdFillRec(lx_inside,ly_inside,rx_inside,ry_inside,1,color_cyan);
	
	LcdFillRec(lx_inside-1, ly_inside-1, lx_inside-1, ry_inside+1,1, color_grey2);	//画左垂直边框线,汪色
	LcdFillRec(lx_inside-2, ly_inside-2, lx_inside-2, ry_inside+2,1, color_grey2);
	LcdFillRec(lx_inside-3, ly_inside-3, lx_inside-3, ry_inside+3,1,color_grey2);
	
	LcdFillRec(lx_inside-1, ly_inside-1, rx_inside+1, ly_inside-1,1, color_grey2);	//画上边框线,浅色
	LcdFillRec(lx_inside-2, ly_inside-2, rx_inside+2, ly_inside-2,1,color_grey2);
	LcdFillRec(lx_inside-3, ly_inside-3, rx_inside+3, ly_inside-3,1, color_grey2);	

	LcdFillRec(rx_inside+1, ly_inside,   rx_inside+1, ry_inside+1,1, color_grey1);		//画右垂直边框线,深色
	LcdFillRec(rx_inside+2, ly_inside-1, rx_inside+2, ry_inside+2,1,color_grey1);
	LcdFillRec(rx_inside+3, ly_inside-2, rx_inside+3, ry_inside+3,1, color_grey1);
	
	LcdFillRec(lx_inside-1, ry_inside,   rx_inside+1, ry_inside-1,1, color_grey1);		//画下边框线,深色
	LcdFillRec(lx_inside-2, ry_inside+1, rx_inside+2, ry_inside+2,1, color_grey1);
	LcdFillRec(lx_inside-3, ry_inside+2, rx_inside+3, ry_inside+3,1, color_grey1);	
	/*if (len > 50) //当按键长度大于50时，边框线变宽
	{
		LcdFillRec(lx_inside-4, ly_inside-4, lx_inside-4, ry_inside+4,1, color_grey2); //画左垂直边框线,浅色
		LcdFillRec(lx_inside-5, ly_inside-5, lx_inside-5, ry_inside+5,1, color_grey2);
		LcdFillRec(lx_inside-6, ly_inside-6, lx_inside-6, ry_inside+6,1, color_grey2);
	
		LcdFillRec(lx_inside-4, ly_inside-4, rx_inside+4, ly_inside-4,1, color_grey2); //画上边框线,浅色
		LcdFillRec(lx_inside-5, ly_inside-5, rx_inside+5, ly_inside-5,1, color_grey2);
		LcdFillRec(lx_inside-6, ly_inside-6, rx_inside+6, ly_inside-6,1, color_grey2);
	
		LcdFillRec(rx_inside+4, ly_inside-3, rx_inside+4, ry_inside+4,1, color_grey1);	//画右垂直边框线,深色
		LcdFillRec(rx_inside+5, ly_inside-4, rx_inside+5, ry_inside+5,1, color_grey1);
		LcdFillRec(rx_inside+6, ly_inside-5, rx_inside+6, ry_inside+6,1, color_grey1);
	
		LcdFillRec(lx_inside-4, ry_inside+3, rx_inside+4, ry_inside+4,1, color_grey1);	 //画下边框线,深色
		LcdFillRec(lx_inside-5, ry_inside+4, rx_inside+5, ry_inside+5,1, color_grey1);
		LcdFillRec(lx_inside-6, ry_inside+5, rx_inside+6, ry_inside+6,1, color_grey1);
	}*/
	Text_Mode();	
}
void AnJianPressed(u16 x, u16 y, u16 len, u16 wid)
{
	u16 lx_inside, ly_inside, rx_inside, ry_inside;
	Active_Window(0,479,0,271);	
	//显示图片
	Graphic_Mode();
	lx_inside = x - len/2;
	ly_inside = y - wid/2;
	rx_inside = x + len/2;
	ry_inside = y + wid/2;
	LcdFillRec(lx_inside,ly_inside,rx_inside,ry_inside,1,color_cyan); 						//按键主颜色

	LcdFillRec(lx_inside-1, ly_inside-1, lx_inside-1, ry_inside+1,1, color_grey1);	//画左垂直边框线
	LcdFillRec(lx_inside-2, ly_inside-2, lx_inside-2, ry_inside+2,1, color_grey1);
	LcdFillRec(lx_inside-3, ly_inside-3, lx_inside-3, ry_inside+3,1, color_grey1);

	LcdFillRec(lx_inside-1, ly_inside-1, rx_inside+1, ly_inside-1,1, color_grey1);	//画上边框线
	LcdFillRec(lx_inside-2, ly_inside-2, rx_inside+2, ly_inside-2,1, color_grey1);
	LcdFillRec(lx_inside-3, ly_inside-3, rx_inside+3, ly_inside-3,1, color_grey1);
	
	LcdFillRec(rx_inside+1, ly_inside,   rx_inside+1, ry_inside+1,1, color_grey2);	//画右垂直边框线
	LcdFillRec(rx_inside+2, ly_inside-1, rx_inside+2, ry_inside+2,1, color_grey2);
	LcdFillRec(rx_inside+3, ly_inside-2, rx_inside+3, ry_inside+3,1, color_grey2);
	
	LcdFillRec(lx_inside-1, ry_inside,   rx_inside+1, ry_inside-1,1, color_grey2);	//画下边框线
	LcdFillRec(lx_inside-2, ry_inside+1, rx_inside+2, ry_inside+2,1, color_grey2);
	LcdFillRec(lx_inside-3, ry_inside+2, rx_inside+3, ry_inside+3,1, color_grey2);
	/*if (len > 50)  //当按键长度大于50时，边框线变宽
	{
		LcdFillRec(lx_inside-4, ly_inside-4, lx_inside-4, ry_inside+4,1, color_grey1);	//画左垂直边框线
		LcdFillRec(lx_inside-5, ly_inside-5, lx_inside-5, ry_inside+5,1, color_grey1);
		LcdFillRec(lx_inside-6, ly_inside-6, lx_inside-6, ry_inside+6,1,color_grey1);

		LcdFillRec(lx_inside-4, ly_inside-4, rx_inside+4, ly_inside-4,1, color_grey1);	//画上边框线
		LcdFillRec(lx_inside-5, ly_inside-5, rx_inside+5, ly_inside-5,1, color_grey1);
		LcdFillRec(lx_inside-6, ly_inside-6, rx_inside+6, ly_inside-6,1, color_grey1);

		LcdFillRec(rx_inside+4, ly_inside-3, rx_inside+4, ry_inside+4,1, color_grey2); //画右垂直边框线
		LcdFillRec(rx_inside+5, ly_inside-4, rx_inside+5, ry_inside+5,1, color_grey2);
		LcdFillRec(rx_inside+6, ly_inside-5, rx_inside+6, ry_inside+6,1, color_grey2);

		LcdFillRec(lx_inside-4, ry_inside+3, rx_inside+4, ry_inside+4,1, color_grey2); //画下边框线
		LcdFillRec(lx_inside-5, ry_inside+4, rx_inside+5, ry_inside+5,1, color_grey2);
		LcdFillRec(lx_inside-6, ry_inside+5, rx_inside+6, ry_inside+6,1, color_grey2); 	
	}*/
		Text_Mode();
}
//显示时间和日期
void display_time(void)
{
	u8 t;		
	if(t!=calendar.sec)
		{
			t=calendar.sec;	
			Text_Foreground_Color(color_blue);
			Text_Background_Color(color_cyan);//设定文字的前景色和背景色
			sprintf(Data,"%04d-%02d-%02d",calendar.w_year,calendar.w_month,calendar.w_date);
			XY_Coordinate(380,17);
			Show_StringNum(Data,10);
			sprintf(Time,"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);
			XY_Coordinate(385,32);
			Show_StringNum(Time,8);	
		}	
 		Delay1ms(10);	
}
