#include "stm32f10x.h"
#include "setup.h"
#include "ra8870.h"
//#include "malloc.h"
#include "24cxx.h" 
#include "touch.h" 
#include "flash.h" 
#include "picture.h"
#include "rtc.h" 
#include "ads8341.h"
#include "printer.h"
#include "string.h"
#include "stdio.h"

//定义EEPROM的保存地址 0-13 屏幕校准系数
//14：试剂系数
//15：测试次数
//16：测试组数
//17-18：AD K整数部分 19-20：AD K小数部分
//21-22: AD C整数部分 23-24：AD C小数部分
#define SAVE_ADDR_XISHU 14 //EEPROM保存系数的地址 14开始 （0-13保存的是屏幕校准系数）
#define SAVE_ADDR_CISHU 15	//EEPROM保存测试次数的地址 15开始 
#define SAVE_ADDR_ZUSHU 16	//EEPROM保存测试组数的地址 16开始
 
//AD校准密码
#define password 70605

//参数声明
static u8 key_value;//按键键值

static u8 test_cishu=1;//测试序号
static u8 test_zunum=1;//测试组号
extern float press_k;//曲线斜率
extern float press_c;//曲线斜率
//250ms標誌
u8 RTC_200MS=0;
u8 RTC_250MS=0;
//开/关气标志
static u8 open_off_flag;

u16 pressAD_kint,pressAD_kfloat;//曲线系数的整数和小数部分
u16 pressAD_cint,pressAD_cfloat;//曲线系数的整数和小数部分

u16 AD1value=0,AD2value=0,biaoding1=0,biaoding2=0;//AD采样值

u16 press_zeroAD=0;//零点AD
float gas_res=0;//通气阻力
u8 test_succesflag=0;//测试成功标志
u8 checkTestnum=0;//用于查询测试组


extern u16 pressADvalue;//AD值
volatile u16 pressvalue_avg;//AD值  平均值 为压差结果
static u8 password_flag;
static u8 slect_print=1;
static u8 printf_flag=1;
//函数声明
void menu1(void);//初始界面
void menu_setting(void);//设置界面
void menu_menu(void);//菜单
void key_function(void);
void display_number(void);
void JianPan_Displayfu(void);//显示虚拟键盘
void PrintReport(void);
void speekers(u16 time);//蜂鸣器 time/1ms
void AD_Calibration_menu(void);//AD校准界面
void AD_Calibration_Function(void);//AD校准
u16 biaoding_menu(void);//标定
u16 display_press(u16 pressAD,u16 x,u16 y);//指定位置显示压力值
float display_holdsize(u16 pressAD,u16 x,u16 y);//指定位置显示孔径值
void menu_flashmax(void);//数据区满提示
void save_data(void);//保存测试数据到EEPROM
void menu_dataQuery(void);
void data_Query(void);
void display_pressspeed(void);
u16 paixu(u8 times,u16 rcv_data[]);
void menu_password(void);
void menu_printselect(void);
void set_printnum(void);
void show_excel(void);
void display_pre(u16 press,u16 x,u16 y);
void off_gas(void);
void open_gas(void);
void test_failule(void);
void flash_full(void);
void plese_opengas(void);
//数组声明
//struct oncetest  onedata[5];//80b 用于存储5次测试数据
struct data_diapaly data_once[10];//用于存储10组测试数据
//主函数
int main(void)
 {	
	u8 b[16]={0};

  InitHardware();										//硬件初始化
	//my_mem_init(0);										//内存分配管理
	while(RTC_Init())	;								//RTC初始化
	AT24CXX_Init();										//AT24C02初始化
	while(AT24CXX_Check());						//检测24c02	
	

	Printer_Init();										//打印端口初始化	
	SPI_Flash_Init();									//W25Q16初始化				
	SPI_AD8341_Init();								//AD8341初始化
	LCDRA8870_Init();;								//LCD的初始化	
	RA8870_TouchInit();								//触摸屏初始化
	Adc_Init();
	while(TP_Get_Adjdata()==0)						//读取校准参数，看是否校准过
		XY_Calibration_Function();				//触摸屏校准
	//读取保存的压力AD系数
	AT24CXX_Read(b,17,16);
	pressAD_kint=(b[0]<<8)+b[1];
	pressAD_kfloat=(b[2]<<8)+b[3];//k
	pressAD_cint=(b[4]<<8)+b[5];
	pressAD_cfloat=(b[6]<<8)+b[7];//c
	press_k=(float)pressAD_kint+((float)pressAD_kfloat/10000);
	press_c=(float)pressAD_cint+((float)pressAD_cfloat/10000);
	biaoding1=(b[8]<<8)+b[9];	
	AD1value=(b[10]<<8)+b[11];
	biaoding2=(b[12]<<8)+b[13];	
	AD2value=(b[14]<<8)+b[15];
	
	//读取测试组号和测试次数
	test_cishu=	AT24CXX_ReadOneByte(SAVE_ADDR_CISHU);	
	test_zunum=	AT24CXX_ReadOneByte(SAVE_ADDR_ZUSHU);
	if(test_cishu>5) test_cishu=1;
	SPI_Flash_Read(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,30+7*(test_cishu-1));
	menu1();//加载初始菜单界面	
	speekers(500);//蜂鸣器响500mS
	
	while(1)
	{		
		if(RTC_250MS)
		{
			RTC_250MS=0;
			pressADvalue=Get_Adc_Average(10, 50);;//12位AD
			display_press(pressADvalue,280,110);//显示实时压力
			display_time();//显示时间
		}		
		key_function();//主界面的按键功能函数
	}
}

//主界面的按键功能函数
void key_function(void)
{
	static u8 j,k;
	u16 data_press[5]={0};

		KeyScan();//进行按键扫描，并转换为屏幕坐标值
		if(touch_flag)//触摸屏被按下
		{	
			if((LCD_X>30)&&(LCD_X<105)&&(LCD_Y>215)&&(LCD_Y<255))//菜单按钮被按下
				{						
					key_value=1;//设置按键标志	
					speekers(100);//蜂鸣器响100ms
					touch_flag=0;//标记按键已经被处理过了.	
				}
			else if((LCD_X>145)&&(LCD_X<220)&&(LCD_Y>215)&&(LCD_Y<255))//通气按钮被按下
				{				
					key_value=2;//	菜单按键标志
					speekers(100);//蜂鸣器响100ms					
					touch_flag=0;//标记按键已经被处理过了.	
				}
			else if((LCD_X>260)&&(LCD_X<335)&&(LCD_Y>215)&&(LCD_Y<255))//清零按钮被按下
				{						
					key_value=3;//	停止按键标志
					speekers(100);//蜂鸣器响100ms
					touch_flag=0;//标记按键已经被处理过了.
				}	
			else if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//测试按钮被按下
				{					
					key_value=4;//	测试按键标志
					touch_flag=0;//标记按键已经被处理过了.	
					speekers(100);//蜂鸣器响100ms
					if(test_zunum>10)											
						{
							flash_full();
							key_value = 0;
						}				
					else 
					{
						if(open_off_flag==0)
						{
							plese_opengas();
							key_value = 0;
						}						
					}	
				}
			else
				{
					key_value=0;						
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{
					Text_Foreground_Color(color_blue);
					Text_Background_Color(color_cyan);//设定文字的前景色和背景色
					switch(key_value)
					{
						case 1:  //菜单
							key_value = 0;
							menu_menu();												
							menu1();//加载初始菜单界面
							display_number();
							display_pre(pressvalue_avg,280,140);												
							gas_res=display_holdsize(pressvalue_avg,280,170);//显示通气阻力
							break;
						case 2://通气
							key_value = 0;
							j++;
							if(j==1) 
							{
								open_gas();
								open_off_flag=1;
							}
							else
							{
								j=0;
								open_off_flag=0;
								off_gas();															
							}			
							break;											
						case 3://清零
							key_value = 0;
						//数据清除
							press_zeroAD=pressADvalue;										
							gas_res=0;
							display_number();
							display_press(0,280,110);//显示实时压力
							display_press(0,280,140);//显示压差结果															
							display_holdsize(0,280,170);//显示通气阻力
								
							Graphic_Mode();	
							LcdFillRec(0,0,360,60,1,color_cyan);	
							LcdPrint8bitBmp(gImage_222,5,10,174,40);
							Text_Mode();
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
							External_CGROM();
							External_CGROM_GB();
							Active_Window(0,480,0,272);//设定显示区域
							XY_Coordinate(50,15);
							Horizontal_FontEnlarge_x2();
							Vertical_FontEnlarge_x2();
							Font_with_BackgroundTransparency();//开启穿透功能
							Show_String("清  零"); 	
							Font_with_BackgroundColor();
							Horizontal_FontEnlarge_x1();
							Vertical_FontEnlarge_x1();
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_white);//设定文字的前景色和背景色	
							
							break;
						case 4://测试	
							key_value = 0;								
													
							Graphic_Mode();	
							LcdFillRec(0,0,360,60,1,color_cyan);	
							LcdPrint8bitBmp(gImage_222,5,10,174,40);
							AnJianPressed(412,235,95,40);;//按键按下
							Text_Mode();								
							External_CGROM();
							External_CGROM_GB();
							Active_Window(0,480,0,272);//设定显示区域						
							Horizontal_FontEnlarge_x2();
							Vertical_FontEnlarge_x2();
							Font_with_BackgroundTransparency();//开启穿透功能							
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_cyan);//设定文字的前景色和背景色
							XY_Coordinate(50,15);
							Show_String("测  试");							
							Text_Foreground_Color(color_green);	
							XY_Coordinate(375,220);
							Show_String("测试");							
							Font_with_BackgroundColor();
							Horizontal_FontEnlarge_x1();
							Vertical_FontEnlarge_x1();
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_white);//设定文字的前景色和背景色	
							
							OS_TimeMS=0;	
							pressvalue_avg=0;
							gas_res=0;								
							
							do{
								if(RTC_200MS)
									{
										RTC_200MS=0;
										pressADvalue=Get_Adc_Average(10, 50);;//12位AD											
										data_press[k]=display_press(pressADvalue,280,110);//显示实时压差
										if(data_press[k]>500||data_press[k]==0) 
											{
												test_succesflag=0;
												pressvalue_avg=0;
												test_failule();	
												break;
											}
										k++;
										if(k==5)
										{
											pressvalue_avg=paixu(5,data_press);//计算平均压差，作为压差结果输出显示
											test_succesflag=1;												
										}
									}										
								}while(k!=5);
								k=0;															
								display_pre(pressvalue_avg,280,140);																	
								gas_res=display_holdsize(pressvalue_avg,280,170);//显示通气阻力	
								if(test_succesflag)
								{
									test_succesflag=0;									
									save_data();//测试成功后保存测试数据到flash
									test_cishu++;//测试成功 测试次数增加1																	
									Graphic_Mode();	
									LcdFillRec(0,0,360,60,1,color_cyan);	
									LcdPrint8bitBmp(gImage_222,5,10,174,40);
									Text_Mode();
									Text_Foreground_Color(color_blue);
									Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
									External_CGROM();
									External_CGROM_GB();
									Active_Window(0,480,0,272);//设定显示区域
									XY_Coordinate(30,15);
									Horizontal_FontEnlarge_x2();
									Vertical_FontEnlarge_x2();
									Font_with_BackgroundTransparency();//开启穿透功能							
									Show_String("测试成功"); 									
									Font_with_BackgroundColor();
									Horizontal_FontEnlarge_x1();
									Vertical_FontEnlarge_x1();
									Text_Foreground_Color(color_blue);
									Text_Background_Color(color_white);//设定文字的前景色和背景色	
								}	
								
								if(test_cishu==6) 
								{	
									test_cishu=1;//测试序号1-5
									test_zunum++;//测试组号+1	
								}				

							//保存测试组号和测试次数
							AT24CXX_WriteOneByte(SAVE_ADDR_ZUSHU,test_zunum);
							AT24CXX_WriteOneByte(SAVE_ADDR_CISHU,test_cishu);
							Put_AnJian(412,235,95,40);
							display_number();//更新显示测试次数
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
							Horizontal_FontEnlarge_x2();
							Vertical_FontEnlarge_x2();								
							XY_Coordinate(375,220);
							Show_String("测试");
							Horizontal_FontEnlarge_x1();
							Vertical_FontEnlarge_x1();
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_white);//设定文字的前景色和背景色

							//测试完成后，关闭气源
							j=0;
							open_off_flag=0;
							off_gas();
							
							break;
							
						default:
							break;						
					}				
					key_value=0;//清除按键标志
				}
		}
}
void flash_full(void)
{
	u8 i;
	i=1;
								
	menu_flashmax();//提示数据区满
	while(i)
		{											
			KeyScan();
			if(touch_flag)//触摸屏被按下
			{	
				if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//返回按钮被按下
				{				
					key_value=1;//	菜单按键标志
					speekers(100);//蜂鸣器响100ms
					touch_flag=0;//标记按键已经被处理过了.	
				}	
				else
					{
						key_value=0;
						//speekers(100);//蜂鸣器响100ms		
						touch_flag=0;
					}	
			}
			else
			{
				if(key_value!=0)
					{														
						switch(key_value)
						{
							case 1:  
								key_value = 0;	
								i=0;
								menu1();//加载初始菜单界面
								break;														
							default:
								break;
						}					
						key_value=0;//清除按键标志
					}
			}
			
		}										
	
}
void open_gas(void)
{
	//显示图片
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);	
	LcdPrint8bitBmp(gImage_111,0,17,25,25);
	AnJianPressed(182,235,95,40);//按键按下
	//显示文字
	Text_Mode();	
	Text_Foreground_Color(color_blue);	
	XY_Coordinate(30,22);
	Bold_Font();//字体加粗
	Show_String("医用口罩压差测试仪");
	NoBold_Font();//取消字体加粗	
	
	Horizontal_FontEnlarge_x2();
	Vertical_FontEnlarge_x2();
	Text_Foreground_Color(color_green);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	XY_Coordinate(145,220);
	Show_String("闭气");							
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();
	Text_Foreground_Color(color_blue);
	starttest();//气泵吹气
}
void off_gas(void)
{
	Put_AnJian(182,235,95,40);
	Horizontal_FontEnlarge_x2();
	Vertical_FontEnlarge_x2();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色
	XY_Coordinate(145,220);
	Show_String("通气");							
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();	
	stoptest();//停止吹气
}
void plese_opengas(void)
{
		Graphic_Mode();	
		LcdFillRec(0,0,360,60,1,color_cyan);	
		LcdPrint8bitBmp(gImage_222,5,10,174,40);
		Text_Mode();
		Text_Foreground_Color(color_blue);
		Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
		External_CGROM();
		External_CGROM_GB();
		Active_Window(0,480,0,272);//设定显示区域
		XY_Coordinate(30,15);
		Horizontal_FontEnlarge_x2();
		Vertical_FontEnlarge_x2();
		Font_with_BackgroundTransparency();//开启穿透功能							
		Show_String("请先通气"); 	
		Font_with_BackgroundColor();
		Horizontal_FontEnlarge_x1();
		Vertical_FontEnlarge_x1();
		Text_Foreground_Color(color_blue);
		Text_Background_Color(color_white);//设定文字的前景色和背景色
}
void test_failule(void)
{
		Graphic_Mode();	
		LcdFillRec(0,0,360,60,1,color_cyan);	
		LcdPrint8bitBmp(gImage_222,5,10,174,40);
		Text_Mode();
		Text_Foreground_Color(color_blue);
		Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
		External_CGROM();
		External_CGROM_GB();
		Active_Window(0,480,0,272);//设定显示区域
		XY_Coordinate(30,15);
		Horizontal_FontEnlarge_x2();
		Vertical_FontEnlarge_x2();
		Font_with_BackgroundTransparency();//开启穿透功能							
		Show_String("测试失败"); 	
		Font_with_BackgroundColor();
		Horizontal_FontEnlarge_x1();
		Vertical_FontEnlarge_x1();
		Text_Foreground_Color(color_blue);
		Text_Background_Color(color_white);//设定文字的前景色和背景色
}
//显示序号
void display_number(void)
{
	char mdata[2]={0};
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//设定文字的前景色和背景色
	//组
	sprintf(mdata,"%02d",test_zunum);
	XY_Coordinate(280,80);
	Show_StringNum(mdata,2);
	XY_Coordinate(296,80);
	Show_String("/");
	//次
	sprintf(mdata,"%02d",test_cishu);
	XY_Coordinate(304,80);
	Show_StringNum(mdata,2);

	Text_Background_Color(color_cyan);//设定文字的前景色和背景色
}

