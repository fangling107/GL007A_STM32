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

//����EEPROM�ı����ַ 0-13 ��ĻУ׼ϵ��
//14���Լ�ϵ��
//15�����Դ���
//16����������
//17-18��AD K�������� 19-20��AD KС������
//21-22: AD C�������� 23-24��AD CС������
#define SAVE_ADDR_XISHU 14 //EEPROM����ϵ���ĵ�ַ 14��ʼ ��0-13���������ĻУ׼ϵ����
#define SAVE_ADDR_CISHU 15	//EEPROM������Դ����ĵ�ַ 15��ʼ 
#define SAVE_ADDR_ZUSHU 16	//EEPROM������������ĵ�ַ 16��ʼ
 
//ADУ׼����
#define password 70605

//��������
static u8 key_value;//������ֵ

static u8 test_cishu=1;//�������
static u8 test_zunum=1;//�������
extern float press_k;//����б��
extern float press_c;//����б��
//250ms���I
u8 RTC_200MS=0;
u8 RTC_250MS=0;
//��/������־
static u8 open_off_flag;

u16 pressAD_kint,pressAD_kfloat;//����ϵ����������С������
u16 pressAD_cint,pressAD_cfloat;//����ϵ����������С������

u16 AD1value=0,AD2value=0,biaoding1=0,biaoding2=0;//AD����ֵ

u16 press_zeroAD=0;//���AD
float gas_res=0;//ͨ������
u8 test_succesflag=0;//���Գɹ���־
u8 checkTestnum=0;//���ڲ�ѯ������


extern u16 pressADvalue;//ADֵ
volatile u16 pressvalue_avg;//ADֵ  ƽ��ֵ Ϊѹ����
static u8 password_flag;
static u8 slect_print=1;
static u8 printf_flag=1;
//��������
void menu1(void);//��ʼ����
void menu_setting(void);//���ý���
void menu_menu(void);//�˵�
void key_function(void);
void display_number(void);
void JianPan_Displayfu(void);//��ʾ�������
void PrintReport(void);
void speekers(u16 time);//������ time/1ms
void AD_Calibration_menu(void);//ADУ׼����
void AD_Calibration_Function(void);//ADУ׼
u16 biaoding_menu(void);//�궨
u16 display_press(u16 pressAD,u16 x,u16 y);//ָ��λ����ʾѹ��ֵ
float display_holdsize(u16 pressAD,u16 x,u16 y);//ָ��λ����ʾ�׾�ֵ
void menu_flashmax(void);//����������ʾ
void save_data(void);//����������ݵ�EEPROM
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
//��������
//struct oncetest  onedata[5];//80b ���ڴ洢5�β�������
struct data_diapaly data_once[10];//���ڴ洢10���������
//������
int main(void)
 {	
	u8 b[16]={0};

  InitHardware();										//Ӳ����ʼ��
	//my_mem_init(0);										//�ڴ�������
	while(RTC_Init())	;								//RTC��ʼ��
	AT24CXX_Init();										//AT24C02��ʼ��
	while(AT24CXX_Check());						//���24c02	
	

	Printer_Init();										//��ӡ�˿ڳ�ʼ��	
	SPI_Flash_Init();									//W25Q16��ʼ��				
	SPI_AD8341_Init();								//AD8341��ʼ��
	LCDRA8870_Init();;								//LCD�ĳ�ʼ��	
	RA8870_TouchInit();								//��������ʼ��
	Adc_Init();
	while(TP_Get_Adjdata()==0)						//��ȡУ׼���������Ƿ�У׼��
		XY_Calibration_Function();				//������У׼
	//��ȡ�����ѹ��ADϵ��
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
	
	//��ȡ������źͲ��Դ���
	test_cishu=	AT24CXX_ReadOneByte(SAVE_ADDR_CISHU);	
	test_zunum=	AT24CXX_ReadOneByte(SAVE_ADDR_ZUSHU);
	if(test_cishu>5) test_cishu=1;
	SPI_Flash_Read(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,30+7*(test_cishu-1));
	menu1();//���س�ʼ�˵�����	
	speekers(500);//��������500mS
	
	while(1)
	{		
		if(RTC_250MS)
		{
			RTC_250MS=0;
			pressADvalue=Get_Adc_Average(10, 50);;//12λAD
			display_press(pressADvalue,280,110);//��ʾʵʱѹ��
			display_time();//��ʾʱ��
		}		
		key_function();//������İ������ܺ���
	}
}

//������İ������ܺ���
void key_function(void)
{
	static u8 j,k;
	u16 data_press[5]={0};

		KeyScan();//���а���ɨ�裬��ת��Ϊ��Ļ����ֵ
		if(touch_flag)//������������
		{	
			if((LCD_X>30)&&(LCD_X<105)&&(LCD_Y>215)&&(LCD_Y<255))//�˵���ť������
				{						
					key_value=1;//���ð�����־	
					speekers(100);//��������100ms
					touch_flag=0;//��ǰ����Ѿ����������.	
				}
			else if((LCD_X>145)&&(LCD_X<220)&&(LCD_Y>215)&&(LCD_Y<255))//ͨ����ť������
				{				
					key_value=2;//	�˵�������־
					speekers(100);//��������100ms					
					touch_flag=0;//��ǰ����Ѿ����������.	
				}
			else if((LCD_X>260)&&(LCD_X<335)&&(LCD_Y>215)&&(LCD_Y<255))//���㰴ť������
				{						
					key_value=3;//	ֹͣ������־
					speekers(100);//��������100ms
					touch_flag=0;//��ǰ����Ѿ����������.
				}	
			else if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//���԰�ť������
				{					
					key_value=4;//	���԰�����־
					touch_flag=0;//��ǰ����Ѿ����������.	
					speekers(100);//��������100ms
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
					Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
					switch(key_value)
					{
						case 1:  //�˵�
							key_value = 0;
							menu_menu();												
							menu1();//���س�ʼ�˵�����
							display_number();
							display_pre(pressvalue_avg,280,140);												
							gas_res=display_holdsize(pressvalue_avg,280,170);//��ʾͨ������
							break;
						case 2://ͨ��
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
						case 3://����
							key_value = 0;
						//�������
							press_zeroAD=pressADvalue;										
							gas_res=0;
							display_number();
							display_press(0,280,110);//��ʾʵʱѹ��
							display_press(0,280,140);//��ʾѹ����															
							display_holdsize(0,280,170);//��ʾͨ������
								
							Graphic_Mode();	
							LcdFillRec(0,0,360,60,1,color_cyan);	
							LcdPrint8bitBmp(gImage_222,5,10,174,40);
							Text_Mode();
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
							External_CGROM();
							External_CGROM_GB();
							Active_Window(0,480,0,272);//�趨��ʾ����
							XY_Coordinate(50,15);
							Horizontal_FontEnlarge_x2();
							Vertical_FontEnlarge_x2();
							Font_with_BackgroundTransparency();//������͸����
							Show_String("��  ��"); 	
							Font_with_BackgroundColor();
							Horizontal_FontEnlarge_x1();
							Vertical_FontEnlarge_x1();
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
							
							break;
						case 4://����	
							key_value = 0;								
													
							Graphic_Mode();	
							LcdFillRec(0,0,360,60,1,color_cyan);	
							LcdPrint8bitBmp(gImage_222,5,10,174,40);
							AnJianPressed(412,235,95,40);;//��������
							Text_Mode();								
							External_CGROM();
							External_CGROM_GB();
							Active_Window(0,480,0,272);//�趨��ʾ����						
							Horizontal_FontEnlarge_x2();
							Vertical_FontEnlarge_x2();
							Font_with_BackgroundTransparency();//������͸����							
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
							XY_Coordinate(50,15);
							Show_String("��  ��");							
							Text_Foreground_Color(color_green);	
							XY_Coordinate(375,220);
							Show_String("����");							
							Font_with_BackgroundColor();
							Horizontal_FontEnlarge_x1();
							Vertical_FontEnlarge_x1();
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
							
							OS_TimeMS=0;	
							pressvalue_avg=0;
							gas_res=0;								
							
							do{
								if(RTC_200MS)
									{
										RTC_200MS=0;
										pressADvalue=Get_Adc_Average(10, 50);;//12λAD											
										data_press[k]=display_press(pressADvalue,280,110);//��ʾʵʱѹ��
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
											pressvalue_avg=paixu(5,data_press);//����ƽ��ѹ���Ϊѹ���������ʾ
											test_succesflag=1;												
										}
									}										
								}while(k!=5);
								k=0;															
								display_pre(pressvalue_avg,280,140);																	
								gas_res=display_holdsize(pressvalue_avg,280,170);//��ʾͨ������	
								if(test_succesflag)
								{
									test_succesflag=0;									
									save_data();//���Գɹ��󱣴�������ݵ�flash
									test_cishu++;//���Գɹ� ���Դ�������1																	
									Graphic_Mode();	
									LcdFillRec(0,0,360,60,1,color_cyan);	
									LcdPrint8bitBmp(gImage_222,5,10,174,40);
									Text_Mode();
									Text_Foreground_Color(color_blue);
									Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
									External_CGROM();
									External_CGROM_GB();
									Active_Window(0,480,0,272);//�趨��ʾ����
									XY_Coordinate(30,15);
									Horizontal_FontEnlarge_x2();
									Vertical_FontEnlarge_x2();
									Font_with_BackgroundTransparency();//������͸����							
									Show_String("���Գɹ�"); 									
									Font_with_BackgroundColor();
									Horizontal_FontEnlarge_x1();
									Vertical_FontEnlarge_x1();
									Text_Foreground_Color(color_blue);
									Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
								}	
								
								if(test_cishu==6) 
								{	
									test_cishu=1;//�������1-5
									test_zunum++;//�������+1	
								}				

							//���������źͲ��Դ���
							AT24CXX_WriteOneByte(SAVE_ADDR_ZUSHU,test_zunum);
							AT24CXX_WriteOneByte(SAVE_ADDR_CISHU,test_cishu);
							Put_AnJian(412,235,95,40);
							display_number();//������ʾ���Դ���
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
							Horizontal_FontEnlarge_x2();
							Vertical_FontEnlarge_x2();								
							XY_Coordinate(375,220);
							Show_String("����");
							Horizontal_FontEnlarge_x1();
							Vertical_FontEnlarge_x1();
							Text_Foreground_Color(color_blue);
							Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ

							//������ɺ󣬹ر���Դ
							j=0;
							open_off_flag=0;
							off_gas();
							
							break;
							
						default:
							break;						
					}				
					key_value=0;//���������־
				}
		}
}
void flash_full(void)
{
	u8 i;
	i=1;
								
	menu_flashmax();//��ʾ��������
	while(i)
		{											
			KeyScan();
			if(touch_flag)//������������
			{	
				if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//���ذ�ť������
				{				
					key_value=1;//	�˵�������־
					speekers(100);//��������100ms
					touch_flag=0;//��ǰ����Ѿ����������.	
				}	
				else
					{
						key_value=0;
						//speekers(100);//��������100ms		
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
								menu1();//���س�ʼ�˵�����
								break;														
							default:
								break;
						}					
						key_value=0;//���������־
					}
			}
			
		}										
	
}
void open_gas(void)
{
	//��ʾͼƬ
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);	
	LcdPrint8bitBmp(gImage_111,0,17,25,25);
	AnJianPressed(182,235,95,40);//��������
	//��ʾ����
	Text_Mode();	
	Text_Foreground_Color(color_blue);	
	XY_Coordinate(30,22);
	Bold_Font();//����Ӵ�
	Show_String("ҽ�ÿ���ѹ�������");
	NoBold_Font();//ȡ������Ӵ�	
	
	Horizontal_FontEnlarge_x2();
	Vertical_FontEnlarge_x2();
	Text_Foreground_Color(color_green);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	XY_Coordinate(145,220);
	Show_String("����");							
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();
	Text_Foreground_Color(color_blue);
	starttest();//���ô���
}
void off_gas(void)
{
	Put_AnJian(182,235,95,40);
	Horizontal_FontEnlarge_x2();
	Vertical_FontEnlarge_x2();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
	XY_Coordinate(145,220);
	Show_String("ͨ��");							
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();	
	stoptest();//ֹͣ����
}
void plese_opengas(void)
{
		Graphic_Mode();	
		LcdFillRec(0,0,360,60,1,color_cyan);	
		LcdPrint8bitBmp(gImage_222,5,10,174,40);
		Text_Mode();
		Text_Foreground_Color(color_blue);
		Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
		External_CGROM();
		External_CGROM_GB();
		Active_Window(0,480,0,272);//�趨��ʾ����
		XY_Coordinate(30,15);
		Horizontal_FontEnlarge_x2();
		Vertical_FontEnlarge_x2();
		Font_with_BackgroundTransparency();//������͸����							
		Show_String("����ͨ��"); 	
		Font_with_BackgroundColor();
		Horizontal_FontEnlarge_x1();
		Vertical_FontEnlarge_x1();
		Text_Foreground_Color(color_blue);
		Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
}
void test_failule(void)
{
		Graphic_Mode();	
		LcdFillRec(0,0,360,60,1,color_cyan);	
		LcdPrint8bitBmp(gImage_222,5,10,174,40);
		Text_Mode();
		Text_Foreground_Color(color_blue);
		Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
		External_CGROM();
		External_CGROM_GB();
		Active_Window(0,480,0,272);//�趨��ʾ����
		XY_Coordinate(30,15);
		Horizontal_FontEnlarge_x2();
		Vertical_FontEnlarge_x2();
		Font_with_BackgroundTransparency();//������͸����							
		Show_String("����ʧ��"); 	
		Font_with_BackgroundColor();
		Horizontal_FontEnlarge_x1();
		Vertical_FontEnlarge_x1();
		Text_Foreground_Color(color_blue);
		Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
}
//��ʾ���
void display_number(void)
{
	char mdata[2]={0};
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
	//��
	sprintf(mdata,"%02d",test_zunum);
	XY_Coordinate(280,80);
	Show_StringNum(mdata,2);
	XY_Coordinate(296,80);
	Show_String("/");
	//��
	sprintf(mdata,"%02d",test_cishu);
	XY_Coordinate(304,80);
	Show_StringNum(mdata,2);

	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
}

