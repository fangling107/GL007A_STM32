#ifndef __ra8870_H
#define __ra8870_H	
#include "stm32f10x.h"
//////////////////////////////////////////////////////////////////////////////////	 
//7寸 TFT液晶驱动	  	
//********************************************************************************
//无
//////////////////////////////////////////////////////////////////////////////////
struct oncetest//7b
{	
	u8 once_xuhao;//测试序号	
	u8 once_kpa[2];//压差结果
	u8 once_gas_res[4];//一次测试的通气阻力
	
};

//58b
struct data_diapaly  //;显示一组数据的结构体(包含五次测试数据)
{
    u8  Mark;				//	组号（五次测试为一组）
    u8 newyears[2];    //;年
    u8 newmonth;    //;月
    u8 newday;     //;日		
    u8 Ka_max[2];     //;一组中压差结果的最大值
		u8 Ka_avg[2];     //;一组中压差结果的平均值
		u8 Ka_min[2];     //;一组中压差结果的最小值
		u8 Km_max[4];     //;一组中通气阻力的最大值
		u8 Km_avg[4];     //;一组中通气阻力的平均值
		u8 Km_min[4];     //;一组中通气阻力的最小值
    struct oncetest  onedata[5];//35b		
};

extern struct data_diapaly data_once[10];//一组数据（5次测试样本数据）,保存10组
//A10  RS
#define RA8875_BASE		((uint32_t)(0x6C000000 | 0x00000000))
#define LCD_REG		*(__IO uint16_t *)(RA8875_BASE +  (1 << (10 + 1)))	
#define LCD_RAM		*(__IO uint16_t *)(RA8875_BASE)
#define	LCD_StatusRead()	*(__IO uint16_t *)(RA8875_BASE +  (1 << (10 + 1))) //if use read  Mcu interface DB0~DB15 needs increase pull high 
#define	LCD_DataRead()   	*(__IO uint16_t *)(RA8875_BASE) //if use read  Mcu interface DB0~DB15 needs increase pull high


#define color_black   0x00		
#define color_white   0xff
#define color_red     0xE0
#define color_green   0x1C
#define color_blue    0x03
#define color_grey1   0x6d		//灰色1 01101101
#define color_grey2   0xB6		//灰色2 10110110
#define color_yellow  color_red|color_green
#define color_cyan    color_green|color_blue
#define color_purple  color_red|color_blue

void Chk_Busy(void);	    															  
void LCDRA8870_Init(void);													   	//初始化
void LCD_WR_DATA(u8 data);
void LCD_WR_REG(u8 regval);
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue);
u8 LCD_ReadReg(u8 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);		
void Display_ON(void);
void Display_OFF(void);
void Delay1ms(u16 x);
void Graphic_Mode(void);
void Text_Foreground_Color(u8 color);
void Text_Background_Color(u8 color);
void Memory_Clear_with_Font_BgColor(void);
void Font_with_BackgroundColor(void);
void Clear_Full_Window(void);
void Active_Window(u16 XL,u16 XR ,u16 YT ,u16 YB);
void Memory_Clear(void);
uint16_t RA8870_GetPixel(uint16_t _usX, uint16_t _usY);
void RA8870_PutPixel(u16 X, u16 Y, u16 _usColor);
void XY_Coordinate(u16 X,u16 Y);
void RA8870_setpoint(u16 X, u16 Y);
void Bold_Font(void);
void NoBold_Font(void);
void Geometric_Coordinate(u16 XL,u16 XR ,u16 YT ,u16 YB);
void Draw_square_fill(void);
void Draw_square(void);
void Draw_circle(void);
void Draw_circle_fill(void);
void Text_Mode(void);
void External_CGROM_GB(void);
void ASCII_Mode_enable(void);
void  ASCII_Mode_disable(void);
void External_CGROM(void);
void Show_String(char *str);
void Text_Color(u8 color);
void Horizontal_FontEnlarge_x1(void);
void Vertical_FontEnlarge_x1(void);
void Horizontal_FontEnlarge_x2(void);
void Vertical_FontEnlarge_x2(void);
void Vertical_FontEnlarge_x3(void);
void Horizontal_FontEnlarge_x3(void);
void Font_with_BackgroundTransparency(void);
void NoBackgroundColor(void);
void BackgroundColor(void);
void Circle_Coordinate_Radius(u16 X,u16 Y,u16 R);
void RA8875_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor);
void BTE_SetTarBlock(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t _ucLayer);
void BTE_SetOperateCode(uint8_t _ucOperate);
void BTE_Wait(void);
void Chk_Busy_BTE(void);
void BTE_Start(void);
void RA8870_TouchInit(void);
void clear_TP_interrupt(void);
u8 Touch_Status(void);
uint16_t RA8870_TouchReadY(void);
uint16_t RA8870_TouchReadX(void);
void BTE_Size_setting(u8 width,u8 height);
void BTE_Source_Destination	(u16 SX,u16 DX ,u16 SY ,u16 DY);
void BTE_Background_red(u16 color);
void BTE_Background_green(u16 color);
void BTE_Background_blue(u16 color);
void BTE_Foreground_red(u16 color);
void BTE_Foreground_green(u16 color);
void BTE_Foreground_blue(u16 color);
void BTE_ROP_Code(u8 CODE);
void BTE_enable(void);
void DisplayGif(void);
void LcdFillRec(u16 x1, u16 y1, u16 x2, u16 y2, u8 fill, u8 color);
void LcdPrint8bitBmp(const u8* image,u16 x,u16 y,u16 widht,u16 height);
void Show_StringNum(char *str,u8 len);
void LcdPrintRecZuHe(u16 x1, u16 y1, u16 x2, u16 y2, char *str1,u8 color,u8 bcolor,u8 mode,u8 max);
void DisplayEdit(u16 x1,u16 y1,u16 x2,u16 y2,u16 color);
void DisplayButtonDown(u16 x1,u16 y1,u16 x2,u16 y2);
void DisplayButtonUp(u16 x1,u16 y1,u16 x2,u16 y2);
void display_time(void);
void DisplayButton(u16 x1,u16 y1,u16 x2,u16 y2);
void Show_Cursor(void);
void Del_Cursor(void);
void LcdPrintHorz(u16 x, u16 y, u16 width, u16 color);
void LcdPrintVert(u16 x, u16 y, u16 height, u16 color);
void Put_AnJian(u16 x, u16 y, u16 len, u16 wid);
void AnJianPressed(u16 x, u16 y, u16 len, u16 wid);
#endif  
	 
	 