//指定位置显示孔径值 运用毛细管方程式计算
//pressAD：压力值 pa 
//x,y:需要显示的位置
//返回的是孔径值 4位小数
float display_holdsize(u16 pressAD,u16 x,u16 y)
{

	float a;
	char mdata[6]={0};

	a=((float)pressAD)/4.9;
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//设定文字的前景色和背景色
	sprintf(mdata,"%-6.2f",a);
	XY_Coordinate(x,y);
	Show_StringNum(mdata,6);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色
	return a;
}
//指定位置显示压力值
//pressAD:AD值
u16 display_press(u16 pressAD,u16 x,u16 y)
{
	u16 a;
	u16 press_now;
	char mdata[4]={0};	
	press_now=press_k*(pressAD-press_zeroAD);
	if(pressAD<press_zeroAD) press_now=0;
	if(press_now>=500||pressAD>=4095) //发现超过量程,立即关闭气门，防止损坏传感器
	{
		if(open_off_flag) off_gas();//关闭气门		
	}
	a=press_now;
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//设定文字的前景色和背景色
	sprintf(mdata,"%04d",a);
	XY_Coordinate(x,y);
	Show_StringNum(mdata,4);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色
	return press_now;
}
//指定位置显示压力值
//pressAD:AD值
void display_pre(u16 press,u16 x,u16 y)
{	
	char mdata[4]={0};		
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//设定文字的前景色和背景色
	sprintf(mdata,"%04d",press);
	XY_Coordinate(x,y);
	Show_StringNum(mdata,4);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色
	
}

void speekers(u16 time)
{
 	GPIO_SetBits(GPIOB,GPIO_Pin_8);						 //PB8 输出高
 	Delay1ms(time);//更新显示界面
 	GPIO_ResetBits(GPIOB,GPIO_Pin_8);					//PB8 输出低 
}
//冒泡法排序 从小到大排列
void bubble_sort(float a[], u8 n)
{	u8 i, j;
	float temp;
	for (j = 0; j < n - 1; j++)
			for (i = 0; i < n - 1 - j; i++)
			 if(a[i] > a[i + 1])
					{temp=a[i]; a[i]=a[i+1]; a[i+1]=temp;}
}
//将测试数据写入结构体 
void save_data(void)
{
	u16 m,n;
	u8 i;

	float a[5]={0};
	float b;
		
		
		//测试序号
		data_once[test_zunum-1].onedata[test_cishu-1].once_xuhao=test_cishu;//测试序号
		//保存压差结果
		
		data_once[test_zunum-1].onedata[test_cishu-1].once_kpa[0]=pressvalue_avg>>8;//压差结果的高8位
		data_once[test_zunum-1].onedata[test_cishu-1].once_kpa[1]=pressvalue_avg;//压差结果的低8位
				
		//通气阻力 小数部分*10000
		m=gas_res;//整数部分		
		n=(gas_res-m)*10000;//小数部分
		data_once[test_zunum-1].onedata[test_cishu-1].once_gas_res[0]=m>>8;//通气阻力整数部分高8位
		data_once[test_zunum-1].onedata[test_cishu-1].once_gas_res[1]=m;//通气阻力整数部分低8位
		data_once[test_zunum-1].onedata[test_cishu-1].once_gas_res[2]=n>>8;//通气阻力小数部分高8位
		data_once[test_zunum-1].onedata[test_cishu-1].once_gas_res[3]=n;//通气阻力小数部分低8位
		
	
		data_once[test_zunum-1].Mark=test_zunum;//组号
		data_once[test_zunum-1].newyears[0]=calendar.w_year>>8;//年的高位
		data_once[test_zunum-1].newyears[1]=calendar.w_year;//年的低位
		data_once[test_zunum-1].newmonth=calendar.w_month;//月
		data_once[test_zunum-1].newday=calendar.w_date;//日			

		//一组中通气阻力的最大值最小值平均值
		for(i=0;i<test_cishu;i++)
		{
			m=data_once[test_zunum-1].onedata[i].once_gas_res[0]<<8;
			m=m+data_once[test_zunum-1].onedata[i].once_gas_res[1];//整数部分
			n=data_once[test_zunum-1].onedata[i].once_gas_res[2]<<8;
			n=n+data_once[test_zunum-1].onedata[i].once_gas_res[3];
			b=m+(float)n/10000;
			a[i]=b;
		}
		bubble_sort(a,test_cishu);//冒泡法排序 从小到大			
			//通气阻力最大值a[4]
			m=a[test_cishu-1];
			n=(a[test_cishu-1]-m)*10000;
			data_once[test_zunum-1].Km_max[0]=m>>8;
			data_once[test_zunum-1].Km_max[1]=m;
			data_once[test_zunum-1].Km_max[2]=n>>8;
			data_once[test_zunum-1].Km_max[3]=n;
			//孔径平均值
			b=0;
			b=(a[0]+a[1]+a[2]+a[3]+a[4])/test_cishu;
			m=b;
			n=(b-m)*10000;
			data_once[test_zunum-1].Km_avg[0]=m>>8;
			data_once[test_zunum-1].Km_avg[1]=m;
			data_once[test_zunum-1].Km_avg[2]=n>>8;
			data_once[test_zunum-1].Km_avg[3]=n;
			//孔径最小值
			m=a[0];
			n=(a[0]-m)*10000;
			data_once[test_zunum-1].Km_min[0]=m>>8;
			data_once[test_zunum-1].Km_min[1]=m;
			data_once[test_zunum-1].Km_min[2]=n>>8;
			data_once[test_zunum-1].Km_min[3]=n;
			/////////////////////////////////////////
			//一组中压差结果的最大值最小值平均值
			for(i=0;i<test_cishu;i++)
			{
				m=data_once[test_zunum-1].onedata[i].once_kpa[0]<<8;
				m=m+data_once[test_zunum-1].onedata[i].once_kpa[1];//整数部分					
				a[i]=m;
			}
			bubble_sort(a,test_cishu);//冒泡法排序 从小到大			
			//压差结果最大值a[4]
			m=a[test_cishu-1];			
			data_once[test_zunum-1].Ka_max[0]=m>>8;
			data_once[test_zunum-1].Ka_max[1]=m;
			
			//压差结果平均值
			b=0;
			b=(a[0]+a[1]+a[2]+a[3]+a[4])/test_cishu;
			m=b;			
			data_once[test_zunum-1].Ka_avg[0]=m>>8;
			data_once[test_zunum-1].Ka_avg[1]=m;
			
			//压差结果最小值
			m=a[0];			
			data_once[test_zunum-1].Ka_min[0]=m>>8;
			data_once[test_zunum-1].Ka_min[1]=m;
			
			//保存到flash		
			SPI_Flash_Write(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,30+7*(test_cishu-1));
				
		
	
}
//提示存储空间已满
void menu_flashmax(void)
{
	Active_Window(0,479,0,271);	
	//显示图片
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdFillRec(0,60,480,200,1,color_white);	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	//DisplayButtonUp(365,215,460,255);
	Put_AnJian(412,235,95,40);
	
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//设定显示区域
	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();	
	XY_Coordinate(20,15);	
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("数据区满");	
	XY_Coordinate(375,220);
	Show_String("返回");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	Font_with_BackgroundColor();
	
	Active_Window(0,480,60,200);//设定显示区域
	Text_Background_Color(color_white);//设定文字的前景色和背景色	
	XY_Coordinate(100,110);
	Show_String("请按《返回》后,进入《菜单》进行数据清空!");
}


//初始菜单界面
void menu1(void)
{
	Active_Window(0,479,0,271);
	Text_Background_Color(color_white);  
	Memory_Clear();
	//显示图片
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);	
	LcdPrint8bitBmp(gImage_111,0,17,25,25);
	
	//显示按钮