//ָ��λ����ʾ�׾�ֵ ����ëϸ�ܷ���ʽ����
//pressAD��ѹ��ֵ pa 
//x,y:��Ҫ��ʾ��λ��
//���ص��ǿ׾�ֵ 4λС��
float display_holdsize(u16 pressAD,u16 x,u16 y)
{

	float a;
	char mdata[6]={0};

	a=((float)pressAD)/4.9;
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
	sprintf(mdata,"%-6.2f",a);
	XY_Coordinate(x,y);
	Show_StringNum(mdata,6);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
	return a;
}
//ָ��λ����ʾѹ��ֵ
//pressAD:ADֵ
u16 display_press(u16 pressAD,u16 x,u16 y)
{
	u16 a;
	u16 press_now;
	char mdata[4]={0};	
	press_now=press_k*(pressAD-press_zeroAD);
	if(pressAD<press_zeroAD) press_now=0;
	if(press_now>=500||pressAD>=4095) //���ֳ�������,�����ر����ţ���ֹ�𻵴�����
	{
		if(open_off_flag) off_gas();//�ر�����		
	}
	a=press_now;
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
	sprintf(mdata,"%04d",a);
	XY_Coordinate(x,y);
	Show_StringNum(mdata,4);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
	return press_now;
}
//ָ��λ����ʾѹ��ֵ
//pressAD:ADֵ
void display_pre(u16 press,u16 x,u16 y)
{	
	char mdata[4]={0};		
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
	sprintf(mdata,"%04d",press);
	XY_Coordinate(x,y);
	Show_StringNum(mdata,4);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
	
}

void speekers(u16 time)
{
 	GPIO_SetBits(GPIOB,GPIO_Pin_8);						 //PB8 �����
 	Delay1ms(time);//������ʾ����
 	GPIO_ResetBits(GPIOB,GPIO_Pin_8);					//PB8 ����� 
}
//ð�ݷ����� ��С��������
void bubble_sort(float a[], u8 n)
{	u8 i, j;
	float temp;
	for (j = 0; j < n - 1; j++)
			for (i = 0; i < n - 1 - j; i++)
			 if(a[i] > a[i + 1])
					{temp=a[i]; a[i]=a[i+1]; a[i+1]=temp;}
}
//����������д��ṹ�� 
void save_data(void)
{
	u16 m,n;
	u8 i;

	float a[5]={0};
	float b;
		
		
		//�������
		data_once[test_zunum-1].onedata[test_cishu-1].once_xuhao=test_cishu;//�������
		//����ѹ����
		
		data_once[test_zunum-1].onedata[test_cishu-1].once_kpa[0]=pressvalue_avg>>8;//ѹ�����ĸ�8λ
		data_once[test_zunum-1].onedata[test_cishu-1].once_kpa[1]=pressvalue_avg;//ѹ�����ĵ�8λ
				
		//ͨ������ С������*10000
		m=gas_res;//��������		
		n=(gas_res-m)*10000;//С������
		data_once[test_zunum-1].onedata[test_cishu-1].once_gas_res[0]=m>>8;//ͨ�������������ָ�8λ
		data_once[test_zunum-1].onedata[test_cishu-1].once_gas_res[1]=m;//ͨ�������������ֵ�8λ
		data_once[test_zunum-1].onedata[test_cishu-1].once_gas_res[2]=n>>8;//ͨ������С�����ָ�8λ
		data_once[test_zunum-1].onedata[test_cishu-1].once_gas_res[3]=n;//ͨ������С�����ֵ�8λ
		
	
		data_once[test_zunum-1].Mark=test_zunum;//���
		data_once[test_zunum-1].newyears[0]=calendar.w_year>>8;//��ĸ�λ
		data_once[test_zunum-1].newyears[1]=calendar.w_year;//��ĵ�λ
		data_once[test_zunum-1].newmonth=calendar.w_month;//��
		data_once[test_zunum-1].newday=calendar.w_date;//��			

		//һ����ͨ�����������ֵ��Сֵƽ��ֵ
		for(i=0;i<test_cishu;i++)
		{
			m=data_once[test_zunum-1].onedata[i].once_gas_res[0]<<8;
			m=m+data_once[test_zunum-1].onedata[i].once_gas_res[1];//��������
			n=data_once[test_zunum-1].onedata[i].once_gas_res[2]<<8;
			n=n+data_once[test_zunum-1].onedata[i].once_gas_res[3];
			b=m+(float)n/10000;
			a[i]=b;
		}
		bubble_sort(a,test_cishu);//ð�ݷ����� ��С����			
			//ͨ���������ֵa[4]
			m=a[test_cishu-1];
			n=(a[test_cishu-1]-m)*10000;
			data_once[test_zunum-1].Km_max[0]=m>>8;
			data_once[test_zunum-1].Km_max[1]=m;
			data_once[test_zunum-1].Km_max[2]=n>>8;
			data_once[test_zunum-1].Km_max[3]=n;
			//�׾�ƽ��ֵ
			b=0;
			b=(a[0]+a[1]+a[2]+a[3]+a[4])/test_cishu;
			m=b;
			n=(b-m)*10000;
			data_once[test_zunum-1].Km_avg[0]=m>>8;
			data_once[test_zunum-1].Km_avg[1]=m;
			data_once[test_zunum-1].Km_avg[2]=n>>8;
			data_once[test_zunum-1].Km_avg[3]=n;
			//�׾���Сֵ
			m=a[0];
			n=(a[0]-m)*10000;
			data_once[test_zunum-1].Km_min[0]=m>>8;
			data_once[test_zunum-1].Km_min[1]=m;
			data_once[test_zunum-1].Km_min[2]=n>>8;
			data_once[test_zunum-1].Km_min[3]=n;
			/////////////////////////////////////////
			//һ����ѹ���������ֵ��Сֵƽ��ֵ
			for(i=0;i<test_cishu;i++)
			{
				m=data_once[test_zunum-1].onedata[i].once_kpa[0]<<8;
				m=m+data_once[test_zunum-1].onedata[i].once_kpa[1];//��������					
				a[i]=m;
			}
			bubble_sort(a,test_cishu);//ð�ݷ����� ��С����			
			//ѹ�������ֵa[4]
			m=a[test_cishu-1];			
			data_once[test_zunum-1].Ka_max[0]=m>>8;
			data_once[test_zunum-1].Ka_max[1]=m;
			
			//ѹ����ƽ��ֵ
			b=0;
			b=(a[0]+a[1]+a[2]+a[3]+a[4])/test_cishu;
			m=b;			
			data_once[test_zunum-1].Ka_avg[0]=m>>8;
			data_once[test_zunum-1].Ka_avg[1]=m;
			
			//ѹ������Сֵ
			m=a[0];			
			data_once[test_zunum-1].Ka_min[0]=m>>8;
			data_once[test_zunum-1].Ka_min[1]=m;
			
			//���浽flash		
			SPI_Flash_Write(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,30+7*(test_cishu-1));
				
		
	
}
//��ʾ�洢�ռ�����
void menu_flashmax(void)
{
	Active_Window(0,479,0,271);	
	//��ʾͼƬ
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdFillRec(0,60,480,200,1,color_white);	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	//DisplayButtonUp(365,215,460,255);
	Put_AnJian(412,235,95,40);
	
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//�趨��ʾ����
	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();	
	XY_Coordinate(20,15);	
	Font_with_BackgroundTransparency();//������͸����
	Show_String("��������");	
	XY_Coordinate(375,220);
	Show_String("����");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	Font_with_BackgroundColor();
	
	Active_Window(0,480,60,200);//�趨��ʾ����
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	XY_Coordinate(100,110);
	Show_String("�밴�����ء���,���롶�˵��������������!");
}