// 	DisplayButtonUp(20,215,115,255);
// 	DisplayButtonUp(135,215,230,255);
// 	DisplayButtonUp(250,215,345,255);
// 	DisplayButtonUp(365,215,460,255);
	Put_AnJian(67,235,95,40);
	Put_AnJian(182,235,95,40);
	Put_AnJian(297,235,95,40);
	Put_AnJian(412,235,95,40);
	//显示文字
	Text_Mode();
	
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//设定显示区域
	XY_Coordinate(30,22);
	Bold_Font();//字体加粗
	Show_String("医用口罩压差测试仪");
	NoBold_Font();//取消字体加粗	
  
	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	//Text_Background_Color(color_white);
	XY_Coordinate(30,220);
	Show_String("菜单");
	XY_Coordinate(145,220);
	Show_String("通气");
	XY_Coordinate(260,220);
	Show_String("清零");
	XY_Coordinate(375,220);
	Show_String("测试");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	//显示测试数据
	Active_Window(0,480,60,200);//设定显示区域
	
	Text_Background_Color(color_white);//设定文字的前景色和背景色	
	XY_Coordinate(100,80);
	Show_String("组号/序号:");
	XY_Coordinate(100,110);
	Show_String("实时压差(Pa):");
	XY_Coordinate(100,140);
	Show_String("压差结果(Pa):");
	XY_Coordinate(100,170);
	Show_String("通气阻力(Pa/cm2):");
	
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定时钟文字的前景色和背景色

	display_press(0,280,140);//显示压差结果															
	display_holdsize(0,280,170);//显示通气阻力
	display_number();
	if(open_off_flag)	open_gas();
	else off_gas();
}
//设置界面
/*
void menu_setting(void)
{
	u8 flag=1,xishu_flag=0;
	u8 temp;

	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(0xFF);	
	Memory_Clear();	
	Delay1ms(100);//更新显示界面
		//显示图片
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	LcdPrint8bitBmp(gImage_333,100,70,45,60);
	 //将坐标起始坐标为（0，0），区域大小为100*100的区块数据，搬移到指定位置（240，170）显示
  BTE_Source_Destination(100,217,70,70);
  BTE_Size_setting(45,60);  
	BTE_ROP_Code(0xc2);// move BTE in positive  direction with ROP
	BTE_enable();
  Chk_Busy_BTE();   

  BTE_Source_Destination(100,334,70,70);
  BTE_Size_setting(45,60);  
	BTE_ROP_Code(0xc2);// move BTE in positive  direction with ROP
	BTE_enable();
  Chk_Busy_BTE(); 
  
	//显示按钮
	DisplayButtonUp(20,215,115,255);
	DisplayButtonUp(365,215,460,255);
	
		//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//设定显示区域
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("试剂选择"); 	
	Font_with_BackgroundColor();	
	XY_Coordinate(30,220);
	Show_String("确认");
	XY_Coordinate(375,220);
	Show_String("返回");

	//显示测试数据
	Active_Window(0,480,60,200);//设定显示区域
	Text_Background_Color(color_white);//设定文字的前景色和背景色	
	XY_Coordinate(90,140);
	Show_String("甲醇");
	XY_Coordinate(207,140);
	Show_String("乙醇");
	XY_Coordinate(304,140);
	Show_String("异丙醇");

	Text_Foreground_Color(color_red);
	Text_Background_Color(color_cyan);	
	Active_Window(0,480,0,60);//设定显示区域	
	//显示保存的系数值	
	temp=AT24CXX_ReadOneByte(SAVE_ADDR_XISHU);	
	if(temp==227)//甲醇
	{
	XY_Coordinate(380,15);						
	Show_String("甲醇"); 
	}	
	else if(temp==228)//乙醇
	{
	XY_Coordinate(380,15);
	Show_String("乙醇");
	}	
	else//异丙醇
	{
	XY_Coordinate(380,15);					
	Show_String("异丙醇");
	}	
	while(1)
	{		
		if(flag==0) return;//退出大循环
		KeyScan();
		if(touch_flag)//触摸屏被按下
		{	
			if((LCD_X>20)&&(LCD_X<115)&&(LCD_Y>215)&&(LCD_Y<255))//确认按钮被按下
				{						
					key_value=1;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>340)&&(LCD_X<435)&&(LCD_Y>215)&&(LCD_Y<255))//返回按钮被按下
				{						
					key_value=2;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
			else if((LCD_X>80)&&(LCD_X<145)&&(LCD_Y>70)&&(LCD_Y<170))//甲醇
				{						
					key_value=3;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>197)&&(LCD_X<262)&&(LCD_Y>70)&&(LCD_Y<170))//乙醇
				{							
					key_value=4;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}	
			else if((LCD_X>294)&&(LCD_X<359)&&(LCD_Y>70)&&(LCD_Y<170))//异丙醇
				{					
					key_value=5;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}	
			else
				{
					key_value=0;
					//speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{
					
					
					switch(key_value)
					{
						case 1:  //确认  进行系数保存
							key_value = 0;	
							if(xishu_flag==3)//甲醇系数 0.0227
							{
								temp=0.0227*10000;								
								AT24CXX_WriteOneByte(SAVE_ADDR_XISHU,temp); 
							}							
							else if(xishu_flag==5)//异丙醇系数 0.0217
							{
								temp=0.0217*10000;
								AT24CXX_WriteOneByte(SAVE_ADDR_XISHU,temp); 
							}
							else//默认的为乙醇
							{
								temp=0.0228*10000;
								AT24CXX_WriteOneByte(SAVE_ADDR_XISHU,temp); 
							}
							if(temp==228)	midu=0.789;//乙醇的密度
							else if(temp==227)	midu=0.7918;//甲醇的密度
							else if(temp==217)	midu=0.7855;//异丙醇的密度
							xishu=(float)temp/10000;//得到保存的系数值
							XY_Coordinate(380,15);
							Show_String("      ");
							XY_Coordinate(380,15);						
							Show_String("保存");	
							break;						
						case 2://返回
							key_value = 0;
							flag=0;
						  Horizontal_FontEnlarge_x1();
							Vertical_FontEnlarge_x1();						
							Graphic_Mode();
							break;
						case 3://甲醇							
							XY_Coordinate(380,15);
							Show_String("      ");
							XY_Coordinate(380,15);						
							Show_String("甲醇"); 
							xishu_flag=3;
							key_value = 0;																			
							break;
						case 4://											
							XY_Coordinate(380,15);
							Show_String("      ");
							XY_Coordinate(380,15);
							Show_String("乙醇");
							xishu_flag=4;
							key_value = 0;
							break;
						case 5://						
							XY_Coordinate(380,15);
							Show_String("      ");
							XY_Coordinate(380,15);					
							Show_String("异丙醇");	
							xishu_flag=5;
							key_value = 0;
							break;
						default:
							break;
					}					
					key_value=0;//清除按键标志
				}
		}
	}
}
*/
//显示虚拟键盘
void JianPan_Displayfu(void)
{
	Graphic_Mode();	
	LcdFillRec(75,60,410,260,1,color_white);//画一个白色矩形方框
	DisplayButtonUp(75,60,410,260);//画一个键盘的大外框	
	DisplayButton(80,65,405,90);
	DisplayButton(80,95,360,120);//画显示区域
	//画键盘
	 DisplayButton(99,132,139,162); //1	 
   DisplayButton(161,132,201,162); //2	
	 DisplayButton(223,132,263,162); //3	
	 DisplayButton(285,132,325,162); //4	
	
	
	 DisplayButton(99,174,139,204); //5	
   DisplayButton(161,174,201,204); //6	 
	 DisplayButton(223,174,263,204); //7	
	 DisplayButton(285,174,325,204); //8	
	 
	
	 DisplayButton(99,216,139,246); //9	
   DisplayButton(161,216,201,246); //0	
	 DisplayButton(223,216,263,246); //清零	
	 DisplayButton(285,216,325,246); //删除	 
	 DisplayButton(347,125,405,185); //确认		
	 DisplayButton(347,195,405,255); //返回
	
		
	Text_Mode();						//字符模式(LCD默认为图形模式)
	Text_Foreground_Color(color_red);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,60,272);//设定显示区域

	XY_Coordinate(360,147);//确定键
	Show_String("确定");
	XY_Coordinate(360,217);	//返回键
	Show_String("返回");

	NoBackgroundColor();//文字无背景色
	XY_Coordinate(113,139);	//1
	Show_String("1");

	XY_Coordinate(175,139);	//2
	Show_String("2");

	XY_Coordinate(237,139);	//3
	Show_String("3");

	XY_Coordinate(299,139);	//4
	Show_String("4");

	XY_Coordinate(113,181);	//5
	Show_String("5");

	XY_Coordinate(175,181);	//6
	Show_String("6");

	XY_Coordinate(237,181);	//7
	Show_String("7");

	XY_Coordinate(299,181);	//8
	Show_String("8");

	XY_Coordinate(113,223);	//9
	Show_String("9");

	XY_Coordinate(175,223);	//0
	Show_String("0");
	
	XY_Coordinate(227,223);	//清零
	Show_String("清零");

	XY_Coordinate(289,223);	//删除
	Show_String("删除");	
	BackgroundColor();
	
}

//键盘扫描
void ScanKeyBoard(void)
{
	KeyScan();
	if(touch_flag)//有按键按下并松手
	{
		if((LCD_X>99)&&(LCD_X<139)&&(LCD_Y>132)&&(LCD_Y<162))//  1
				{						
					key_value=1;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;
				}

		else if((LCD_X>161)&&(LCD_X<201)&&(LCD_Y>132)&&(LCD_Y<162))// 2
				{						
					key_value=2;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}

		else if((LCD_X>223)&&(LCD_X<263)&&(LCD_Y>132)&&(LCD_Y<162))// 3
				{						
					key_value=3;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
		else if((LCD_X>285)&&(LCD_X<325)&&(LCD_Y>132)&&(LCD_Y<162))// 4
				{						
					key_value=4;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
		else if((LCD_X>99)&&(LCD_X<139)&&(LCD_Y>174)&&(LCD_Y<204))// 5
				{						
					key_value=5;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
		else if((LCD_X>161)&&(LCD_X<201)&&(LCD_Y>174)&&(LCD_Y<204))//6
				{						
					key_value=6;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
		else if((LCD_X>223)&&(LCD_X<263)&&(LCD_Y>174)&&(LCD_Y<204))// 7
				{						
					key_value=7;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}	
		else if((LCD_X>285)&&(LCD_X<325)&&(LCD_Y>174)&&(LCD_Y<204))// 8
				{						
					key_value=8;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
		else if((LCD_X>99)&&(LCD_X<139)&&(LCD_Y>216)&&(LCD_Y<246))// 9
				{						
					key_value=9;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}

		else if((LCD_X>161)&&(LCD_X<201)&&(LCD_Y>216)&&(LCD_Y<246))// 0
				{						
					key_value=10;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
		else if((LCD_X>223)&&(LCD_X<263)&&(LCD_Y>216)&&(LCD_Y<246))// 清零
				{						
					key_value=11;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
		else if((LCD_X>285)&&(LCD_X<325)&&(LCD_Y>216)&&(LCD_Y<246))// 删除
				{						
					key_value=12;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
		else if((LCD_X>352)&&(LCD_X<400)&&(LCD_Y>125)&&(LCD_Y<185))// 确认
				{						
					key_value=13;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}	
		else if((LCD_X>352)&&(LCD_X<400)&&(LCD_Y>195)&&(LCD_Y<255))// 返回
				{						
					key_value=14;
					speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}			
		else
				{
					key_value=0;
					//speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}
	}

}



//日期设置界面
void menu_data(void)
{
	u8 i=1,j=0;
	u16 setyears;//设定的年
	u8 setmonth;//设定月
	u8 setday;	//设定年
	char setdata[8]={0};

	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(color_cyan);	
	Memory_Clear();	
	Delay1ms(100);//更新显示界面
		//显示图片
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	JianPan_Displayfu();//显示虚拟键盘
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//设定显示区域
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("日期设置"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("日期设置格式例如：20180907");
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) return;//退出大循环
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				setdata[j]=key_value-10+0x30;//将键值转换成ASCII码暂存
			else 
				setdata[j]=key_value+0x30;//将键值转换成ASCII码暂存			
			if(setdata[0]==0x30)//第一位为0
			{
				XY_Coordinate(90,100);//指定显示位置
				Show_String("Data setting Error!");				
				continue;
			}
			j++;
			if(j>8) 
			{
			XY_Coordinate(240,100);//指定显示位置
			Show_String("out of rang!");	
			Delay1ms(1000);	//延时1S	
			XY_Coordinate(240,100);//指定显示位置
			Show_String("            ");	//清除显示
			key_value=0;
			j=8;		
			continue;
			}
		}
		switch(key_value)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:					
			case 9:
			case 10://0
					key_value=0;					
					XY_Coordinate(90+(j-1)*8,100);//指定显示位置
					Show_StringNum(&setdata[j-1],1);					
					break;
			case 11://清零					
					for(j=0;j<8;j++) setdata[j]=0;
					j=0;
					XY_Coordinate(90,100);//指定显示位置
					Show_String("         ");
					key_value=0;
					break;
			case 12://删除
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 3:
										XY_Coordinate(90+16,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 4:
										XY_Coordinate(90+24,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 5:
										XY_Coordinate(90+32,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 6:
										XY_Coordinate(90+40,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 7:
										XY_Coordinate(90+48,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 8:
										XY_Coordinate(90+56,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							default: break;	
						}
					
					break;
			case 13://确认
					for(j=0;j<8;j++) setdata[j]-=0x30;
					setyears=setdata[0]*1000+setdata[1]*100+setdata[2]*10+setdata[3];
					setmonth=setdata[4]*10+setdata[5];
					if((setmonth>12)||(setmonth==0)) setmonth=1;//超出范围 置1
					setday=setdata[6]*10+setdata[7];
					if((setday>31)||(setday==0)) setday=1;//超出范围 置1
					RTC_Set(setyears,setmonth,setday,calendar.hour,calendar.min,calendar.sec);
// 					XY_Coordinate(360,147);//确定键
// 					Show_String("保存");
// 					Delay1ms(100);	//延时1S	
// 					XY_Coordinate(360,147);//确定键
// 					Show_String("确定");
					i=0;//退出大循环
					key_value=0;
					break;
			case 14://返回
					i=0;//退出大循环
					Graphic_Mode();	//恢复图片模式
					key_value=0;
					break;
			default: break;			
		}
	}
}
//时间设置界面
void menu_time(void)
{
	u8 i=1,j=0;
	u8 sethour;//设定的时
	u8 setmin;//设定分
	u8 setsec;	//设定秒
	char settime[6]={0};

	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(color_cyan);	
	Memory_Clear();	
	Delay1ms(100);//更新显示界面
		//显示图片
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	JianPan_Displayfu();//显示虚拟键盘
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//设定显示区域
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("时间设置"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("时间设置格式例如：15:08:07");
	XY_Coordinate(90,100);//指定显示位置显示时间格式
	Show_String("  :  :  ");	
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) return;//退出大循环
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				settime[j]=key_value-10+0x30;//将键值转换成ASCII码暂存
			else 
				settime[j]=key_value+0x30;//将键值转换成ASCII码暂存			
			
			j++;
			if(j>6) 
			{
			XY_Coordinate(240,100);//指定显示位置
			Show_String("out of rang!");	
			Delay1ms(1000);	//延时1S	
			XY_Coordinate(240,100);//指定显示位置
			Show_String("            ");	//清除显示
			key_value=0;
			j=6;		
			continue;
			}
		}
		switch(key_value)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:					
			case 9:
			case 10://0
					key_value=0;
					if((j>2)&&(j<5))
						XY_Coordinate(98+(j-1)*8,100);//指定显示位置
					else if(j>4)
						XY_Coordinate(106+(j-1)*8,100);//指定显示位置
					else	
						XY_Coordinate(90+(j-1)*8,100);//指定显示位置
					Show_StringNum(&settime[j-1],1);					
					break;
			case 11://清零					
					for(j=0;j<6;j++) settime[j]=0;
					j=0;					
					XY_Coordinate(90,100);//指定显示位置显示时间格式
					Show_String("  :  :  ");
					key_value=0;
					break;
			case 12://删除
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 3:
										XY_Coordinate(98+16,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 4:
										XY_Coordinate(98+24,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 5:
										XY_Coordinate(138,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 6:
										XY_Coordinate(146,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
						
							default: break;	
						}
					
					break;
			case 13://确认
					for(j=0;j<6;j++) settime[j]-=0x30;
					sethour=settime[0]*10+settime[1];
					if(sethour>24) sethour=1;//超出范围 置1
					setmin=settime[2]*10+settime[3];
					if(setmin>60) setmin=1;//超出范围 置1
					setsec=settime[4]*10+settime[5];
					if(setsec>60) setsec=1;//超出范围 置1
					RTC_Set(calendar.w_year,calendar.w_month,calendar.w_date,sethour,setmin,setsec);
					XY_Coordinate(360,147);//确定键
					Show_String("保存");
					Delay1ms(1000);	//延时1S	
					XY_Coordinate(360,147);//确定键
					Show_String("确定");
					key_value=0;
					break;
			case 14://返回
					i=0;//退出大循环
					Graphic_Mode();	//恢复图片模式
					key_value=0;
					break;
			default: break;			
		}
	}
}
//密码设置界面
void menu_password(void)
{
	u8 i=1,j=0;
	u32 password_data=0;
	char settime[6]={0};

	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(color_cyan);	
	Memory_Clear();	
	Delay1ms(100);//更新显示界面
		//显示图片
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	JianPan_Displayfu();//显示虚拟键盘
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//设定显示区域
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("  AD校准"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("请输入密码:");
	//XY_Coordinate(90,100);//指定显示位置显示时间格式
	//Show_String("  :  :  ");	
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) return;//退出大循环
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				settime[j]=key_value-10+0x30;//将键值转换成ASCII码暂存
			else 
				settime[j]=key_value+0x30;//将键值转换成ASCII码暂存			
			
			j++;
			if(j>6) 
			{
			XY_Coordinate(240,100);//指定显示位置
			Show_String("out of rang!");	
			Delay1ms(1000);	//延时1S	
			XY_Coordinate(240,100);//指定显示位置
			Show_String("            ");	//清除显示
			key_value=0;
			j=6;		
			continue;
			}
		}
		switch(key_value)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:					
			case 9:
			case 10://0
					key_value=0;
					//if((j>2)&&(j<5))
					//	XY_Coordinate(98+(j-1)*8,100);//指定显示位置
					//else if(j>4)
					//	XY_Coordinate(106+(j-1)*8,100);//指定显示位置
					//else	
						XY_Coordinate(90+(j-1)*8,100);//指定显示位置
					Show_StringNum(&settime[j-1],1);					
					break;
			case 11://清零					
					for(j=0;j<6;j++) settime[j]=0;
					j=0;
					XY_Coordinate(90,100);//指定显示位置
					Show_String("         ");
					key_value=0;
					break;
			case 12://删除
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 3:
										XY_Coordinate(90+16,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 4:
										XY_Coordinate(90+24,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 5:
										XY_Coordinate(90+32,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 6:
										XY_Coordinate(90+40,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
						
							default: break;	
						}
					
					break;
			case 13://确认
					for(j=0;j<6;j++) settime[j]-=0x30;
					password_data=settime[0]*100000+settime[1]*10000+settime[2]*1000+settime[3]*100+settime[4]*10+settime[5];
					if(password_data==password) //密码正确 退出大循环
					{
					password_flag=1;
					i=0;//退出大循环	
					Graphic_Mode();	//恢复图片模式		
					key_value=0;
					}
					else//密码错误 清除显示区域
					{
						for(j=0;j<6;j++) settime[j]=0;
						j=0;
						XY_Coordinate(90,100);//指定显示位置
						Show_String("         ");
						key_value=0;
						XY_Coordinate(240,100);//指定显示位置
						Show_String("password error!");	
						Delay1ms(1000);	//延时1S	
						XY_Coordinate(240,100);//指定显示位置
						Show_String("               ");	//清除显示
					}
					break;
			case 14://返回
					i=0;//退出大循环
					Graphic_Mode();	//恢复图片模式
					key_value=0;
					break;
			default: break;			
		}
	}
}
//菜单选择界面
void menu_select(void)
{
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(0xFF);	
	Memory_Clear();	
	Delay1ms(100);//更新显示界面
		//显示图片
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
  
	//显示按钮   |确认 返回|
	//DisplayButtonUp(20,215,115,255);//确认
	//DisplayButtonUp(365,215,460,255);//返回
	Put_AnJian(412,235,95,40);
	DisplayButtonUp(100,80,190,100);
	DisplayButtonUp(290,80,380,100);
	
	DisplayButtonUp(100,120,190,140);
	DisplayButtonUp(290,120,380,140);
	
	DisplayButtonUp(100,160,190,180);
	DisplayButtonUp(290,160,380,180);
	
		//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//设定显示区域
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("菜单选择"); 	
	Font_with_BackgroundColor();
	XY_Coordinate(375,220);
	Show_String("返回");

	//显示测试数据
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();
	Bold_Font();//字体加粗
	Active_Window(0,480,60,200);//设定显示区域
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//设定文字的前景色和背景色	
	XY_Coordinate(110,82);
	Show_String("日期设置");
	XY_Coordinate(110,122);
	Show_String("时间设置");
	XY_Coordinate(110,162);
	Show_String("屏幕校准");
	
	XY_Coordinate(310,82);
	Show_String("AD校准");
	XY_Coordinate(310,122);
	Show_String("打  印");
	XY_Coordinate(300,162);
	Show_String("数据查询");
	NoBold_Font();//取消字体加粗
	Graphic_Mode();
}

//菜单界面
void menu_menu(void)
{
	u8 flag=1;	
	menu_select();
	while(1)
	{		
		if(flag==0) return;//退出大循环		
		KeyScan();
		if(touch_flag)//触摸屏被按下
		{	
			if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//返回按钮被按下
				{						
					key_value=1;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;
				}
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>80)&&(LCD_Y<100))//日期设置
				{						
					key_value=2;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>120)&&(LCD_Y<140))//时间设置
				{						
					key_value=3;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>160)&&(LCD_Y<180))//屏幕校准
				{						
					key_value=4;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}	
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>80)&&(LCD_Y<100))//AD校准
				{						
					key_value=5;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}	
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>120)&&(LCD_Y<140))//打印
				{						
					key_value=6;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>160)&&(LCD_Y<180))//数据查询
				{						
					key_value=7;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else
				{
					key_value=0;
					//speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //返回
							key_value = 0;
							flag=0;
							break;						
						case 2:////日期设置
							key_value = 0;
							menu_data();
						  menu_select();
							break;
						case 3://	//时间设置								
							key_value = 0;
							menu_time();
							menu_select();
							break;
						case 4:////屏幕校准	
							key_value = 0;
							XY_Calibration_Function();				//触摸屏校准							
							menu_select();
							break;
						case 5://	//AD校准	
							key_value = 0;
							menu_password();
							if(password_flag==1)
							{
								password_flag=0;
								AD_Calibration_Function();//AD校准界面
							}
							menu_select();							
							break;
						case 6://	//打印							
							menu_printselect();
							menu_select();
							key_value = 0;
							break;
						case 7://	//数据查询					
							data_Query();
							menu_select();
							key_value = 0;
							break;
						default:
							break;
					}				
					
				}
		}
	}
}
//数据查询界面
void menu_dataQuery(void)
{
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(0xFF);	
	Memory_Clear();	
	Delay1ms(100);//更新显示界面
		//显示图片
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
  
	//显示按钮   |确认 返回|
	//DisplayButtonUp(20,215,115,255);//确认
	//DisplayButtonUp(365,215,460,255);//返回
	Put_AnJian(412,235,95,40);
	DisplayButtonUp(180,80,328,100);	
	DisplayButtonUp(180,120,328,140);	
	DisplayButtonUp(180,160,328,180);
	
		//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//设定显示区域
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("数据查询"); 	
	Font_with_BackgroundColor();
	XY_Coordinate(375,220);
	Show_String("返回");

	//显示测试数据
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();
	Bold_Font();//字体加粗
	Active_Window(0,480,60,200);//设定显示区域
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//设定文字的前景色和背景色	 
	XY_Coordinate(190,82);
	Show_String("  查询测试数据  ");
	XY_Coordinate(190,122);
	Show_String("清除本次测试数据");
	XY_Coordinate(190,162);
	Show_String("清除所有测试数据");	
	
	NoBold_Font();//取消字体加粗
	Graphic_Mode();
}
//显示组号
void display_zunum(void)
{
	char Data[10];
	if(data_once[checkTestnum].Mark>11) 
		sprintf(Data,"%03d",0);
	else	
		sprintf(Data,"%03d",data_once[checkTestnum].Mark);
	XY_Coordinate(115,3);
	Show_StringNum(Data,3);
}
//显示日期
void display_riqi(void)
{
	char Data[10];
	u16 a;
	a=(data_once[checkTestnum].newyears[0])<<8;
	a=a+data_once[checkTestnum].newyears[1];
	if(a==0xffff) 
		sprintf(Data,"%04d-%02d-%02d",0,0,0);
	else	
		sprintf(Data,"%04d-%02d-%02d",a,data_once[checkTestnum].newmonth,data_once[checkTestnum].newday);
	XY_Coordinate(395,3);
	Show_StringNum(Data,10);
}
// //显示试剂
// void display_shiji(void)
// {
// 	char Data[10];
// 	if(data_once[checkTestnum].shiji==0xff)
// 		sprintf(Data,"%6.4f",(float)0);
// 	else	
// 		sprintf(Data,"%6.4f",((float)data_once[checkTestnum].shiji/10000));	
// 	XY_Coordinate(400,5);
// 	Show_StringNum(Data,6);
// }
//显示通气阻力
// x,y:指定显示位置 
//xuhao：需要显示的数据序号 1-5
//flag:类别 1表示最大孔径 0表示平均孔径
void display_holesize(u16 x,u16 y,u8 xuhao)
{
	char Data[10];
	float a;
	if((data_once[checkTestnum].onedata[xuhao].once_gas_res[0]!=0xff)&&(data_once[checkTestnum].onedata[xuhao].once_gas_res[1]!=0xff)&&(data_once[checkTestnum].onedata[xuhao].once_gas_res[2]!=0xff)&&(data_once[checkTestnum].onedata[xuhao].once_gas_res[3]!=0xff))//最大孔径	
		a=data_once[checkTestnum].onedata[xuhao].once_gas_res[0]*256+data_once[checkTestnum].onedata[xuhao].once_gas_res[1]+(float)(data_once[checkTestnum].onedata[xuhao].once_gas_res[2]*256+data_once[checkTestnum].onedata[xuhao].once_gas_res[3])/10000;	
	
	else
		return;
	sprintf(Data,"%6.2f    ",a);
	XY_Coordinate(x,y);
	Show_StringNum(Data,6);
}
//显示一组中的最大、平均、最小孔径
// x,y:指定显示位置 
//xuhao：需要显示的数据序号 1-5
//flag:类别 1表示一组通气阻力中的最大
//          2表示一组通气阻力中的平均 
//          3表示一组通气阻力中的最小
//          4表示一组压差结果中的最大
//          5表示一组压差结果中的平均
//          6表示一组压差结果中的最小
void display_holeinnum(u16 x,u16 y,u8 flag)
{
	char Data[10];
	float a;
	u16 b;
	if(flag==1)
		a=data_once[checkTestnum].Km_max[0]*256+data_once[checkTestnum].Km_max[1]+(float)(data_once[checkTestnum].Km_max[2]*256+data_once[checkTestnum].Km_max[3])/10000;
	else if(flag==2)
		a=data_once[checkTestnum].Km_avg[0]*256+data_once[checkTestnum].Km_avg[1]+(float)(data_once[checkTestnum].Km_avg[2]*256+data_once[checkTestnum].Km_avg[3])/10000;
	else if(flag==3)
		a=data_once[checkTestnum].Km_min[0]*256+data_once[checkTestnum].Km_min[1]+(float)(data_once[checkTestnum].Km_min[2]*256+data_once[checkTestnum].Km_min[3])/10000;
	else if(flag==4)//一组压差结果中的最大值
		b=data_once[checkTestnum].Ka_max[0]*256+data_once[checkTestnum].Ka_max[1];
	else if(flag==5)
		b=data_once[checkTestnum].Ka_avg[0]*256+data_once[checkTestnum].Ka_avg[1];
	else if(flag==6)
		b=data_once[checkTestnum].Ka_min[0]*256+data_once[checkTestnum].Ka_min[1];
	if(flag==1||flag==2||flag==3)
	{
		sprintf(Data,"%6.2f",a);
		XY_Coordinate(x,y);
		Show_StringNum(Data,8);
	}
	else
	{
		sprintf(Data,"%04d",b);
		XY_Coordinate(x,y);
		Show_StringNum(Data,4);
	}
}
//显示平均压强
// x,y:指定显示位置 
//xuhao：需要显示的数据序号 1-5
//flag:类别 1表示最大压强 0表示平均压强
void display_pa(u16 x,u16 y,u8 xuhao)
{
	char Data[10];
	u16 Pa;
	
	if((data_once[checkTestnum].onedata[xuhao].once_kpa[0]==0xff&&data_once[checkTestnum].onedata[xuhao].once_kpa[1]==0xff))
		return;		
	Pa=data_once[checkTestnum].onedata[xuhao].once_kpa[0]*256+data_once[checkTestnum].onedata[xuhao].once_kpa[1];
				
	sprintf(Data,"%04d",Pa);
	XY_Coordinate(x,y);
	Show_StringNum(Data,4);
}

//显示所有测试数据到界面
void display_testdata(void)
{
	//Graphic_Mode();		
	//LcdPrint8bitBmp(biaoge,0,0,479,195);
	show_excel();
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();	
	display_zunum();		
	display_riqi();	
	if(((checkTestnum>0||checkTestnum==0))&&(checkTestnum<10)&&(checkTestnum<test_zunum-1))
	{		
	//显示第1号测试数据		
	display_pa(181,41,0);//第一组的压差
	display_holesize(350,41,0);//第一组的通气阻力
		
	//显示第2号测试数据	
	display_pa(181,60,1);
	display_holesize(350,60,1);
		
	//显示第3号测试数据
	display_pa(181,79,2);
	display_holesize(350,79,2);
	
	//显示第4号测试数据
	display_pa(181,98,3);
	display_holesize(350,98,3);	
	
	//显示第5号测试数据		
	display_pa(181,117,4);
	display_holesize(350,117,4);
		
	//一组中的最大孔径中的最大值
	display_holeinnum(181,136,4);//平均压差
	display_holeinnum(181,155,5);
	display_holeinnum(181,174,6);
	
	display_holeinnum(350,136,1);//通气阻力
	display_holeinnum(350,155,2);
	display_holeinnum(350,174,3);	
	}
	if(((checkTestnum>0||checkTestnum==0))&&(checkTestnum<10)&&(checkTestnum==test_zunum-1)&&(test_cishu!=1))
	{	
		if(test_cishu==2)//有一组数据
		{			
			display_holesize(350,41,0);//第一组的通气阻力	
			display_pa(181,41,0);//第一组的压差
			//一组中的最大孔径中的最大值
			display_holeinnum(181,136,4);//平均压差
			display_holeinnum(181,155,5);
			display_holeinnum(181,174,6);
			display_holeinnum(350,136,1);//通气阻力
			display_holeinnum(350,155,2);
			display_holeinnum(350,174,3);	
		}	
		else if(test_cishu==3)//有2组数据
		{		
			//显示第1号测试数据
			display_holesize(350,41,0);//第一组的通气阻力	
			display_pa(181,41,0);//第一组的压差
			
			//显示第2号测试数据
			display_holesize(350,60,1);
			display_pa(181,60,1);
			//一组中的最大孔径中的最大值
			
			display_holeinnum(181,136,4);//平均压差
			display_holeinnum(181,155,5);
			display_holeinnum(181,174,6);
			display_holeinnum(350,136,1);//通气阻力
			display_holeinnum(350,155,2);
			display_holeinnum(350,174,3);	
		}
		else if(test_cishu==4)//有3组数据
		{			
		
			//显示第1号测试数据
			display_holesize(350,41,0);//第一组的通气阻力	
			display_pa(181,41,0);//第一组的压差
			
			//显示第2号测试数据
			display_holesize(350,60,1);
			display_pa(181,60,1);
			
			//显示第3号测试数据
			display_holesize(350,79,2);
			display_pa(181,79,2);
			//一组中的最大孔径中的最大值
			display_holeinnum(181,136,4);//平均压差
			display_holeinnum(181,155,5);
			display_holeinnum(181,174,6);
			display_holeinnum(350,136,1);//通气阻力
			display_holeinnum(350,155,2);
			display_holeinnum(350,174,3);	
		}
		else if(test_cishu==5)//有4组数据
		{			
			//显示第1号测试数据
			display_holesize(350,41,0);//第一组的通气阻力	
			display_pa(181,41,0);//第一组的压差
			
			//显示第2号测试数据
			display_holesize(350,60,1);
			display_pa(181,60,1);
			
			//显示第3号测试数据
			display_holesize(350,79,2);
			display_pa(181,79,2);
			//显示第4号测试数据
			display_holesize(350,98,3);	
			display_pa(181,98,3);
			
			//一组中的最大孔径中的最大值
			display_holeinnum(181,136,4);//平均压差
			display_holeinnum(181,155,5);
			display_holeinnum(181,174,6);
			display_holeinnum(350,136,1);//通气阻力
			display_holeinnum(350,155,2);
			display_holeinnum(350,174,3);		
		}	
	}	
		
}


//查询所有测试数据界面
void check_datamenu(void)
{		
	Active_Window(0,479,0,271);
	Text_Background_Color(color_white);  
	Memory_Clear();
	//显示图片
	Graphic_Mode();	
	LcdFillRec(0,200,480,272,1,color_cyan);
	//显示按钮
// 	DisplayButtonUp(20,215,115,255);
// 	DisplayButtonUp(135,215,230,255);
// 	DisplayButtonUp(250,215,345,255);
// 	DisplayButtonUp(365,215,460,255);
	Put_AnJian(67,235,95,40);
	Put_AnJian(182,235,95,40);
	Put_AnJian(297,235,95,40);
	Put_AnJian(412,235,95,40);
	show_excel();

	
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();	
	XY_Coordinate(30,220);
	Show_String("刷新");
	XY_Coordinate(145,220);
	Show_String("<---");
	XY_Coordinate(260,220);
	Show_String("--->");
	XY_Coordinate(375,220);
	Show_String("返回");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();		
	
}
void check_data(void)
{
	u8 j;
	static u8 flag;
	
	for(j=0;j<test_zunum;j++)
		SPI_Flash_Read(&data_once[j].Mark,j*256,58);	
	check_datamenu();//查询所有测试数据界面	
	display_testdata();
	flag=1;
	while(flag)
	{
		KeyScan();
		if(touch_flag)//触摸屏被按下
		{	
			if((LCD_X>30)&&(LCD_X<105)&&(LCD_Y>215)&&(LCD_Y<255))//刷新按钮被按下
				{						
					key_value=1;//设置按键标志	
					speekers(100);//蜂鸣器响100ms
					touch_flag=0;//标记按键已经被处理过了.	
				}
			else if((LCD_X>145)&&(LCD_X<220)&&(LCD_Y>215)&&(LCD_Y<255))//上翻按钮被按下
				{				
					key_value=2;//	菜单按键标志
					speekers(100);//蜂鸣器响100ms
					touch_flag=0;//标记按键已经被处理过了.	
				}
			else if((LCD_X>260)&&(LCD_X<335)&&(LCD_Y>215)&&(LCD_Y<255))//下翻按钮被按下
				{						
					key_value=3;//	停止按键标志
					speekers(100);//蜂鸣器响100ms
					touch_flag=0;//标记按键已经被处理过了.
				}	
			else if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//返回按钮被按下
				{					
					key_value=4;//	测试按键标志
					speekers(100);//蜂鸣器响100ms					
					touch_flag=0;//标记按键已经被处理过了.	
				}
			else
				{
					key_value=0;
					//speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //刷新											
							key_value = 0;
							checkTestnum=0;
							display_testdata();
							break;	
						case 2:  //上翻页								
								checkTestnum--;
								if(((checkTestnum>0||checkTestnum==0))&&(checkTestnum<10)&&(checkTestnum<test_zunum)&&data_once[checkTestnum].Mark!=0&&data_once[checkTestnum].Mark!=0xff)
									display_testdata();
// 								else if(data_once[checkTestnum].Mark==0xff)
// 								{
// 									flag=0;
// 									checkTestnum=0;
// 								}
								else
								{
									checkTestnum=0;
									display_testdata();
								}							
							key_value = 0;					
							break;						
						case 3:  //下翻页
							checkTestnum++;
							if(((checkTestnum>0||checkTestnum==0))&&(checkTestnum<10)&&(checkTestnum<test_zunum)&&data_once[checkTestnum].Mark!=0&&data_once[checkTestnum].Mark!=0xff)
								display_testdata();
// 							else if(data_once[checkTestnum].Mark==0xff)
// 							{
// 								flag=0;
// 								checkTestnum=0;
// 							}
							else
							{
								checkTestnum=0;
								display_testdata();
							}
							key_value = 0;						
							break;		
						case 4: //返回
							flag=0;
							checkTestnum=0;
							key_value = 0;						
							break;					
						default:
							break;
					}				
					
				}
		}
	}
}

//数据查询
void data_Query(void)
{
	u8 i,j=0,flag=1;
	u16 m,n;
	float a[5]={0};
	float b;
	menu_dataQuery();//数据查询界面
	while(flag)
	{
		KeyScan();
		if(touch_flag)//触摸屏被按下
		{			
			if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//返回按钮被按下
				{						
					key_value=1;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;
				}			
			else if((LCD_X>180)&&(LCD_X<328)&&(LCD_Y>80)&&(LCD_Y<100))//查询测试数据
				{						
					key_value=2;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>180)&&(LCD_X<328)&&(LCD_Y>120)&&(LCD_Y<140))//清除本次测试数据
				{						
					key_value=3;
					for(i=0;i<5;i++) a[i]=0;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>180)&&(LCD_X<328)&&(LCD_Y>160)&&(LCD_Y<180))//清除所有测试数据
				{						
					key_value=4;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}			
			else
				{
					key_value=0;
					//speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //返回						
							flag=0;					
							key_value = 0;							
							break;	
						case 2:  //查询测试数据
							key_value = 0;
							check_data();//查询测试数据界面
							menu_dataQuery();//数据查询界面
							key_value = 0;					
							break;						
						case 3:  //清除本次测试数据							
							key_value = 0;							
							if((test_cishu==1)&&test_zunum>1)//测试完了一组数据
							{
								test_cishu=6;
								test_zunum=test_zunum-1;
							}	
							SPI_Flash_Read(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,58);
							if((test_cishu>1)&&(test_cishu<7))//本组的测试还没有测试完
							{	
								if(test_cishu==2)
								{		
									data_once[test_zunum-1].Km_max[0]=0xff;
									data_once[test_zunum-1].Km_max[1]=0xff;
									data_once[test_zunum-1].Km_max[2]=0xff;
									data_once[test_zunum-1].Km_max[3]=0xff;
									
									data_once[test_zunum-1].Km_avg[0]=0xff;
									data_once[test_zunum-1].Km_avg[1]=0xff;
									data_once[test_zunum-1].Km_avg[2]=0xff;
									data_once[test_zunum-1].Km_avg[3]=0xff;
									
									data_once[test_zunum-1].Km_min[0]=0xff;
									data_once[test_zunum-1].Km_min[1]=0xff;
									data_once[test_zunum-1].Km_min[2]=0xff;
									data_once[test_zunum-1].Km_min[2]=0xff;
									
									
									data_once[test_zunum-1].Ka_max[0]=0xff;
									data_once[test_zunum-1].Ka_max[1]=0xff;									
									
									data_once[test_zunum-1].Ka_avg[0]=0xff;
									data_once[test_zunum-1].Ka_avg[1]=0xff;									
									
									data_once[test_zunum-1].Ka_min[0]=0xff;
									data_once[test_zunum-1].Ka_min[1]=0xff;								
									
									data_once[test_zunum-1].onedata[0].once_kpa[0]=0xff;
									data_once[test_zunum-1].onedata[0].once_kpa[1]=0xff;
									
									data_once[test_zunum-1].onedata[0].once_gas_res[0]=0xff;
									data_once[test_zunum-1].onedata[0].once_gas_res[1]=0xff;
									data_once[test_zunum-1].onedata[0].once_gas_res[2]=0xff;
									data_once[test_zunum-1].onedata[0].once_gas_res[3]=0xff;
									SPI_Flash_Write(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,35);
								}
								test_cishu--;
								if(test_cishu==1) 									
										continue;								
								SPI_Flash_Read(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,30+7*(test_cishu-2));
// 								if(test_cishu==1)
// 								{	
								data_once[test_zunum-1].Mark=test_zunum;//序号
								data_once[test_zunum-1].newyears[0]=calendar.w_year>>8;//年的高位
								data_once[test_zunum-1].newyears[1]=calendar.w_year;//年的低位
								data_once[test_zunum-1].newmonth=calendar.w_month;//月
								data_once[test_zunum-1].newday=calendar.w_date;//日			
								
								//一组中通气阻力的最大值最小值平均值
								for(i=0;i<test_cishu-1;i++)
								{
									m=data_once[test_zunum-1].onedata[i].once_gas_res[0]<<8;
									m=m+data_once[test_zunum-1].onedata[i].once_gas_res[1];//整数部分
									n=data_once[test_zunum-1].onedata[i].once_gas_res[2]<<8;
									n=n+data_once[test_zunum-1].onedata[i].once_gas_res[3];
									b=m+(float)n/10000;
									a[i]=b;
								}
								bubble_sort(a,test_cishu-1);//冒泡法排序 从小到大			
								//孔径最大值a[4]
								m=a[test_cishu-2];
								n=(a[test_cishu-2]-m)*10000;
								data_once[test_zunum-1].Km_max[0]=m>>8;
								data_once[test_zunum-1].Km_max[1]=m;
								data_once[test_zunum-1].Km_max[2]=n>>8;
								data_once[test_zunum-1].Km_max[3]=n;
								//孔径平均值
								b=0;
								b=(a[0]+a[1]+a[2]+a[3]+a[4])/(test_cishu-1);
								m=b;
								n=(b-m)*10000;
								data_once[test_zunum-1].Km_avg[0]=m>>8;
								data_once[test_zunum-1].Km_avg[1]=m;
								data_once[test_zunum-1].Km_avg[2]=n>>8;
								data_once[test_zunum-1].Km_avg[3]=n;
								//孔径最小值
								m=a[0];
								n=(a[0]-m)*10000;
								data_once[test_zunum-1].Km_min[0]=m>>8;
								data_once[test_zunum-1].Km_min[1]=m;
								data_once[test_zunum-1].Km_min[2]=n>>8;
								data_once[test_zunum-1].Km_min[3]=n;
								/////////////////////////////////////////
								//一组中压差结果的最大值最小值平均值
								for(i=0;i<test_cishu-1;i++)
								{
									m=data_once[test_zunum-1].onedata[i].once_kpa[0]<<8;
									m=m+data_once[test_zunum-1].onedata[i].once_kpa[1];//整数部分
									
									a[i]=m;
								}
								bubble_sort(a,test_cishu-1);//冒泡法排序 从小到大			
								//孔径最大值a[4]
								m=a[test_cishu-2];								
								data_once[test_zunum-1].Ka_max[0]=m>>8;
								data_once[test_zunum-1].Ka_max[1]=m;
								
								//孔径平均值
								b=0;
								b=(a[0]+a[1]+a[2]+a[3]+a[4])/(test_cishu-1);
								m=b;								
								data_once[test_zunum-1].Ka_avg[0]=m>>8;
								data_once[test_zunum-1].Ka_avg[1]=m;
								
								//孔径最小值
								m=a[0];								
								data_once[test_zunum-1].Ka_min[0]=m>>8;
								data_once[test_zunum-1].Ka_min[1]=m;
								
								//结构体数据清空
								for(i=test_cishu-1;i<5;i++)
								{
									data_once[test_zunum-1].onedata[i].once_kpa[0]=0xff;
									data_once[test_zunum-1].onedata[i].once_kpa[1]=0xff;
									
									data_once[test_zunum-1].onedata[i].once_gas_res[2]=0xff;
									data_once[test_zunum-1].onedata[i].once_gas_res[3]=0xff;
									data_once[test_zunum-1].onedata[i].once_gas_res[0]=0xff;
									data_once[test_zunum-1].onedata[i].once_gas_res[1]=0xff;
									
								}
								//保存到flash		
								AT24CXX_WriteOneByte(SAVE_ADDR_ZUSHU,test_zunum);
								AT24CXX_WriteOneByte(SAVE_ADDR_CISHU,test_cishu);								
								SPI_Flash_Write(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,35);
							}
							break;		
						case 4: //清除所有测试数据
							SPI_Flash_Erase_Sector(0);//擦除0号扇区
							test_cishu=1;
							test_zunum=1;
							AT24CXX_WriteOneByte(SAVE_ADDR_ZUSHU,1);
							AT24CXX_WriteOneByte(SAVE_ADDR_CISHU,1);
							for(;j<10;j++)
								SPI_Flash_Read(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,58);
							key_value = 0;						
							break;					
						default:
							break;
					}				
					
				}
		}

	}
}