//��ʼ�˵�����
void menu1(void)
{
	Active_Window(0,479,0,271);
	Text_Background_Color(color_white);  
	Memory_Clear();
	//��ʾͼƬ
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);	
	LcdPrint8bitBmp(gImage_111,0,17,25,25);
	
	//��ʾ��ť
// 	DisplayButtonUp(20,215,115,255);
// 	DisplayButtonUp(135,215,230,255);
// 	DisplayButtonUp(250,215,345,255);
// 	DisplayButtonUp(365,215,460,255);
	Put_AnJian(67,235,95,40);
	Put_AnJian(182,235,95,40);
	Put_AnJian(297,235,95,40);
	Put_AnJian(412,235,95,40);
	//��ʾ����
	Text_Mode();
	
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//�趨��ʾ����
	XY_Coordinate(30,22);
	Bold_Font();//����Ӵ�
	Show_String("ҽ�ÿ���ѹ�������");
	NoBold_Font();//ȡ������Ӵ�	
  
	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	//Text_Background_Color(color_white);
	XY_Coordinate(30,220);
	Show_String("�˵�");
	XY_Coordinate(145,220);
	Show_String("ͨ��");
	XY_Coordinate(260,220);
	Show_String("����");
	XY_Coordinate(375,220);
	Show_String("����");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	//��ʾ��������
	Active_Window(0,480,60,200);//�趨��ʾ����
	
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	XY_Coordinate(100,80);
	Show_String("���/���:");
	XY_Coordinate(100,110);
	Show_String("ʵʱѹ��(Pa):");
	XY_Coordinate(100,140);
	Show_String("ѹ����(Pa):");
	XY_Coordinate(100,170);
	Show_String("ͨ������(Pa/cm2):");
	
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨ʱ�����ֵ�ǰ��ɫ�ͱ���ɫ

	display_press(0,280,140);//��ʾѹ����															
	display_holdsize(0,280,170);//��ʾͨ������
	display_number();
	if(open_off_flag)	open_gas();
	else off_gas();
}
//���ý���
/*
void menu_setting(void)
{
	u8 flag=1,xishu_flag=0;
	u8 temp;

	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(0xFF);	
	Memory_Clear();	
	Delay1ms(100);//������ʾ����
		//��ʾͼƬ
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	LcdPrint8bitBmp(gImage_333,100,70,45,60);
	 //��������ʼ����Ϊ��0��0���������СΪ100*100���������ݣ����Ƶ�ָ��λ�ã�240��170����ʾ
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
  
	//��ʾ��ť
	DisplayButtonUp(20,215,115,255);
	DisplayButtonUp(365,215,460,255);
	
		//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//�趨��ʾ����
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//������͸����
	Show_String("�Լ�ѡ��"); 	
	Font_with_BackgroundColor();	
	XY_Coordinate(30,220);
	Show_String("ȷ��");
	XY_Coordinate(375,220);
	Show_String("����");

	//��ʾ��������
	Active_Window(0,480,60,200);//�趨��ʾ����
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	XY_Coordinate(90,140);
	Show_String("�״�");
	XY_Coordinate(207,140);
	Show_String("�Ҵ�");
	XY_Coordinate(304,140);
	Show_String("�����");

	Text_Foreground_Color(color_red);
	Text_Background_Color(color_cyan);	
	Active_Window(0,480,0,60);//�趨��ʾ����	
	//��ʾ�����ϵ��ֵ	
	temp=AT24CXX_ReadOneByte(SAVE_ADDR_XISHU);	
	if(temp==227)//�״�
	{
	XY_Coordinate(380,15);						
	Show_String("�״�"); 
	}	
	else if(temp==228)//�Ҵ�
	{
	XY_Coordinate(380,15);
	Show_String("�Ҵ�");
	}	
	else//�����
	{
	XY_Coordinate(380,15);					
	Show_String("�����");
	}	
	while(1)
	{		
		if(flag==0) return;//�˳���ѭ��
		KeyScan();
		if(touch_flag)//������������
		{	
			if((LCD_X>20)&&(LCD_X<115)&&(LCD_Y>215)&&(LCD_Y<255))//ȷ�ϰ�ť������
				{						
					key_value=1;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>340)&&(LCD_X<435)&&(LCD_Y>215)&&(LCD_Y<255))//���ذ�ť������
				{						
					key_value=2;
					speekers(100);//��������100ms		
					touch_flag=0;
				}
			else if((LCD_X>80)&&(LCD_X<145)&&(LCD_Y>70)&&(LCD_Y<170))//�״�
				{						
					key_value=3;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>197)&&(LCD_X<262)&&(LCD_Y>70)&&(LCD_Y<170))//�Ҵ�
				{							
					key_value=4;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}	
			else if((LCD_X>294)&&(LCD_X<359)&&(LCD_Y>70)&&(LCD_Y<170))//�����
				{					
					key_value=5;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}	
			else
				{
					key_value=0;
					//speekers(100);//��������100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{
					
					
					switch(key_value)
					{
						case 1:  //ȷ��  ����ϵ������
							key_value = 0;	
							if(xishu_flag==3)//�״�ϵ�� 0.0227
							{
								temp=0.0227*10000;								
								AT24CXX_WriteOneByte(SAVE_ADDR_XISHU,temp); 
							}							
							else if(xishu_flag==5)//�����ϵ�� 0.0217
							{
								temp=0.0217*10000;
								AT24CXX_WriteOneByte(SAVE_ADDR_XISHU,temp); 
							}
							else//Ĭ�ϵ�Ϊ�Ҵ�
							{
								temp=0.0228*10000;
								AT24CXX_WriteOneByte(SAVE_ADDR_XISHU,temp); 
							}
							if(temp==228)	midu=0.789;//�Ҵ����ܶ�
							else if(temp==227)	midu=0.7918;//�״����ܶ�
							else if(temp==217)	midu=0.7855;//��������ܶ�
							xishu=(float)temp/10000;//�õ������ϵ��ֵ
							XY_Coordinate(380,15);
							Show_String("      ");
							XY_Coordinate(380,15);						
							Show_String("����");	
							break;						
						case 2://����
							key_value = 0;
							flag=0;
						  Horizontal_FontEnlarge_x1();
							Vertical_FontEnlarge_x1();						
							Graphic_Mode();
							break;
						case 3://�״�							
							XY_Coordinate(380,15);
							Show_String("      ");
							XY_Coordinate(380,15);						
							Show_String("�״�"); 
							xishu_flag=3;
							key_value = 0;																			
							break;
						case 4://											
							XY_Coordinate(380,15);
							Show_String("      ");
							XY_Coordinate(380,15);
							Show_String("�Ҵ�");
							xishu_flag=4;
							key_value = 0;
							break;
						case 5://						
							XY_Coordinate(380,15);
							Show_String("      ");
							XY_Coordinate(380,15);					
							Show_String("�����");	
							xishu_flag=5;
							key_value = 0;
							break;
						default:
							break;
					}					
					key_value=0;//���������־
				}
		}
	}
}
*/
//��ʾ�������
void JianPan_Displayfu(void)
{
	Graphic_Mode();	
	LcdFillRec(75,60,410,260,1,color_white);//��һ����ɫ���η���
	DisplayButtonUp(75,60,410,260);//��һ�����̵Ĵ����	
	DisplayButton(80,65,405,90);
	DisplayButton(80,95,360,120);//����ʾ����
	//������
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
	 DisplayButton(223,216,263,246); //����	
	 DisplayButton(285,216,325,246); //ɾ��	 
	 DisplayButton(347,125,405,185); //ȷ��		
	 DisplayButton(347,195,405,255); //����
	
		
	Text_Mode();						//�ַ�ģʽ(LCDĬ��Ϊͼ��ģʽ)
	Text_Foreground_Color(color_red);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,60,272);//�趨��ʾ����

	XY_Coordinate(360,147);//ȷ����
	Show_String("ȷ��");
	XY_Coordinate(360,217);	//���ؼ�
	Show_String("����");

	NoBackgroundColor();//�����ޱ���ɫ
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
	
	XY_Coordinate(227,223);	//����
	Show_String("����");

	XY_Coordinate(289,223);	//ɾ��
	Show_String("ɾ��");	
	BackgroundColor();
	
}