//AD校准界面
void AD_Calibration_menu(void)
{
	char a[9]={0};
	
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(0xFF);	
	Memory_Clear();	
	Delay1ms(100);//更新显示界面
		//显示图片
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
  
	//显示按钮   | 校准 返回|		
	//DisplayButtonUp(20,215,115,255);//校准
	//DisplayButtonUp(365,215,460,255);//返回
	Put_AnJian(67,235,95,40);
	Put_AnJian(412,235,95,40);
	DisplayButtonUp(100,80,190,100);
	DisplayButtonUp(290,80,380,100);
	
	DisplayButtonUp(100,120,190,140);
	DisplayButtonUp(290,120,380,140);

	
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//设定显示区域
	XY_Coordinate(40,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("AD校准"); 	
	Font_with_BackgroundColor();
	XY_Coordinate(30,220);
	Show_String("校准");	
	XY_Coordinate(375,220);
	Show_String("返回");
	//显示测试数据
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();
	Bold_Font();//字体加粗
	Active_Window(0,480,60,200);//设定显示区域
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//设定文字的前景色和背景色	
	
	XY_Coordinate(110,82);
	Show_String("标定点1");
	XY_Coordinate(110,122);
	Show_String("AD1采样");
	
	XY_Coordinate(310,82);
	Show_String("标定点2");
	XY_Coordinate(310,122);
	Show_String("AD2采样");
	
	//显示设定数字
	XY_Coordinate(0,162);
	Show_String("标定点1：");
	XY_Coordinate(108,162);
	Show_String("(pa)");
	
	sprintf(a,"%05d",biaoding1);
	XY_Coordinate(64,162);
	Show_StringNum(a,5);
	
	XY_Coordinate(150,162);
	Show_String("AD1:");
	sprintf(a,"%05d",AD1value);
	XY_Coordinate(190,162);
	Show_StringNum(a,5);
	
	XY_Coordinate(240,162);
	Show_String("标定点2：");	
	XY_Coordinate(350,162);
	Show_String("(pa)");
	sprintf(a,"%05d",biaoding2);
	XY_Coordinate(304,162);
	Show_StringNum(a,5);
	
	XY_Coordinate(390,162);
	Show_String("AD2:");
	sprintf(a,"%05d",AD2value);
	XY_Coordinate(424,162);
	Show_StringNum(a,5);
	
	XY_Coordinate(100,180);	
	Show_String("K:");
	XY_Coordinate(100+16,180);
	sprintf(a,"%f",press_k);
	Show_StringNum(a,9);
	
	XY_Coordinate(240,180);	
	Show_String("C:");
	if((biaoding1+biaoding2)>=(press_k*(AD1value+AD2value)))//+
	{
		XY_Coordinate(240+16,180);
		sprintf(a,"%f",press_c);
		Show_StringNum(a,9);
	}
	else//-
	{
		XY_Coordinate(240+16,180);	
		Show_String("-");
		XY_Coordinate(240+24,180);
		sprintf(a,"%f",press_c);
		Show_StringNum(a,9);
	}		
	NoBold_Font();//取消字体加粗
	//Graphic_Mode();
}
//标定系数设置
u16 biaoding_menu(void)
{
	u8 i=1,j=0;
	char biaoding[3]={0};
	u16 num=0;
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(color_cyan);	
	Memory_Clear();	
	Delay1ms(100);//更新显示界面
		//显示图片
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);	
	JianPan_Displayfu();//显示虚拟键盘
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//设定显示区域
	XY_Coordinate(40,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("标  定"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("标定参数设置：(0-500)pa");
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) break;//退出大循环
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				biaoding[j]=key_value-10+0x30;//将键值转换成ASCII码暂存
			else 
				biaoding[j]=key_value+0x30;//将键值转换成ASCII码暂存			
			
			j++;			
			if(j>3) 
			{
			XY_Coordinate(240,100);//指定显示位置
			Show_String("out of rang!");	
			Delay1ms(1000);	//延时1S	
			XY_Coordinate(240,100);//指定显示位置
			Show_String("            ");	//清除显示	
			key_value=0;
			j=3;		
			continue;
			}
		}
		switch(key_value)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:					
			case 9:
			case 10://0
					key_value=0;
					XY_Coordinate(90+(j-1)*8,100);//指定显示位置
					Show_StringNum(&biaoding[j-1],1);	
					if(j==3) 
					{
						//for(j=0;j<3;j++) 
							num=(biaoding[0]-0x30)*100+(biaoding[1]-0x30)*10+(biaoding[2]-0x30);
						
						if(num>500)
						{
							num=0;
							XY_Coordinate(240,100);//指定显示位置
							Show_String("out of rang!");	
							Delay1ms(1000);	//延时1S	
							XY_Coordinate(240,100);//指定显示位置
							Show_String("            ");	//清除显示
							key_value=0;
							for(j=0;j<3;j++) biaoding[j]=0;
							j=0;		
							continue;
						}
					}
					break;
			case 11://清零				
					for(j=0;j<3;j++) biaoding[j]=0;
					j=0;
					XY_Coordinate(90,100);//指定显示位置
					Show_String("         ");
					key_value=0;
					break;
			case 12://删除
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 3:
										XY_Coordinate(9+16,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
// 							case 4:
// 										XY_Coordinate(90+24,100);//指定显示位置
// 										Show_StringNum(" ",1);
// 										j--;
// 										break;
// 							case 5:
// 										XY_Coordinate(90+32,100);//指定显示位置
// 										Show_StringNum(" ",1);
// 										j--;
// 										break;						
							default: break;	
						}
					
					break;
			case 13://确认
					if(j==1)
						num=biaoding[0]-0x30;
					else if(j==2)
						num=(biaoding[0]-0x30)*10+(biaoding[1]-0x30);
					else if(j==3)
						num=(biaoding[0]-0x30)*100+(biaoding[1]-0x30)*10+(biaoding[2]-0x30);
// 					else if(j==4)
// 						num=(biaoding[0]-0x30)*1000+(biaoding[1]-0x30)*100+(biaoding[2]-0x30)*10+(biaoding[3]-0x30);
// 					else//j==5
// 						num=biaoding[0]*10000+biaoding[1]*1000+biaoding[2]*100+biaoding[3]*10+biaoding[4];
					XY_Coordinate(360,147);//确定键
					Show_String("保存");
					Delay1ms(100);	//延时1S	
					XY_Coordinate(360,147);//确定键
					Show_String("确定");					
					key_value=0;
					i=0;//退出大循环
					Graphic_Mode();	//恢复图片模式					
					break;
			case 14://返回
					i=0;//退出大循环
					Graphic_Mode();	//恢复图片模式
					key_value=0;
					break;
			default: break;			
		}
	}
		return num;
}


//AD校准界面
void AD_Calibration_Function(void)
{
	u8 flag1=0,flag11=0;
	static u8 flag_return;
	char value[5]={0};
	//u16 AD1value=0,AD2value=0,biaoding1=0,biaoding2=0;//AD采样值
	//u8 idata[16]={0};
	u8 b[16]={0};
	//读取保存的压力AD系数
	AT24CXX_Read(b,17,16);
	pressAD_kint=(b[0]<<8)+b[1];
	pressAD_kfloat=(b[2]<<8)+b[3];

	pressAD_cint=(b[4]<<8)+b[5];
	pressAD_cfloat=(b[6]<<8)+b[7];

	press_k=(float)pressAD_kint+((float)pressAD_kfloat/10000);
	press_c=(float)pressAD_cint+((float)pressAD_cfloat/10000);
	biaoding1=(b[8]<<8)+b[9];	
	AD1value=(b[10]<<8)+b[11];
	biaoding2=(b[12]<<8)+b[13];	
	AD2value=(b[14]<<8)+b[15];
	
	AD_Calibration_menu();	//AD校准菜单
	flag_return=1;
	while(1)
	{
		if(flag_return==0) return;//退出大循环
		KeyScan();
		if(touch_flag)//触摸屏被按下
		{	
			
			if((LCD_X>30)&&(LCD_X<105)&&(LCD_Y>215)&&(LCD_Y<255))//校准按钮被按下
				{						
					key_value=1;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//返回按钮被按下
				{						
					key_value=2;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;
				}			
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>80)&&(LCD_Y<100))//标定点1设置
				{						
					key_value=3;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>120)&&(LCD_Y<140))//AD1采样
				{						
					key_value=4;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>80)&&(LCD_Y<100))//标定点2
				{						
					key_value=5;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}	
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>120)&&(LCD_Y<140))//AD2采样
				{						
					key_value=6;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}	
			else
				{
					key_value=0;
					//speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //校准计算 并显示计算出的K值
							//press_k=(biaoding1-biaoding2)/(AD1-AD2)
						//press_c=((biaoding1+biaoding2)-press_k(AD1+AD2))/2
							Text_Foreground_Color(color_red);
							press_k=(float)(biaoding1-biaoding2)/(AD1value-AD2value);
							press_c=((float)(biaoding1+biaoding2)-press_k*(AD1value+AD2value))/2;
							pressAD_kint=press_k;
							pressAD_kfloat=(press_k-pressAD_kint)*10000;
							pressAD_cint=press_c;
							pressAD_cfloat=(press_c-pressAD_cint)*10000;
							//显示K
							sprintf(value,"%04d",pressAD_kint);
							XY_Coordinate(116,180);
							Show_StringNum(value,4);
							XY_Coordinate(148,180);
							Show_String(".");							
							XY_Coordinate(156,180);
							sprintf(value,"%04d",pressAD_kfloat);
							Show_StringNum(value,4);
						
							//显示C
							if((biaoding1+biaoding2)>=(press_k*(AD1value+AD2value)))//+
							{
							sprintf(value,"%04d",pressAD_cint);
							XY_Coordinate(256,180);
							Show_StringNum(value,4);
							XY_Coordinate(288,180);
							Show_String(".");							
							XY_Coordinate(296,180);
							sprintf(value,"%04d",pressAD_cfloat);
							Show_StringNum(value,4);
							}
							else
							{
							XY_Coordinate(240+16,180);	
							Show_String("-");	
							sprintf(value,"%04d",pressAD_cint);
							XY_Coordinate(264,180);
							Show_StringNum(value,4);
							XY_Coordinate(296,180);
							Show_String(".");							
							XY_Coordinate(304,180);
							sprintf(value,"%04d",pressAD_cfloat);
							Show_StringNum(value,4);

							}	
							//保存系数K
							b[0]=pressAD_kint>>8;
							b[1]=pressAD_kint;
							b[2]=pressAD_kfloat>>8;
							b[3]=pressAD_kfloat;
							
							//保存系数C
							b[4]=pressAD_cint>>8;
							b[5]=pressAD_cint;
							b[6]=pressAD_cfloat>>8;
							b[7]=pressAD_cfloat;
							
							//biaoding1
							b[8]=biaoding1>>8;
							b[9]=biaoding1;
							//ad1
							b[10]=AD1value>>8;
							b[11]=AD1value;
							//biaoding2
							b[12]=biaoding2>>8;
							b[13]=biaoding2;
							//ad2
							b[14]=AD2value>>8;
							b[15]=AD2value;
							AT24CXX_Write(b,17,16)	;						
							key_value = 0;
							flag1=0;
							flag11=0;							
							break;	
						case 2:  //返回							
							key_value = 0;							
							flag_return=0;//退出大循环
							Graphic_Mode();	
							break;						
						case 3:  //标定点1							
							key_value = 0;							
							biaoding1=biaoding_menu();			
						
							AD_Calibration_menu();	//AD校准菜单
							Text_Foreground_Color(color_red);
							sprintf(value,"%05d",biaoding1);
							XY_Coordinate(64,162);
							Show_StringNum(value,5);
							flag1=1;
							break;		
						case 4:  //AD1采样
							pressADvalue=Get_Adc_Average(10, 50);	
							//pressADvalue=read_16bit_pressAD(0,50);
							Text_Foreground_Color(color_red);
							AD1value=pressADvalue;
							sprintf(value,"%05d",AD1value);
							XY_Coordinate(190,162);
							Show_StringNum(value,5);							
							key_value = 0;
							flag11=1;	
							break;
						case 5:  //标定点2								
							key_value = 0;	
							biaoding2=biaoding_menu();
							AD_Calibration_menu();	//AD校准菜单
							Text_Foreground_Color(color_red);
							if(flag1==1)
							{
							sprintf(value,"%05d",biaoding1);
							XY_Coordinate(64,162);
							Show_StringNum(value,5);
							}
							if(flag11==1)
							{
							//AD1value=pressADvalue;
							sprintf(value,"%05d",AD1value);
							XY_Coordinate(190,162);
							Show_StringNum(value,5);
							}
							sprintf(value,"%05d",biaoding2);
							XY_Coordinate(304,162);
							Show_StringNum(value,5);
							flag1=0;
							flag11=0;
							break;
						case 6:  //AD2
							pressADvalue=Get_Adc_Average(10, 50);;
							//pressADvalue=read_16bit_pressAD(0,20);
							Text_Foreground_Color(color_red);
							AD2value=pressADvalue;
							sprintf(value,"%05d",AD2value);
							XY_Coordinate(424,162);
							Show_StringNum(value,5);		
							key_value = 0;							
							break;
						default:
							break;
					}				
					
				}
		}

	}

}