//����ɨ��
void ScanKeyBoard(void)
{
	KeyScan();
	if(touch_flag)//�а������²�����
	{
		if((LCD_X>99)&&(LCD_X<139)&&(LCD_Y>132)&&(LCD_Y<162))//  1
				{						
					key_value=1;
					speekers(100);//��������100ms	
					touch_flag=0;
				}

		else if((LCD_X>161)&&(LCD_X<201)&&(LCD_Y>132)&&(LCD_Y<162))// 2
				{						
					key_value=2;
					speekers(100);//��������100ms		
					touch_flag=0;
				}

		else if((LCD_X>223)&&(LCD_X<263)&&(LCD_Y>132)&&(LCD_Y<162))// 3
				{						
					key_value=3;
					speekers(100);//��������100ms		
					touch_flag=0;
				}
		else if((LCD_X>285)&&(LCD_X<325)&&(LCD_Y>132)&&(LCD_Y<162))// 4
				{						
					key_value=4;
					speekers(100);//��������100ms		
					touch_flag=0;
				}
		else if((LCD_X>99)&&(LCD_X<139)&&(LCD_Y>174)&&(LCD_Y<204))// 5
				{						
					key_value=5;
					speekers(100);//��������100ms		
					touch_flag=0;
				}
		else if((LCD_X>161)&&(LCD_X<201)&&(LCD_Y>174)&&(LCD_Y<204))//6
				{						
					key_value=6;
					speekers(100);//��������100ms		
					touch_flag=0;
				}
		else if((LCD_X>223)&&(LCD_X<263)&&(LCD_Y>174)&&(LCD_Y<204))// 7
				{						
					key_value=7;
					speekers(100);//��������100ms		
					touch_flag=0;
				}	
		else if((LCD_X>285)&&(LCD_X<325)&&(LCD_Y>174)&&(LCD_Y<204))// 8
				{						
					key_value=8;
					speekers(100);//��������100ms		
					touch_flag=0;
				}
		else if((LCD_X>99)&&(LCD_X<139)&&(LCD_Y>216)&&(LCD_Y<246))// 9
				{						
					key_value=9;
					speekers(100);//��������100ms		
					touch_flag=0;
				}

		else if((LCD_X>161)&&(LCD_X<201)&&(LCD_Y>216)&&(LCD_Y<246))// 0
				{						
					key_value=10;
					speekers(100);//��������100ms		
					touch_flag=0;
				}
		else if((LCD_X>223)&&(LCD_X<263)&&(LCD_Y>216)&&(LCD_Y<246))// ����
				{						
					key_value=11;
					speekers(100);//��������100ms		
					touch_flag=0;
				}
		else if((LCD_X>285)&&(LCD_X<325)&&(LCD_Y>216)&&(LCD_Y<246))// ɾ��
				{						
					key_value=12;
					speekers(100);//��������100ms		
					touch_flag=0;
				}
		else if((LCD_X>352)&&(LCD_X<400)&&(LCD_Y>125)&&(LCD_Y<185))// ȷ��
				{						
					key_value=13;
					speekers(100);//��������100ms		
					touch_flag=0;
				}	
		else if((LCD_X>352)&&(LCD_X<400)&&(LCD_Y>195)&&(LCD_Y<255))// ����
				{						
					key_value=14;
					speekers(100);//��������100ms		
					touch_flag=0;
				}			
		else
				{
					key_value=0;
					//speekers(100);//��������100ms		
					touch_flag=0;
				}
	}

}