void menu_printselect1(void)
{
	char data[2]={0};
	Active_Window(0,479,0,271);	
	//显示图片
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdFillRec(0,60,480,200,1,color_white);	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	
	//DisplayButtonUp(20,215,115,255);//打印
	//DisplayButtonUp(365,215,460,255);//返回
	Put_AnJian(67,235,95,40);
	Put_AnJian(412,235,95,40);
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//设定显示区域
	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();	
	XY_Coordinate(40,15);	
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("打  印");
	XY_Coordinate(30,220);
	Show_String("打印");
	XY_Coordinate(375,220);
	Show_String("返回");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	Font_with_BackgroundColor();
	
	Active_Window(0,480,60,200);//设定显示区域
	Text_Background_Color(color_white);//设定文字的前景色和背景色	
	//XY_Coordinate(150,96);
	//Show_Cursor();
	if(printf_flag==1)
	{
		XY_Coordinate(150,96);
		Show_Cursor();
		XY_Coordinate(150,148);
		Del_Cursor();
	}		
	else
	{
	XY_Coordinate(150,96);
	Del_Cursor();
	XY_Coordinate(150,148);
	Show_Cursor();
	}
	XY_Coordinate(160,96);
	Show_String("选择打印第几组数据：");
	sprintf(data,"%d",slect_print);
	XY_Coordinate(330,96);
	Show_StringNum(data,2);
	XY_Coordinate(160,148);
	Show_String("选择打印所有组数据!");
}



//打印选择组数界面
void menu_printselect(void)
{	
	u8 flag=1;	
	menu_printselect1();
	while(flag)
	{
		KeyScan();
		if(touch_flag)//触摸屏被按下
		{			
			if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//返回按钮被按下
				{						
					key_value=1;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;
				}			
			else if((LCD_X>30)&&(LCD_X<105)&&(LCD_Y>215)&&(LCD_Y<255))//打印按钮
				{						
					key_value=2;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}
			else if((LCD_X>160)&&(LCD_X<320)&&(LCD_Y>90)&&(LCD_Y<118))//选择打印第几组数据
				{						
					key_value=3;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;
					XY_Coordinate(150,96);
					Show_Cursor();
					XY_Coordinate(150,148);
					Del_Cursor();	
				}
			else if((LCD_X>160)&&(LCD_X<320)&&(LCD_Y>142)&&(LCD_Y<170))//选择打印所有测试数据
				{	
					XY_Coordinate(150,96);
					Del_Cursor();
					XY_Coordinate(150,148);
					Show_Cursor();	
					key_value=4;
					speekers(100);//蜂鸣器响100ms	
					touch_flag=0;	
				}			
			else
				{
					key_value=0;
					//speekers(100);//蜂鸣器响100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //返回						
							flag=0;					
							key_value = 0;
							printf_flag=1;
							break;	
						case 2:  //打印测试数据
							key_value = 0;
							PrintReport();
							menu_printselect1();	
							break;						
						case 3:  //选择打印第几组数据							
							key_value = 0;
							printf_flag=1;		
							set_printnum();
							break;		
						case 4: //选择打印所有测试数据							
							key_value = 0;
							printf_flag=2;
							break;					
						default:
							break;
					}				
					
				}
		}

	}
	
}

void set_printnum(void)
{
	u8 i=1,j=0,k,a;
	char data[2]={0};
	u8 slect=0;
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(color_cyan);	
	Memory_Clear();	
	Delay1ms(100);//更新显示界面
		//显示图片
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);	
	JianPan_Displayfu();//显示虚拟键盘
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//设定显示区域
	XY_Coordinate(40,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("打  印"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("设置打印第几组数据");
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) return;//退出大循环
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				data[j]=key_value-10+0x30;//将键值转换成ASCII码暂存
			else 
				data[j]=key_value+0x30;//将键值转换成ASCII码暂存			
			
			j++;
			if(j>2) 
			{
			XY_Coordinate(240,100);//指定显示位置
			Show_String("out of rang!");	
			Delay1ms(1000);	//延时1S	
			XY_Coordinate(240,100);//指定显示位置
			Show_String("            ");	//清除显示
			key_value=0;
			j=2;		
			continue;
			}
		}
		switch(key_value)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:					
			case 9:
			case 10://0
					key_value=0;					
					XY_Coordinate(90+(j-1)*8,100);//指定显示位置
					Show_StringNum(&data[j-1],1);					
					break;
			case 11://清零					
					for(j=0;j<2;j++) data[j]=0;
					j=0;
					XY_Coordinate(90,100);//指定显示位置
					Show_String("         ");
					key_value=0;
					break;
			case 12://删除
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//指定显示位置
										Show_StringNum(" ",1);
										j--;
										break;						
							default: break;	
						}
					
					break;
			case 13://确认
					for(k=0;k<2;k++) data[k]-=0x30;
					if(j==2)
						slect=data[0]*10+data[1];
					else
						slect=data[0];
					if(test_cishu>1)       a=test_zunum;
					else if(test_cishu==1) a=test_zunum-1;
					if(slect<=a&&slect>0) 
					{
					slect_print=slect;
					
					i=0;//退出大循环	
					Graphic_Mode();	//恢复图片模式		
					key_value=0;
					menu_printselect1();	
					}
					else//密码错误 清除显示区域
					{
						for(k=0;k<2;k++) data[k]=0;
						j=0;
						XY_Coordinate(90,100);//指定显示位置
						Show_String("         ");
						key_value=0;
						XY_Coordinate(240,100);//指定显示位置
						Show_String("slect error!");	
						Delay1ms(1000);	//延时1S	
						XY_Coordinate(240,100);//指定显示位置
						Show_String("            ");	//清除显示
					}
					break;
			case 14://返回
					i=0;//退出大循环
					Graphic_Mode();	//恢复图片模式
					menu_printselect1();
					key_value=0;
					break;
			default: break;			
		}
	}
}

//打印等待界面
void menu_print(void)
{
	Active_Window(0,479,0,271);	
	//显示图片
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdFillRec(0,60,480,200,1,color_white);	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
//	DisplayButtonUp(365,215,460,255);
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//设定显示区域
	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();	
	XY_Coordinate(40,15);	
	Font_with_BackgroundTransparency();//开启穿透功能
	Show_String("打  印");	
	//XY_Coordinate(375,220);
	//Show_String("返回");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	Font_with_BackgroundColor();
	
	Active_Window(0,480,60,200);//设定显示区域
	Text_Background_Color(color_white);//设定文字的前景色和背景色	
	XY_Coordinate(110,110);
	Show_String("正在打印中,请耐心等待打印完成!");
}
//表格绘制
void show_excel(void)
{
	Active_Window(0,479,0,271);
	Graphic_Mode();
	LcdFillRec(0,0,480,200,1,color_white);
	//划横线
	LcdFillRec(1,1,479,200,0,color_black);
	LcdPrintHorz(1,21,478,color_black);
	LcdPrintHorz(1,40,478,color_black);
	LcdPrintHorz(1,59,478,color_black);
	LcdPrintHorz(1,78,478,color_black);
	
	LcdPrintHorz(1,97,478,color_black);
	LcdPrintHorz(1,116,478,color_black);
	LcdPrintHorz(1,135,478,color_black);
	LcdPrintHorz(1,154,478,color_black);
	LcdPrintHorz(1,173,478,color_black);
	//画垂直线	
	LcdPrintVert(115,21,179,color_black);
	LcdPrintVert(290,21,179,color_black);
	//显示文字
	Text_Mode();
	Text_Foreground_Color(color_black);
	Text_Background_Color(color_white);//设定文字的前景色和背景色	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,200);//设定显示区域		
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	Font_with_BackgroundTransparency();//开启穿透功能
	Font_with_BackgroundColor();
	XY_Coordinate(35,3);
	Show_String("测试组号：");	
	XY_Coordinate(315,3);
	Show_String("测试日期：");
	
	XY_Coordinate(40,22);
	Show_String("序号");
	XY_Coordinate(165,22);
	Show_String("压差(Pa)");
	XY_Coordinate(310,22);
	Show_String("通气阻力(Pa/cm2)");
	
	XY_Coordinate(48,41);
	Show_String("1#");
	XY_Coordinate(48,60);
	Show_String("2#");
	XY_Coordinate(48,79);
	Show_String("3#");
	XY_Coordinate(48,98);
	Show_String("4#");
	XY_Coordinate(48,117);
	Show_String("5#");
	XY_Coordinate(44,136);
	Show_String("MAX");
	XY_Coordinate(44,155);
	Show_String("AVG");
	XY_Coordinate(44,174);
	Show_String("MIN");

}
//打印报表
void PrintReport(void)
{	
    u8 i,j,k,m;
		u16 datayear,b;
		char Data[20]={0};
		float a;
		menu_print();//打印过程中的界面
		//读取数据
		for(i=0;i<test_zunum;i++)
			SPI_Flash_Read(&data_once[i].Mark,i*256,58);	
    pprint(0x1b);                                                                                      
    pprint(0x40);
		pprint(0x1b);                                                                                      
    pprint(0x51);
		pprint(0x00);
		if(printf_flag==1)//打印所选择的组数
		{
		m=slect_print-1;
		k=slect_print;	
		}
		else if(printf_flag==2)//打印所有组的测试数据
		{
			if(test_cishu>1)       k=test_zunum;
			else if(test_cishu==1) k=test_zunum-1;
			m=0;
		}
		
		for(i=m;i<k;i++)
		{		
			//5#数据
			for(j=5;j>0;j--)
			{	
				b=data_once[i].onedata[j-1].once_kpa[0]*256+data_once[i].onedata[j-1].once_kpa[1];	
				if(b!=0xffff)
				{//显示压差结果
					PrintStr("  P=");
					sprintf(Data,"%04d",b);
					PrintStrNnum(Data,4);
					PrintStr("Pa");
					pprint(0x0d);  //回车
					pprint(0x0a); //换行
					//显示通气阻力
					sprintf(Data,"%01d",j);
					PrintStrNnum(Data,1);
					PrintStr("#");
					PrintStr("G=");
					a=data_once[i].onedata[j-1].once_gas_res[0]*256+data_once[i].onedata[j-1].once_gas_res[1]+(float)(data_once[i].onedata[j-1].once_gas_res[2]*256+data_once[i].onedata[j-1].once_gas_res[3])/10000;
					sprintf(Data,"%5.2f",a);
					PrintStrNnum(Data,5);
					PrintStr("Pa/cm2");
					pprint(0x0d);  //回车
					pprint(0x0a); //换行
					
				}	
			}	
			a=data_once[i].Ka_max[0]*256+data_once[i].Ka_max[1];
			if(a!=0xffff)
			{	
			//---------------------
			PrintStr("---------------");
			pprint(0x0d);  //回车
			pprint(0x0a); //换行			
			//ka_max
			PrintStr("PMax=");
			b=data_once[i].Ka_max[0]*256+data_once[i].Ka_max[1];	
			sprintf(Data,"%04d",b);
			PrintStrNnum(Data,4);
			PrintStr("Pa");
			pprint(0x0d);  //回车
			pprint(0x0a); //换行
			//ka_avg
			PrintStr("PAvg=");
			b=data_once[i].Ka_avg[0]*256+data_once[i].Ka_avg[1];
			sprintf(Data,"%04d",b);
			PrintStrNnum(Data,4);
			PrintStr("Pa");
			pprint(0x0d);  //回车
			pprint(0x0a); //换行
			//ka_min
			PrintStr("PMin=");
			b=data_once[i].Ka_min[0]*256+data_once[i].Ka_min[1];
			sprintf(Data,"%04d",b);
			PrintStrNnum(Data,4);
			PrintStr("Pa");
			pprint(0x0d);  //回车
			pprint(0x0a); //换行
			//Km_max
			PrintStr("GMax=");
			a=data_once[i].Km_max[0]*256+data_once[i].Km_max[1]+(float)(data_once[i].Km_max[2]*256+data_once[i].Km_max[3])/10000;
			sprintf(Data,"%5.2f",a);
			PrintStrNnum(Data,5);
			PrintStr("Pa/cm2");
			pprint(0x0d);  //回车
			pprint(0x0a); //换行
			//Km_avg
			PrintStr("GAvg=");
			a=data_once[i].Km_avg[0]*256+data_once[i].Km_avg[1]+(float)(data_once[i].Km_avg[2]*256+data_once[i].Km_avg[3])/10000;
			sprintf(Data,"%5.2f",a);
			PrintStrNnum(Data,5);
			PrintStr("Pa/cm2");
			pprint(0x0d);  //回车
			pprint(0x0a); //换行
			//Km_min
			PrintStr("GMin=");
			a=data_once[i].Km_min[0]*256+data_once[i].Km_min[1]+(float)(data_once[i].Km_min[2]*256+data_once[i].Km_min[3])/10000;
			sprintf(Data,"%5.2f",a);
			PrintStrNnum(Data,5);
			PrintStr("Pa/cm2");
			pprint(0x0d);  //回车
			pprint(0x0a); //换行
			//---------------------
			PrintStr("---------------");
			pprint(0x0d);  //回车
			pprint(0x0a); //换行
			//日期
			datayear=data_once[i].newyears[0]*256+data_once[i].newyears[1];
			PrintStr("Data:");
			sprintf(Data,"%04d",datayear);
			PrintStrNnum(Data,4);
			PrintStr("/");
			
			sprintf(Data,"%02d",data_once[i].newmonth);
			PrintStrNnum(Data,2);
			PrintStr("/");
			
			sprintf(Data,"%02d",data_once[i].newday);
			PrintStrNnum(Data,2);
			pprint(0x0d);  //回车
			pprint(0x0a); //换行
			
			//组号
			PrintStr("NO:");
			sprintf(Data,"%02d",(i+1));			
			PrintStrNnum(Data,2);	
			pprint(0x0d);  //回车
			pprint(0x0a); //换行
		}
	} 
	pprint(0x0d);  //回车
	pprint(0x0a); //换行
	pprint(0x0d);  //回车
	pprint(0x0a); //换行
 
		 
}	
//排序函数并返回平均值  times:排序数据的个数 

u16 paixu(u8 times,u16 rcv_data[])
{
	u8  i, j; 
	u16 a;
	u32 b;

	for(i=0;i<times-1;i++)
	for(j=i+1;j<times;j++)
	if (rcv_data[i] <rcv_data[j])
			{
				a = rcv_data[i];
				rcv_data[i] = rcv_data[j];
				rcv_data[j] = a;
			}
	b=0;
	for(i=1; i<(times-1); i++)
	{
		b += rcv_data[i];
	}
	a=b/(times-2);
	return a;
}
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
  while (1)
  {
  }
}
#endif