//�������ý���
void menu_data(void)
{
	u8 i=1,j=0;
	u16 setyears;//�趨����
	u8 setmonth;//�趨��
	u8 setday;	//�趨��
	char setdata[8]={0};

	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(color_cyan);	
	Memory_Clear();	
	Delay1ms(100);//������ʾ����
		//��ʾͼƬ
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	JianPan_Displayfu();//��ʾ�������
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//�趨��ʾ����
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//������͸����
	Show_String("��������"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("�������ø�ʽ���磺20180907");
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) return;//�˳���ѭ��
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				setdata[j]=key_value-10+0x30;//����ֵת����ASCII���ݴ�
			else 
				setdata[j]=key_value+0x30;//����ֵת����ASCII���ݴ�			
			if(setdata[0]==0x30)//��һλΪ0
			{
				XY_Coordinate(90,100);//ָ����ʾλ��
				Show_String("Data setting Error!");				
				continue;
			}
			j++;
			if(j>8) 
			{
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("out of rang!");	
			Delay1ms(1000);	//��ʱ1S	
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("            ");	//�����ʾ
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
					XY_Coordinate(90+(j-1)*8,100);//ָ����ʾλ��
					Show_StringNum(&setdata[j-1],1);					
					break;
			case 11://����					
					for(j=0;j<8;j++) setdata[j]=0;
					j=0;
					XY_Coordinate(90,100);//ָ����ʾλ��
					Show_String("         ");
					key_value=0;
					break;
			case 12://ɾ��
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 3:
										XY_Coordinate(90+16,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 4:
										XY_Coordinate(90+24,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 5:
										XY_Coordinate(90+32,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 6:
										XY_Coordinate(90+40,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 7:
										XY_Coordinate(90+48,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 8:
										XY_Coordinate(90+56,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							default: break;	
						}
					
					break;
			case 13://ȷ��
					for(j=0;j<8;j++) setdata[j]-=0x30;
					setyears=setdata[0]*1000+setdata[1]*100+setdata[2]*10+setdata[3];
					setmonth=setdata[4]*10+setdata[5];
					if((setmonth>12)||(setmonth==0)) setmonth=1;//������Χ ��1
					setday=setdata[6]*10+setdata[7];
					if((setday>31)||(setday==0)) setday=1;//������Χ ��1
					RTC_Set(setyears,setmonth,setday,calendar.hour,calendar.min,calendar.sec);
// 					XY_Coordinate(360,147);//ȷ����
// 					Show_String("����");
// 					Delay1ms(100);	//��ʱ1S	
// 					XY_Coordinate(360,147);//ȷ����
// 					Show_String("ȷ��");
					i=0;//�˳���ѭ��
					key_value=0;
					break;
			case 14://����
					i=0;//�˳���ѭ��
					Graphic_Mode();	//�ָ�ͼƬģʽ
					key_value=0;
					break;
			default: break;			
		}
	}
}
//ʱ�����ý���
void menu_time(void)
{
	u8 i=1,j=0;
	u8 sethour;//�趨��ʱ
	u8 setmin;//�趨��
	u8 setsec;	//�趨��
	char settime[6]={0};

	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(color_cyan);	
	Memory_Clear();	
	Delay1ms(100);//������ʾ����
		//��ʾͼƬ
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	JianPan_Displayfu();//��ʾ�������
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//�趨��ʾ����
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//������͸����
	Show_String("ʱ������"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("ʱ�����ø�ʽ���磺15:08:07");
	XY_Coordinate(90,100);//ָ����ʾλ����ʾʱ���ʽ
	Show_String("  :  :  ");	
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) return;//�˳���ѭ��
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				settime[j]=key_value-10+0x30;//����ֵת����ASCII���ݴ�
			else 
				settime[j]=key_value+0x30;//����ֵת����ASCII���ݴ�			
			
			j++;
			if(j>6) 
			{
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("out of rang!");	
			Delay1ms(1000);	//��ʱ1S	
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("            ");	//�����ʾ
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
						XY_Coordinate(98+(j-1)*8,100);//ָ����ʾλ��
					else if(j>4)
						XY_Coordinate(106+(j-1)*8,100);//ָ����ʾλ��
					else	
						XY_Coordinate(90+(j-1)*8,100);//ָ����ʾλ��
					Show_StringNum(&settime[j-1],1);					
					break;
			case 11://����					
					for(j=0;j<6;j++) settime[j]=0;
					j=0;					
					XY_Coordinate(90,100);//ָ����ʾλ����ʾʱ���ʽ
					Show_String("  :  :  ");
					key_value=0;
					break;
			case 12://ɾ��
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 3:
										XY_Coordinate(98+16,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 4:
										XY_Coordinate(98+24,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 5:
										XY_Coordinate(138,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 6:
										XY_Coordinate(146,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
						
							default: break;	
						}
					
					break;
			case 13://ȷ��
					for(j=0;j<6;j++) settime[j]-=0x30;
					sethour=settime[0]*10+settime[1];
					if(sethour>24) sethour=1;//������Χ ��1
					setmin=settime[2]*10+settime[3];
					if(setmin>60) setmin=1;//������Χ ��1
					setsec=settime[4]*10+settime[5];
					if(setsec>60) setsec=1;//������Χ ��1
					RTC_Set(calendar.w_year,calendar.w_month,calendar.w_date,sethour,setmin,setsec);
					XY_Coordinate(360,147);//ȷ����
					Show_String("����");
					Delay1ms(1000);	//��ʱ1S	
					XY_Coordinate(360,147);//ȷ����
					Show_String("ȷ��");
					key_value=0;
					break;
			case 14://����
					i=0;//�˳���ѭ��
					Graphic_Mode();	//�ָ�ͼƬģʽ
					key_value=0;
					break;
			default: break;			
		}
	}
}
//�������ý���
void menu_password(void)
{
	u8 i=1,j=0;
	u32 password_data=0;
	char settime[6]={0};

	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(color_cyan);	
	Memory_Clear();	
	Delay1ms(100);//������ʾ����
		//��ʾͼƬ
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	JianPan_Displayfu();//��ʾ�������
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//�趨��ʾ����
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//������͸����
	Show_String("  ADУ׼"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("����������:");
	//XY_Coordinate(90,100);//ָ����ʾλ����ʾʱ���ʽ
	//Show_String("  :  :  ");	
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) return;//�˳���ѭ��
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				settime[j]=key_value-10+0x30;//����ֵת����ASCII���ݴ�
			else 
				settime[j]=key_value+0x30;//����ֵת����ASCII���ݴ�			
			
			j++;
			if(j>6) 
			{
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("out of rang!");	
			Delay1ms(1000);	//��ʱ1S	
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("            ");	//�����ʾ
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
					//	XY_Coordinate(98+(j-1)*8,100);//ָ����ʾλ��
					//else if(j>4)
					//	XY_Coordinate(106+(j-1)*8,100);//ָ����ʾλ��
					//else	
						XY_Coordinate(90+(j-1)*8,100);//ָ����ʾλ��
					Show_StringNum(&settime[j-1],1);					
					break;
			case 11://����					
					for(j=0;j<6;j++) settime[j]=0;
					j=0;
					XY_Coordinate(90,100);//ָ����ʾλ��
					Show_String("         ");
					key_value=0;
					break;
			case 12://ɾ��
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 3:
										XY_Coordinate(90+16,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 4:
										XY_Coordinate(90+24,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 5:
										XY_Coordinate(90+32,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 6:
										XY_Coordinate(90+40,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
						
							default: break;	
						}
					
					break;
			case 13://ȷ��
					for(j=0;j<6;j++) settime[j]-=0x30;
					password_data=settime[0]*100000+settime[1]*10000+settime[2]*1000+settime[3]*100+settime[4]*10+settime[5];
					if(password_data==password) //������ȷ �˳���ѭ��
					{
					password_flag=1;
					i=0;//�˳���ѭ��	
					Graphic_Mode();	//�ָ�ͼƬģʽ		
					key_value=0;
					}
					else//������� �����ʾ����
					{
						for(j=0;j<6;j++) settime[j]=0;
						j=0;
						XY_Coordinate(90,100);//ָ����ʾλ��
						Show_String("         ");
						key_value=0;
						XY_Coordinate(240,100);//ָ����ʾλ��
						Show_String("password error!");	
						Delay1ms(1000);	//��ʱ1S	
						XY_Coordinate(240,100);//ָ����ʾλ��
						Show_String("               ");	//�����ʾ
					}
					break;
			case 14://����
					i=0;//�˳���ѭ��
					Graphic_Mode();	//�ָ�ͼƬģʽ
					key_value=0;
					break;
			default: break;			
		}
	}
}
//�˵�ѡ�����
void menu_select(void)
{
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(0xFF);	
	Memory_Clear();	
	Delay1ms(100);//������ʾ����
		//��ʾͼƬ
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
  
	//��ʾ��ť   |ȷ�� ����|
	//DisplayButtonUp(20,215,115,255);//ȷ��
	//DisplayButtonUp(365,215,460,255);//����
	Put_AnJian(412,235,95,40);
	DisplayButtonUp(100,80,190,100);
	DisplayButtonUp(290,80,380,100);
	
	DisplayButtonUp(100,120,190,140);
	DisplayButtonUp(290,120,380,140);
	
	DisplayButtonUp(100,160,190,180);
	DisplayButtonUp(290,160,380,180);
	
		//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//�趨��ʾ����
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//������͸����
	Show_String("�˵�ѡ��"); 	
	Font_with_BackgroundColor();
	XY_Coordinate(375,220);
	Show_String("����");

	//��ʾ��������
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();
	Bold_Font();//����Ӵ�
	Active_Window(0,480,60,200);//�趨��ʾ����
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	XY_Coordinate(110,82);
	Show_String("��������");
	XY_Coordinate(110,122);
	Show_String("ʱ������");
	XY_Coordinate(110,162);
	Show_String("��ĻУ׼");
	
	XY_Coordinate(310,82);
	Show_String("ADУ׼");
	XY_Coordinate(310,122);
	Show_String("��  ӡ");
	XY_Coordinate(300,162);
	Show_String("���ݲ�ѯ");
	NoBold_Font();//ȡ������Ӵ�
	Graphic_Mode();
}

//�˵�����
void menu_menu(void)
{
	u8 flag=1;	
	menu_select();
	while(1)
	{		
		if(flag==0) return;//�˳���ѭ��		
		KeyScan();
		if(touch_flag)//������������
		{	
			if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//���ذ�ť������
				{						
					key_value=1;
					speekers(100);//��������100ms	
					touch_flag=0;
				}
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>80)&&(LCD_Y<100))//��������
				{						
					key_value=2;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>120)&&(LCD_Y<140))//ʱ������
				{						
					key_value=3;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>160)&&(LCD_Y<180))//��ĻУ׼
				{						
					key_value=4;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}	
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>80)&&(LCD_Y<100))//ADУ׼
				{						
					key_value=5;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}	
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>120)&&(LCD_Y<140))//��ӡ
				{						
					key_value=6;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>160)&&(LCD_Y<180))//���ݲ�ѯ
				{						
					key_value=7;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else
				{
					key_value=0;
					//speekers(100);//��������100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //����
							key_value = 0;
							flag=0;
							break;						
						case 2:////��������
							key_value = 0;
							menu_data();
						  menu_select();
							break;
						case 3://	//ʱ������								
							key_value = 0;
							menu_time();
							menu_select();
							break;
						case 4:////��ĻУ׼	
							key_value = 0;
							XY_Calibration_Function();				//������У׼							
							menu_select();
							break;
						case 5://	//ADУ׼	
							key_value = 0;
							menu_password();
							if(password_flag==1)
							{
								password_flag=0;
								AD_Calibration_Function();//ADУ׼����
							}
							menu_select();							
							break;
						case 6://	//��ӡ							
							menu_printselect();
							menu_select();
							key_value = 0;
							break;
						case 7://	//���ݲ�ѯ					
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
//���ݲ�ѯ����
void menu_dataQuery(void)
{
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(0xFF);	
	Memory_Clear();	
	Delay1ms(100);//������ʾ����
		//��ʾͼƬ
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
  
	//��ʾ��ť   |ȷ�� ����|
	//DisplayButtonUp(20,215,115,255);//ȷ��
	//DisplayButtonUp(365,215,460,255);//����
	Put_AnJian(412,235,95,40);
	DisplayButtonUp(180,80,328,100);	
	DisplayButtonUp(180,120,328,140);	
	DisplayButtonUp(180,160,328,180);
	
		//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//�趨��ʾ����
	XY_Coordinate(20,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//������͸����
	Show_String("���ݲ�ѯ"); 	
	Font_with_BackgroundColor();
	XY_Coordinate(375,220);
	Show_String("����");

	//��ʾ��������
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();
	Bold_Font();//����Ӵ�
	Active_Window(0,480,60,200);//�趨��ʾ����
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	 
	XY_Coordinate(190,82);
	Show_String("  ��ѯ��������  ");
	XY_Coordinate(190,122);
	Show_String("������β�������");
	XY_Coordinate(190,162);
	Show_String("������в�������");	
	
	NoBold_Font();//ȡ������Ӵ�
	Graphic_Mode();
}
//��ʾ���
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
//��ʾ����
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
// //��ʾ�Լ�
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
//��ʾͨ������
// x,y:ָ����ʾλ�� 
//xuhao����Ҫ��ʾ��������� 1-5
//flag:��� 1��ʾ���׾� 0��ʾƽ���׾�
void display_holesize(u16 x,u16 y,u8 xuhao)
{
	char Data[10];
	float a;
	if((data_once[checkTestnum].onedata[xuhao].once_gas_res[0]!=0xff)&&(data_once[checkTestnum].onedata[xuhao].once_gas_res[1]!=0xff)&&(data_once[checkTestnum].onedata[xuhao].once_gas_res[2]!=0xff)&&(data_once[checkTestnum].onedata[xuhao].once_gas_res[3]!=0xff))//���׾�	
		a=data_once[checkTestnum].onedata[xuhao].once_gas_res[0]*256+data_once[checkTestnum].onedata[xuhao].once_gas_res[1]+(float)(data_once[checkTestnum].onedata[xuhao].once_gas_res[2]*256+data_once[checkTestnum].onedata[xuhao].once_gas_res[3])/10000;	
	
	else
		return;
	sprintf(Data,"%6.2f    ",a);
	XY_Coordinate(x,y);
	Show_StringNum(Data,6);
}
//��ʾһ���е����ƽ������С�׾�
// x,y:ָ����ʾλ�� 
//xuhao����Ҫ��ʾ��������� 1-5
//flag:��� 1��ʾһ��ͨ�������е����
//          2��ʾһ��ͨ�������е�ƽ�� 
//          3��ʾһ��ͨ�������е���С
//          4��ʾһ��ѹ�����е����
//          5��ʾһ��ѹ�����е�ƽ��
//          6��ʾһ��ѹ�����е���С
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
	else if(flag==4)//һ��ѹ�����е����ֵ
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
//��ʾƽ��ѹǿ
// x,y:ָ����ʾλ�� 
//xuhao����Ҫ��ʾ��������� 1-5
//flag:��� 1��ʾ���ѹǿ 0��ʾƽ��ѹǿ
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

//��ʾ���в������ݵ�����
void display_testdata(void)
{
	//Graphic_Mode();		
	//LcdPrint8bitBmp(biaoge,0,0,479,195);
	show_excel();
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();	
	display_zunum();		
	display_riqi();	
	if(((checkTestnum>0||checkTestnum==0))&&(checkTestnum<10)&&(checkTestnum<test_zunum-1))
	{		
	//��ʾ��1�Ų�������		
	display_pa(181,41,0);//��һ���ѹ��
	display_holesize(350,41,0);//��һ���ͨ������
		
	//��ʾ��2�Ų�������	
	display_pa(181,60,1);
	display_holesize(350,60,1);
		
	//��ʾ��3�Ų�������
	display_pa(181,79,2);
	display_holesize(350,79,2);
	
	//��ʾ��4�Ų�������
	display_pa(181,98,3);
	display_holesize(350,98,3);	
	
	//��ʾ��5�Ų�������		
	display_pa(181,117,4);
	display_holesize(350,117,4);
		
	//һ���е����׾��е����ֵ
	display_holeinnum(181,136,4);//ƽ��ѹ��
	display_holeinnum(181,155,5);
	display_holeinnum(181,174,6);
	
	display_holeinnum(350,136,1);//ͨ������
	display_holeinnum(350,155,2);
	display_holeinnum(350,174,3);	
	}
	if(((checkTestnum>0||checkTestnum==0))&&(checkTestnum<10)&&(checkTestnum==test_zunum-1)&&(test_cishu!=1))
	{	
		if(test_cishu==2)//��һ������
		{			
			display_holesize(350,41,0);//��һ���ͨ������	
			display_pa(181,41,0);//��һ���ѹ��
			//һ���е����׾��е����ֵ
			display_holeinnum(181,136,4);//ƽ��ѹ��
			display_holeinnum(181,155,5);
			display_holeinnum(181,174,6);
			display_holeinnum(350,136,1);//ͨ������
			display_holeinnum(350,155,2);
			display_holeinnum(350,174,3);	
		}	
		else if(test_cishu==3)//��2������
		{		
			//��ʾ��1�Ų�������
			display_holesize(350,41,0);//��һ���ͨ������	
			display_pa(181,41,0);//��һ���ѹ��
			
			//��ʾ��2�Ų�������
			display_holesize(350,60,1);
			display_pa(181,60,1);
			//һ���е����׾��е����ֵ
			
			display_holeinnum(181,136,4);//ƽ��ѹ��
			display_holeinnum(181,155,5);
			display_holeinnum(181,174,6);
			display_holeinnum(350,136,1);//ͨ������
			display_holeinnum(350,155,2);
			display_holeinnum(350,174,3);	
		}
		else if(test_cishu==4)//��3������
		{			
		
			//��ʾ��1�Ų�������
			display_holesize(350,41,0);//��һ���ͨ������	
			display_pa(181,41,0);//��һ���ѹ��
			
			//��ʾ��2�Ų�������
			display_holesize(350,60,1);
			display_pa(181,60,1);
			
			//��ʾ��3�Ų�������
			display_holesize(350,79,2);
			display_pa(181,79,2);
			//һ���е����׾��е����ֵ
			display_holeinnum(181,136,4);//ƽ��ѹ��
			display_holeinnum(181,155,5);
			display_holeinnum(181,174,6);
			display_holeinnum(350,136,1);//ͨ������
			display_holeinnum(350,155,2);
			display_holeinnum(350,174,3);	
		}
		else if(test_cishu==5)//��4������
		{			
			//��ʾ��1�Ų�������
			display_holesize(350,41,0);//��һ���ͨ������	
			display_pa(181,41,0);//��һ���ѹ��
			
			//��ʾ��2�Ų�������
			display_holesize(350,60,1);
			display_pa(181,60,1);
			
			//��ʾ��3�Ų�������
			display_holesize(350,79,2);
			display_pa(181,79,2);
			//��ʾ��4�Ų�������
			display_holesize(350,98,3);	
			display_pa(181,98,3);
			
			//һ���е����׾��е����ֵ
			display_holeinnum(181,136,4);//ƽ��ѹ��
			display_holeinnum(181,155,5);
			display_holeinnum(181,174,6);
			display_holeinnum(350,136,1);//ͨ������
			display_holeinnum(350,155,2);
			display_holeinnum(350,174,3);		
		}	
	}	
		
}


//��ѯ���в������ݽ���
void check_datamenu(void)
{		
	Active_Window(0,479,0,271);
	Text_Background_Color(color_white);  
	Memory_Clear();
	//��ʾͼƬ
	Graphic_Mode();	
	LcdFillRec(0,200,480,272,1,color_cyan);
	//��ʾ��ť
// 	DisplayButtonUp(20,215,115,255);
// 	DisplayButtonUp(135,215,230,255);
// 	DisplayButtonUp(250,215,345,255);
// 	DisplayButtonUp(365,215,460,255);
	Put_AnJian(67,235,95,40);
	Put_AnJian(182,235,95,40);
	Put_AnJian(297,235,95,40);
	Put_AnJian(412,235,95,40);
	show_excel();

	
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();	
	XY_Coordinate(30,220);
	Show_String("ˢ��");
	XY_Coordinate(145,220);
	Show_String("<---");
	XY_Coordinate(260,220);
	Show_String("--->");
	XY_Coordinate(375,220);
	Show_String("����");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();		
	
}
void check_data(void)
{
	u8 j;
	static u8 flag;
	
	for(j=0;j<test_zunum;j++)
		SPI_Flash_Read(&data_once[j].Mark,j*256,58);	
	check_datamenu();//��ѯ���в������ݽ���	
	display_testdata();
	flag=1;
	while(flag)
	{
		KeyScan();
		if(touch_flag)//������������
		{	
			if((LCD_X>30)&&(LCD_X<105)&&(LCD_Y>215)&&(LCD_Y<255))//ˢ�°�ť������
				{						
					key_value=1;//���ð�����־	
					speekers(100);//��������100ms
					touch_flag=0;//��ǰ����Ѿ����������.	
				}
			else if((LCD_X>145)&&(LCD_X<220)&&(LCD_Y>215)&&(LCD_Y<255))//�Ϸ���ť������
				{				
					key_value=2;//	�˵�������־
					speekers(100);//��������100ms
					touch_flag=0;//��ǰ����Ѿ����������.	
				}
			else if((LCD_X>260)&&(LCD_X<335)&&(LCD_Y>215)&&(LCD_Y<255))//�·���ť������
				{						
					key_value=3;//	ֹͣ������־
					speekers(100);//��������100ms
					touch_flag=0;//��ǰ����Ѿ����������.
				}	
			else if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//���ذ�ť������
				{					
					key_value=4;//	���԰�����־
					speekers(100);//��������100ms					
					touch_flag=0;//��ǰ����Ѿ����������.	
				}
			else
				{
					key_value=0;
					//speekers(100);//��������100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //ˢ��											
							key_value = 0;
							checkTestnum=0;
							display_testdata();
							break;	
						case 2:  //�Ϸ�ҳ								
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
						case 3:  //�·�ҳ
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
						case 4: //����
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

//���ݲ�ѯ
void data_Query(void)
{
	u8 i,j=0,flag=1;
	u16 m,n;
	float a[5]={0};
	float b;
	menu_dataQuery();//���ݲ�ѯ����
	while(flag)
	{
		KeyScan();
		if(touch_flag)//������������
		{			
			if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//���ذ�ť������
				{						
					key_value=1;
					speekers(100);//��������100ms	
					touch_flag=0;
				}			
			else if((LCD_X>180)&&(LCD_X<328)&&(LCD_Y>80)&&(LCD_Y<100))//��ѯ��������
				{						
					key_value=2;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>180)&&(LCD_X<328)&&(LCD_Y>120)&&(LCD_Y<140))//������β�������
				{						
					key_value=3;
					for(i=0;i<5;i++) a[i]=0;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>180)&&(LCD_X<328)&&(LCD_Y>160)&&(LCD_Y<180))//������в�������
				{						
					key_value=4;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}			
			else
				{
					key_value=0;
					//speekers(100);//��������100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //����						
							flag=0;					
							key_value = 0;							
							break;	
						case 2:  //��ѯ��������
							key_value = 0;
							check_data();//��ѯ�������ݽ���
							menu_dataQuery();//���ݲ�ѯ����
							key_value = 0;					
							break;						
						case 3:  //������β�������							
							key_value = 0;							
							if((test_cishu==1)&&test_zunum>1)//��������һ������
							{
								test_cishu=6;
								test_zunum=test_zunum-1;
							}	
							SPI_Flash_Read(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,58);
							if((test_cishu>1)&&(test_cishu<7))//����Ĳ��Ի�û�в�����
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
								data_once[test_zunum-1].Mark=test_zunum;//���
								data_once[test_zunum-1].newyears[0]=calendar.w_year>>8;//��ĸ�λ
								data_once[test_zunum-1].newyears[1]=calendar.w_year;//��ĵ�λ
								data_once[test_zunum-1].newmonth=calendar.w_month;//��
								data_once[test_zunum-1].newday=calendar.w_date;//��			
								
								//һ����ͨ�����������ֵ��Сֵƽ��ֵ
								for(i=0;i<test_cishu-1;i++)
								{
									m=data_once[test_zunum-1].onedata[i].once_gas_res[0]<<8;
									m=m+data_once[test_zunum-1].onedata[i].once_gas_res[1];//��������
									n=data_once[test_zunum-1].onedata[i].once_gas_res[2]<<8;
									n=n+data_once[test_zunum-1].onedata[i].once_gas_res[3];
									b=m+(float)n/10000;
									a[i]=b;
								}
								bubble_sort(a,test_cishu-1);//ð�ݷ����� ��С����			
								//�׾����ֵa[4]
								m=a[test_cishu-2];
								n=(a[test_cishu-2]-m)*10000;
								data_once[test_zunum-1].Km_max[0]=m>>8;
								data_once[test_zunum-1].Km_max[1]=m;
								data_once[test_zunum-1].Km_max[2]=n>>8;
								data_once[test_zunum-1].Km_max[3]=n;
								//�׾�ƽ��ֵ
								b=0;
								b=(a[0]+a[1]+a[2]+a[3]+a[4])/(test_cishu-1);
								m=b;
								n=(b-m)*10000;
								data_once[test_zunum-1].Km_avg[0]=m>>8;
								data_once[test_zunum-1].Km_avg[1]=m;
								data_once[test_zunum-1].Km_avg[2]=n>>8;
								data_once[test_zunum-1].Km_avg[3]=n;
								//�׾���Сֵ
								m=a[0];
								n=(a[0]-m)*10000;
								data_once[test_zunum-1].Km_min[0]=m>>8;
								data_once[test_zunum-1].Km_min[1]=m;
								data_once[test_zunum-1].Km_min[2]=n>>8;
								data_once[test_zunum-1].Km_min[3]=n;
								/////////////////////////////////////////
								//һ����ѹ���������ֵ��Сֵƽ��ֵ
								for(i=0;i<test_cishu-1;i++)
								{
									m=data_once[test_zunum-1].onedata[i].once_kpa[0]<<8;
									m=m+data_once[test_zunum-1].onedata[i].once_kpa[1];//��������
									
									a[i]=m;
								}
								bubble_sort(a,test_cishu-1);//ð�ݷ����� ��С����			
								//�׾����ֵa[4]
								m=a[test_cishu-2];								
								data_once[test_zunum-1].Ka_max[0]=m>>8;
								data_once[test_zunum-1].Ka_max[1]=m;
								
								//�׾�ƽ��ֵ
								b=0;
								b=(a[0]+a[1]+a[2]+a[3]+a[4])/(test_cishu-1);
								m=b;								
								data_once[test_zunum-1].Ka_avg[0]=m>>8;
								data_once[test_zunum-1].Ka_avg[1]=m;
								
								//�׾���Сֵ
								m=a[0];								
								data_once[test_zunum-1].Ka_min[0]=m>>8;
								data_once[test_zunum-1].Ka_min[1]=m;
								
								//�ṹ���������
								for(i=test_cishu-1;i<5;i++)
								{
									data_once[test_zunum-1].onedata[i].once_kpa[0]=0xff;
									data_once[test_zunum-1].onedata[i].once_kpa[1]=0xff;
									
									data_once[test_zunum-1].onedata[i].once_gas_res[2]=0xff;
									data_once[test_zunum-1].onedata[i].once_gas_res[3]=0xff;
									data_once[test_zunum-1].onedata[i].once_gas_res[0]=0xff;
									data_once[test_zunum-1].onedata[i].once_gas_res[1]=0xff;
									
								}
								//���浽flash		
								AT24CXX_WriteOneByte(SAVE_ADDR_ZUSHU,test_zunum);
								AT24CXX_WriteOneByte(SAVE_ADDR_CISHU,test_cishu);								
								SPI_Flash_Write(&data_once[test_zunum-1].Mark,(test_zunum-1)*256,35);
							}
							break;		
						case 4: //������в�������
							SPI_Flash_Erase_Sector(0);//����0������
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


//ADУ׼����
void AD_Calibration_menu(void)
{
	char a[9]={0};
	
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(0xFF);	
	Memory_Clear();	
	Delay1ms(100);//������ʾ����
		//��ʾͼƬ
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
  
	//��ʾ��ť   | У׼ ����|		
	//DisplayButtonUp(20,215,115,255);//У׼
	//DisplayButtonUp(365,215,460,255);//����
	Put_AnJian(67,235,95,40);
	Put_AnJian(412,235,95,40);
	DisplayButtonUp(100,80,190,100);
	DisplayButtonUp(290,80,380,100);
	
	DisplayButtonUp(100,120,190,140);
	DisplayButtonUp(290,120,380,140);

	
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//�趨��ʾ����
	XY_Coordinate(40,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//������͸����
	Show_String("ADУ׼"); 	
	Font_with_BackgroundColor();
	XY_Coordinate(30,220);
	Show_String("У׼");	
	XY_Coordinate(375,220);
	Show_String("����");
	//��ʾ��������
	Horizontal_FontEnlarge_x1();
	Vertical_FontEnlarge_x1();
	Bold_Font();//����Ӵ�
	Active_Window(0,480,60,200);//�趨��ʾ����
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	
	XY_Coordinate(110,82);
	Show_String("�궨��1");
	XY_Coordinate(110,122);
	Show_String("AD1����");
	
	XY_Coordinate(310,82);
	Show_String("�궨��2");
	XY_Coordinate(310,122);
	Show_String("AD2����");
	
	//��ʾ�趨����
	XY_Coordinate(0,162);
	Show_String("�궨��1��");
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
	Show_String("�궨��2��");	
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
	NoBold_Font();//ȡ������Ӵ�
	//Graphic_Mode();
}
//�궨ϵ������
u16 biaoding_menu(void)
{
	u8 i=1,j=0;
	char biaoding[3]={0};
	u16 num=0;
	Clear_Full_Window();	
	Memory_Clear_with_Font_BgColor();
	Text_Background_Color(color_cyan);	
	Memory_Clear();	
	Delay1ms(100);//������ʾ����
		//��ʾͼƬ
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);	
	JianPan_Displayfu();//��ʾ�������
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//�趨��ʾ����
	XY_Coordinate(40,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//������͸����
	Show_String("��  ��"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("�궨�������ã�(0-500)pa");
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) break;//�˳���ѭ��
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				biaoding[j]=key_value-10+0x30;//����ֵת����ASCII���ݴ�
			else 
				biaoding[j]=key_value+0x30;//����ֵת����ASCII���ݴ�			
			
			j++;			
			if(j>3) 
			{
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("out of rang!");	
			Delay1ms(1000);	//��ʱ1S	
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("            ");	//�����ʾ	
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
					XY_Coordinate(90+(j-1)*8,100);//ָ����ʾλ��
					Show_StringNum(&biaoding[j-1],1);	
					if(j==3) 
					{
						//for(j=0;j<3;j++) 
							num=(biaoding[0]-0x30)*100+(biaoding[1]-0x30)*10+(biaoding[2]-0x30);
						
						if(num>500)
						{
							num=0;
							XY_Coordinate(240,100);//ָ����ʾλ��
							Show_String("out of rang!");	
							Delay1ms(1000);	//��ʱ1S	
							XY_Coordinate(240,100);//ָ����ʾλ��
							Show_String("            ");	//�����ʾ
							key_value=0;
							for(j=0;j<3;j++) biaoding[j]=0;
							j=0;		
							continue;
						}
					}
					break;
			case 11://����				
					for(j=0;j<3;j++) biaoding[j]=0;
					j=0;
					XY_Coordinate(90,100);//ָ����ʾλ��
					Show_String("         ");
					key_value=0;
					break;
			case 12://ɾ��
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 3:
										XY_Coordinate(9+16,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
// 							case 4:
// 										XY_Coordinate(90+24,100);//ָ����ʾλ��
// 										Show_StringNum(" ",1);
// 										j--;
// 										break;
// 							case 5:
// 										XY_Coordinate(90+32,100);//ָ����ʾλ��
// 										Show_StringNum(" ",1);
// 										j--;
// 										break;						
							default: break;	
						}
					
					break;
			case 13://ȷ��
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
					XY_Coordinate(360,147);//ȷ����
					Show_String("����");
					Delay1ms(100);	//��ʱ1S	
					XY_Coordinate(360,147);//ȷ����
					Show_String("ȷ��");					
					key_value=0;
					i=0;//�˳���ѭ��
					Graphic_Mode();	//�ָ�ͼƬģʽ					
					break;
			case 14://����
					i=0;//�˳���ѭ��
					Graphic_Mode();	//�ָ�ͼƬģʽ
					key_value=0;
					break;
			default: break;			
		}
	}
		return num;
}


//ADУ׼����
void AD_Calibration_Function(void)
{
	u8 flag1=0,flag11=0;
	static u8 flag_return;
	char value[5]={0};
	//u16 AD1value=0,AD2value=0,biaoding1=0,biaoding2=0;//AD����ֵ
	//u8 idata[16]={0};
	u8 b[16]={0};
	//��ȡ�����ѹ��ADϵ��
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
	
	AD_Calibration_menu();	//ADУ׼�˵�
	flag_return=1;
	while(1)
	{
		if(flag_return==0) return;//�˳���ѭ��
		KeyScan();
		if(touch_flag)//������������
		{	
			
			if((LCD_X>30)&&(LCD_X<105)&&(LCD_Y>215)&&(LCD_Y<255))//У׼��ť������
				{						
					key_value=1;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//���ذ�ť������
				{						
					key_value=2;
					speekers(100);//��������100ms	
					touch_flag=0;
				}			
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>80)&&(LCD_Y<100))//�궨��1����
				{						
					key_value=3;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>90)&&(LCD_X<200)&&(LCD_Y>120)&&(LCD_Y<140))//AD1����
				{						
					key_value=4;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>80)&&(LCD_Y<100))//�궨��2
				{						
					key_value=5;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}	
			else if((LCD_X>280)&&(LCD_X<390)&&(LCD_Y>120)&&(LCD_Y<140))//AD2����
				{						
					key_value=6;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}	
			else
				{
					key_value=0;
					//speekers(100);//��������100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //У׼���� ����ʾ�������Kֵ
							//press_k=(biaoding1-biaoding2)/(AD1-AD2)
						//press_c=((biaoding1+biaoding2)-press_k(AD1+AD2))/2
							Text_Foreground_Color(color_red);
							press_k=(float)(biaoding1-biaoding2)/(AD1value-AD2value);
							press_c=((float)(biaoding1+biaoding2)-press_k*(AD1value+AD2value))/2;
							pressAD_kint=press_k;
							pressAD_kfloat=(press_k-pressAD_kint)*10000;
							pressAD_cint=press_c;
							pressAD_cfloat=(press_c-pressAD_cint)*10000;
							//��ʾK
							sprintf(value,"%04d",pressAD_kint);
							XY_Coordinate(116,180);
							Show_StringNum(value,4);
							XY_Coordinate(148,180);
							Show_String(".");							
							XY_Coordinate(156,180);
							sprintf(value,"%04d",pressAD_kfloat);
							Show_StringNum(value,4);
						
							//��ʾC
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
							//����ϵ��K
							b[0]=pressAD_kint>>8;
							b[1]=pressAD_kint;
							b[2]=pressAD_kfloat>>8;
							b[3]=pressAD_kfloat;
							
							//����ϵ��C
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
						case 2:  //����							
							key_value = 0;							
							flag_return=0;//�˳���ѭ��
							Graphic_Mode();	
							break;						
						case 3:  //�궨��1							
							key_value = 0;							
							biaoding1=biaoding_menu();			
						
							AD_Calibration_menu();	//ADУ׼�˵�
							Text_Foreground_Color(color_red);
							sprintf(value,"%05d",biaoding1);
							XY_Coordinate(64,162);
							Show_StringNum(value,5);
							flag1=1;
							break;		
						case 4:  //AD1����
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
						case 5:  //�궨��2								
							key_value = 0;	
							biaoding2=biaoding_menu();
							AD_Calibration_menu();	//ADУ׼�˵�
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
	//��ʾͼƬ
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdFillRec(0,60,480,200,1,color_white);	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
	
	//DisplayButtonUp(20,215,115,255);//��ӡ
	//DisplayButtonUp(365,215,460,255);//����
	Put_AnJian(67,235,95,40);
	Put_AnJian(412,235,95,40);
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//�趨��ʾ����
	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();	
	XY_Coordinate(40,15);	
	Font_with_BackgroundTransparency();//������͸����
	Show_String("��  ӡ");
	XY_Coordinate(30,220);
	Show_String("��ӡ");
	XY_Coordinate(375,220);
	Show_String("����");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	Font_with_BackgroundColor();
	
	Active_Window(0,480,60,200);//�趨��ʾ����
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
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
	Show_String("ѡ���ӡ�ڼ������ݣ�");
	sprintf(data,"%d",slect_print);
	XY_Coordinate(330,96);
	Show_StringNum(data,2);
	XY_Coordinate(160,148);
	Show_String("ѡ���ӡ����������!");
}



//��ӡѡ����������
void menu_printselect(void)
{	
	u8 flag=1;	
	menu_printselect1();
	while(flag)
	{
		KeyScan();
		if(touch_flag)//������������
		{			
			if((LCD_X>375)&&(LCD_X<450)&&(LCD_Y>215)&&(LCD_Y<255))//���ذ�ť������
				{						
					key_value=1;
					speekers(100);//��������100ms	
					touch_flag=0;
				}			
			else if((LCD_X>30)&&(LCD_X<105)&&(LCD_Y>215)&&(LCD_Y<255))//��ӡ��ť
				{						
					key_value=2;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}
			else if((LCD_X>160)&&(LCD_X<320)&&(LCD_Y>90)&&(LCD_Y<118))//ѡ���ӡ�ڼ�������
				{						
					key_value=3;
					speekers(100);//��������100ms	
					touch_flag=0;
					XY_Coordinate(150,96);
					Show_Cursor();
					XY_Coordinate(150,148);
					Del_Cursor();	
				}
			else if((LCD_X>160)&&(LCD_X<320)&&(LCD_Y>142)&&(LCD_Y<170))//ѡ���ӡ���в�������
				{	
					XY_Coordinate(150,96);
					Del_Cursor();
					XY_Coordinate(150,148);
					Show_Cursor();	
					key_value=4;
					speekers(100);//��������100ms	
					touch_flag=0;	
				}			
			else
				{
					key_value=0;
					//speekers(100);//��������100ms		
					touch_flag=0;
				}	
		}
		else
		{
			if(key_value!=0)
				{					
					switch(key_value)
					{
						case 1:  //����						
							flag=0;					
							key_value = 0;
							printf_flag=1;
							break;	
						case 2:  //��ӡ��������
							key_value = 0;
							PrintReport();
							menu_printselect1();	
							break;						
						case 3:  //ѡ���ӡ�ڼ�������							
							key_value = 0;
							printf_flag=1;		
							set_printnum();
							break;		
						case 4: //ѡ���ӡ���в�������							
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
	Delay1ms(100);//������ʾ����
		//��ʾͼƬ
	Graphic_Mode();	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);	
	JianPan_Displayfu();//��ʾ�������
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,272);//�趨��ʾ����
	XY_Coordinate(40,15);
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();
	Font_with_BackgroundTransparency();//������͸����
	Show_String("��  ӡ"); 	
	Font_with_BackgroundColor();
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	XY_Coordinate(90,70);
	Show_String("���ô�ӡ�ڼ�������");
	Font_with_BackgroundColor();
	while(1)
	{
		if(i==0) return;//�˳���ѭ��
		ScanKeyBoard();
		if((key_value>0)&&(key_value<11))
		{
			if(key_value==10)
				data[j]=key_value-10+0x30;//����ֵת����ASCII���ݴ�
			else 
				data[j]=key_value+0x30;//����ֵת����ASCII���ݴ�			
			
			j++;
			if(j>2) 
			{
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("out of rang!");	
			Delay1ms(1000);	//��ʱ1S	
			XY_Coordinate(240,100);//ָ����ʾλ��
			Show_String("            ");	//�����ʾ
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
					XY_Coordinate(90+(j-1)*8,100);//ָ����ʾλ��
					Show_StringNum(&data[j-1],1);					
					break;
			case 11://����					
					for(j=0;j<2;j++) data[j]=0;
					j=0;
					XY_Coordinate(90,100);//ָ����ʾλ��
					Show_String("         ");
					key_value=0;
					break;
			case 12://ɾ��
					key_value=0;
					switch(j)
						{
							case 1:
										XY_Coordinate(90,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;
							case 2:
										XY_Coordinate(90+8,100);//ָ����ʾλ��
										Show_StringNum(" ",1);
										j--;
										break;						
							default: break;	
						}
					
					break;
			case 13://ȷ��
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
					
					i=0;//�˳���ѭ��	
					Graphic_Mode();	//�ָ�ͼƬģʽ		
					key_value=0;
					menu_printselect1();	
					}
					else//������� �����ʾ����
					{
						for(k=0;k<2;k++) data[k]=0;
						j=0;
						XY_Coordinate(90,100);//ָ����ʾλ��
						Show_String("         ");
						key_value=0;
						XY_Coordinate(240,100);//ָ����ʾλ��
						Show_String("slect error!");	
						Delay1ms(1000);	//��ʱ1S	
						XY_Coordinate(240,100);//ָ����ʾλ��
						Show_String("            ");	//�����ʾ
					}
					break;
			case 14://����
					i=0;//�˳���ѭ��
					Graphic_Mode();	//�ָ�ͼƬģʽ
					menu_printselect1();
					key_value=0;
					break;
			default: break;			
		}
	}
}

//��ӡ�ȴ�����
void menu_print(void)
{
	Active_Window(0,479,0,271);	
	//��ʾͼƬ
	Graphic_Mode();
	LcdFillRec(0,0,480,60,1,color_cyan);
	LcdFillRec(0,200,480,272,1,color_cyan);
	LcdFillRec(0,60,480,200,1,color_white);	
	LcdPrint8bitBmp(gImage_222,5,10,174,40);
//	DisplayButtonUp(365,215,460,255);
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_blue);
	Text_Background_Color(color_cyan);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,60);//�趨��ʾ����
	
	Horizontal_FontEnlarge_x2();
  Vertical_FontEnlarge_x2();	
	XY_Coordinate(40,15);	
	Font_with_BackgroundTransparency();//������͸����
	Show_String("��  ӡ");	
	//XY_Coordinate(375,220);
	//Show_String("����");
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	Font_with_BackgroundColor();
	
	Active_Window(0,480,60,200);//�趨��ʾ����
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	XY_Coordinate(110,110);
	Show_String("���ڴ�ӡ��,�����ĵȴ���ӡ���!");
}
//������
void show_excel(void)
{
	Active_Window(0,479,0,271);
	Graphic_Mode();
	LcdFillRec(0,0,480,200,1,color_white);
	//������
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
	//����ֱ��	
	LcdPrintVert(115,21,179,color_black);
	LcdPrintVert(290,21,179,color_black);
	//��ʾ����
	Text_Mode();
	Text_Foreground_Color(color_black);
	Text_Background_Color(color_white);//�趨���ֵ�ǰ��ɫ�ͱ���ɫ	
	External_CGROM();
	External_CGROM_GB();
	Active_Window(0,480,0,200);//�趨��ʾ����		
	Horizontal_FontEnlarge_x1();
  Vertical_FontEnlarge_x1();
	Font_with_BackgroundTransparency();//������͸����
	Font_with_BackgroundColor();
	XY_Coordinate(35,3);
	Show_String("������ţ�");	
	XY_Coordinate(315,3);
	Show_String("�������ڣ�");
	
	XY_Coordinate(40,22);
	Show_String("���");
	XY_Coordinate(165,22);
	Show_String("ѹ��(Pa)");
	XY_Coordinate(310,22);
	Show_String("ͨ������(Pa/cm2)");
	
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
//��ӡ����
void PrintReport(void)
{	
    u8 i,j,k,m;
		u16 datayear,b;
		char Data[20]={0};
		float a;
		menu_print();//��ӡ�����еĽ���
		//��ȡ����
		for(i=0;i<test_zunum;i++)
			SPI_Flash_Read(&data_once[i].Mark,i*256,58);	
    pprint(0x1b);                                                                                      
    pprint(0x40);
		pprint(0x1b);                                                                                      
    pprint(0x51);
		pprint(0x00);
		if(printf_flag==1)//��ӡ��ѡ�������
		{
		m=slect_print-1;
		k=slect_print;	
		}
		else if(printf_flag==2)//��ӡ������Ĳ�������
		{
			if(test_cishu>1)       k=test_zunum;
			else if(test_cishu==1) k=test_zunum-1;
			m=0;
		}
		
		for(i=m;i<k;i++)
		{		
			//5#����
			for(j=5;j>0;j--)
			{	
				b=data_once[i].onedata[j-1].once_kpa[0]*256+data_once[i].onedata[j-1].once_kpa[1];	
				if(b!=0xffff)
				{//��ʾѹ����
					PrintStr("  P=");
					sprintf(Data,"%04d",b);
					PrintStrNnum(Data,4);
					PrintStr("Pa");
					pprint(0x0d);  //�س�
					pprint(0x0a); //����
					//��ʾͨ������
					sprintf(Data,"%01d",j);
					PrintStrNnum(Data,1);
					PrintStr("#");
					PrintStr("G=");
					a=data_once[i].onedata[j-1].once_gas_res[0]*256+data_once[i].onedata[j-1].once_gas_res[1]+(float)(data_once[i].onedata[j-1].once_gas_res[2]*256+data_once[i].onedata[j-1].once_gas_res[3])/10000;
					sprintf(Data,"%5.2f",a);
					PrintStrNnum(Data,5);
					PrintStr("Pa/cm2");
					pprint(0x0d);  //�س�
					pprint(0x0a); //����
					
				}	
			}	
			a=data_once[i].Ka_max[0]*256+data_once[i].Ka_max[1];
			if(a!=0xffff)
			{	
			//---------------------
			PrintStr("---------------");
			pprint(0x0d);  //�س�
			pprint(0x0a); //����			
			//ka_max
			PrintStr("PMax=");
			b=data_once[i].Ka_max[0]*256+data_once[i].Ka_max[1];	
			sprintf(Data,"%04d",b);
			PrintStrNnum(Data,4);
			PrintStr("Pa");
			pprint(0x0d);  //�س�
			pprint(0x0a); //����
			//ka_avg
			PrintStr("PAvg=");
			b=data_once[i].Ka_avg[0]*256+data_once[i].Ka_avg[1];
			sprintf(Data,"%04d",b);
			PrintStrNnum(Data,4);
			PrintStr("Pa");
			pprint(0x0d);  //�س�
			pprint(0x0a); //����
			//ka_min
			PrintStr("PMin=");
			b=data_once[i].Ka_min[0]*256+data_once[i].Ka_min[1];
			sprintf(Data,"%04d",b);
			PrintStrNnum(Data,4);
			PrintStr("Pa");
			pprint(0x0d);  //�س�
			pprint(0x0a); //����
			//Km_max
			PrintStr("GMax=");
			a=data_once[i].Km_max[0]*256+data_once[i].Km_max[1]+(float)(data_once[i].Km_max[2]*256+data_once[i].Km_max[3])/10000;
			sprintf(Data,"%5.2f",a);
			PrintStrNnum(Data,5);
			PrintStr("Pa/cm2");
			pprint(0x0d);  //�س�
			pprint(0x0a); //����
			//Km_avg
			PrintStr("GAvg=");
			a=data_once[i].Km_avg[0]*256+data_once[i].Km_avg[1]+(float)(data_once[i].Km_avg[2]*256+data_once[i].Km_avg[3])/10000;
			sprintf(Data,"%5.2f",a);
			PrintStrNnum(Data,5);
			PrintStr("Pa/cm2");
			pprint(0x0d);  //�س�
			pprint(0x0a); //����
			//Km_min
			PrintStr("GMin=");
			a=data_once[i].Km_min[0]*256+data_once[i].Km_min[1]+(float)(data_once[i].Km_min[2]*256+data_once[i].Km_min[3])/10000;
			sprintf(Data,"%5.2f",a);
			PrintStrNnum(Data,5);
			PrintStr("Pa/cm2");
			pprint(0x0d);  //�س�
			pprint(0x0a); //����
			//---------------------
			PrintStr("---------------");
			pprint(0x0d);  //�س�
			pprint(0x0a); //����
			//����
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
			pprint(0x0d);  //�س�
			pprint(0x0a); //����
			
			//���
			PrintStr("NO:");
			sprintf(Data,"%02d",(i+1));			
			PrintStrNnum(Data,2);	
			pprint(0x0d);  //�س�
			pprint(0x0a); //����
		}
	} 
	pprint(0x0d);  //�س�
	pprint(0x0a); //����
	pprint(0x0d);  //�س�
	pprint(0x0a); //����
 
		 
}	
//������������ƽ��ֵ  times:�������ݵĸ��� 

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


