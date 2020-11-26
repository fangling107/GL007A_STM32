#include "touch.h" 
#include "24cxx.h"
#include "ra8870.h"
#include "stdlib.h"
#include "math.h"

//==============================================
u16 X1,Y1,X2,Y2,X3,Y3,X4,Y4;//校准点参数
u16 LCD_X,LCD_Y;		//实际坐标值
u16 ADC_value_X,ADC_value_Y;//物理坐标值
float xfac;					
float yfac;
short xoff;
short yoff;
u8 sta;//按键状态//b7:按下1/松开0; 
	                     //b6:0,没有按键按下;1,有按键按下.
u8 touch_flag;	
u8 keystatus;
//保存校准参数										    
void TP_Save_Adjdata(void)
{
	s32 temp;			 
	//保存校正结果!		   							  
	temp=xfac*100000000;//保存x校正因素      
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE,temp,4);   
	temp=yfac*100000000;//保存y校正因素    
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+4,temp,4);
	//保存x偏移量
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+8,xoff,2);		    
	//保存y偏移量
	AT24CXX_WriteLenByte(SAVE_ADDR_BASE+10,yoff,2);	
	//保存触屏类型
	//AT24CXX_WriteOneByte(SAVE_ADDR_BASE+12,touchtype);	
	temp=0X0A;//标记校准过了
	AT24CXX_WriteOneByte(SAVE_ADDR_BASE+13,temp); 
}

//得到保存在EEPROM里面的校准值
//返回值：1，成功获取数据
//        0，获取失败，要重新校准
u8 TP_Get_Adjdata(void)
{					  
	s32 tempfac;
	tempfac=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+13);//读取标记字,看是否校准过！ 		 
	if(tempfac==0X0A)//触摸屏已经校准过了			   
	{    												 
		tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE,4);		   
		xfac=(float)tempfac/100000000;//得到x校准参数
		tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+4,4);			          
		yfac=(float)tempfac/100000000;//得到y校准参数
	    //得到x偏移量
		xoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+8,2);			   	  
 	    //得到y偏移量
		yoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+10,2);				 	  
 		//tp_dev.touchtype=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+12);//读取触屏类型标记				 
		return 1;	 
	}
	return 0;
}	

//bit touch_valid = 0;
//Subroutine:	Detect_TP_X_Y coordinate 
//==============================================================================
u8 ADC_X(void)
{	u8 temp;
	LCD_REG=(0x72);//TPXH	 X_coordinate high byte
	//Chk_Busy();
	temp=LCD_RAM;
	return temp;
}

u8 ADC_Y(void)
{	u8 temp;
	LCD_REG=(0x73);//TPYH	  Y_coordinate high byte
	//Chk_Busy();
	temp=LCD_RAM;
	return temp;
}

u8 ADC_XY(void)
{	u8 temp;
 	LCD_REG=(0x74);//TPXYL	  bit[3:2] Y_coordinate low byte  bit[1:0] X_coordinate low byte 
	temp=LCD_RAM;
	return temp;
} 
//获取X的AD值
u16 Get_X_ADC_Value_10bit(void)
{
   u16 temp,temp1;
   
   temp=0;
   temp|=ADC_X();
   temp1=(temp<<2);
   temp= temp1|((ADC_XY()&0x03));
   
   return temp;   
}

//获取Y的AD值
u16 Get_Y_ADC_Value_10bit(void)
{
   u16 temp,temp1;
   
   temp=0;
   temp|=ADC_Y();
   temp1=(temp<<2);
   temp=temp1|((ADC_XY()>>2)&0x03);
   
   return temp;   
}
//REG[80h]~REG[83h] 图形光标位置
void Graphic_Cursor_Coordinate(u16 X,u16 Y)
{
    LCD_REG=(0x80);
	LCD_RAM=(X);  
    LCD_REG=(0x81);	   
	LCD_RAM=(X>>8);
  
    LCD_REG=(0x82);
	LCD_RAM=(Y);  
    LCD_REG=(0x83);	   
	LCD_RAM=(Y>>8);
}
//取X2 X3 X4的中间值 复制给X1 再将ADC_value_X = X1; ADC_value_Y = Y1;
void MidValueCalculate(void)
{
	if (X2>X3)
	{
		if (X3>X4)
		{X1=X3;}	
		else
		{
		   if (X2>X4)
			{ X1=X4;}
			else	
			{ X1=X2;}
		}		
	}			
 else
	{
		if (X3<X4)
		{ X1=X3;}	
		else
		{
			if (X2<X4)
			{ X1=X4;}
			else	
			{ X1=X2; }
		}	
	}
			
 if (Y2>Y3)
	{
		if (Y3>Y4)
		{Y1=Y3;}	
		else
		{
		   if (Y2>Y4)
			{ Y1=Y4;}
			else	
			{ Y1=Y2;}
		}		
	}			
 else
	{
		if (Y3<Y4)
		{ Y1=Y3;}	
		else
		{
			if (Y2<Y4)
			{ Y1=Y4;}
			else	
			{ Y1=Y2; }
		}									
   }

 	  ADC_value_X = X1;
    ADC_value_Y = Y1;

 }
 

// TP controller application function

void Access_TP_coordinate(u8 tp)	//tp=1 转换实际坐标值 tp=0不进行转换
{

 clear_TP_interrupt();
 Delay1ms(100);  //delay after clear_TP_interrupt		 
 if(Touch_Status())
  {	 			
   X2=Get_X_ADC_Value_10bit();  
   Y2=Get_Y_ADC_Value_10bit();   
	 clear_TP_interrupt(); 
   Delay1ms(10);  //delay after clear_TP_interrupt
   if(Touch_Status())
      { 				
       X3=Get_X_ADC_Value_10bit();
       Y3=Get_Y_ADC_Value_10bit();    
			 clear_TP_interrupt();
			 Delay1ms(10);  //delay after clear_TP_interrupt

       if(Touch_Status())//有按键按下
         {					
          X4 =Get_X_ADC_Value_10bit();
          Y4 =Get_Y_ADC_Value_10bit();										 
          MidValueCalculate();    //取中间值，并赋值给 ADC_value_X ADC_value_Y   
          if(tp) Get_XY_Calibration(); 	//得到实际坐标值	
					touch_flag=1;	
         }	
				else 
				 {					
					ADC_value_X=0; ADC_value_Y=0;
					LCD_X=0;LCD_Y=0;					
				 }
     }
     else
     {
   	  ADC_value_X=0; ADC_value_Y=0;
   	  LCD_X=0;LCD_Y=0;	
						 
   	 }
		
	 }	   
	else
	 {
		ADC_value_X=0; ADC_value_Y=0;
		LCD_X=0;LCD_Y=0;
		
	 }
	
}

	//触摸按键扫描
//tp:0,屏幕坐标;1,物理坐标(校准等特殊场合用)
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
u8 TP_Scan(void)
{			   
	if(Touch_Status())//有按键按下
	{
		Access_TP_coordinate(0);//进行按键扫描,读取物理坐标
		if((sta&TP_PRES_DOWN)==0)//之前没有被按下
		{		 
			sta=TP_PRES_DOWN|TP_CATH_PRES;//按键按下  				   			 
		}			   
	}else
	{
		if(sta&TP_PRES_DOWN)//之前是被按下的
		{
			sta&=~(1<<7);//标记按键松开	
		}
		else//之前就没有被按下
		{			
			sta&=0XE0;	//清除点有效标记		
		}	    
	}
	return sta&TP_PRES_DOWN;//返回当前的触屏状态
}	


//显示XY的物理AD值
void Trace_XY_Vaule(void)
{	  
      u8 temp;
	  u16 temp1;

	  temp1=ADC_value_X;
	  temp=(ADC_value_X>>8);	  		
	  XY_Coordinate(0,60);  //设置文字显示位置
	  Show_String("X=0x");  
	  Printf_Hex(temp);		
	  temp=temp1;
	  Printf_Hex(temp);	
    Chk_Busy();

	  temp1=ADC_value_Y;
	  temp=(ADC_value_Y>>8);
		XY_Coordinate(0,100);  //设置文字显示位置
	  Show_String("Y=0x");
	  Printf_Hex(temp);		
	  temp=temp1;
	  Printf_Hex(temp);
	  Chk_Busy();
}


//================================================
//物理值转换为实际坐标值
//================================================

void Get_XY_Calibration(void)
{
		s16 temp1=0;
		temp1=xfac*ADC_value_X;
		LCD_X = temp1+xoff;
		
		temp1=yfac*ADC_value_Y;
		LCD_Y = temp1+yoff;
}

//==============================================================================
//Subroutine:	Show Hex	
//==============================================================================
void Printf_Hex(u8 buf)
{
	u8 temp;
	temp=buf;
	temp = (temp >>4) & 0x0F;
	if(temp < 0x0A)
	{temp |= 0x30;
	}
	else temp = temp + 0x37;
	LCD_RAM=(temp);
  Chk_Busy();

	temp=buf;
	temp = temp & 0x0F;
	if(temp < 0x0A)
	{temp |= 0x30;
	}
	else temp = temp + 0x37;
	LCD_RAM=(temp);
  Chk_Busy();
}
 //输出显示坐标值
void Trace_coordinate(void)
{
			  		
	  XY_Coordinate(0,140);  //设置文字显示位置
	  Show_String("LCD_X=");		//显示字符
		Print_Decimal(LCD_X);
		
	  XY_Coordinate(0,180);  //设置文字显示位置	  
	  Show_String("LCD_Y=");
	  Print_Decimal(LCD_Y); 
		Graphic_Mode();
}	

//16进制转换十进制输出
void Print_Decimal(u16 buffer)
{
	u16 temp,temp1;
     
    temp=buffer/100;
    temp1= buffer%100;

    temp|=0x30;
		LCD_RAM=(temp);
		Chk_Busy();
  
    temp=temp1/10;	
		temp|=0x30;
    LCD_RAM=(temp);
		Chk_Busy();

    temp=temp1%10;	
		temp|=0x30;
    LCD_RAM=(temp);
		Chk_Busy();

}

//校准函数
/*
void XY_Calibration_Function(void)
{
   u8 state =0;
   Active_Window(0,479,0,271);
   Text_Background_Color(color_white);  
   Memory_Clear();
   Chk_Busy();

   Text_Foreground_Color(color_blue);
   Circle_Coordinate_Radius(Test_Dot1_X,Test_Dot1_Y,1);	//画一个实心圆点
   Draw_circle_fill();
   Chk_Busy();

   Text_Foreground_Color(color_red);// 
   Circle_Coordinate_Radius(Test_Dot1_X,Test_Dot1_Y,4);//画点的外边框	
   Draw_circle();
   Chk_Busy();
   Text_Foreground_Color(color_red);// 
   Circle_Coordinate_Radius(Test_Dot1_X,Test_Dot1_Y,5);//再画一个大的外边框	
   Draw_circle();
   Chk_Busy();


   // 对第一个点进行校准  
	 state=1;
		do{ 
				Access_TP_coordinate(); 				//取中间值，并赋值给 ADC_value_X ADC_value_Y  
				if((X1>(Xa-20))&&(X1<(Xa+20))) 
				{
					if((Y1>(Ya-20))&&(Y1<(Ya+20)))
					{
						XVa=X1; YVa=Y1; state=0;	//判断误差范围，退出循环
					}
				}			 
			 //Trace_XY_Vaule();							//显示XY的物理AD值
			// Trace_coordinate();
			 Delay1ms(200);		
      }while(state); 

   //========================================================//    
 
   Active_Window(0,479,0,271);   
   Text_Background_Color(color_white);  
   Memory_Clear();
   Chk_Busy();
 

   Text_Foreground_Color(color_blue);
   Circle_Coordinate_Radius(Test_Dot2_X,Test_Dot2_Y,1);	
   Draw_circle_fill();
   Chk_Busy();

   Text_Foreground_Color(color_red);// 
   Circle_Coordinate_Radius(Test_Dot2_X,Test_Dot2_Y,4);	
   Draw_circle();
   Chk_Busy();
   Circle_Coordinate_Radius(Test_Dot2_X,Test_Dot2_Y,5);	
   Draw_circle();
   Chk_Busy();

   ///
	 state=1;
		do{ 

					Access_TP_coordinate(); 
          if((X1>(Xb-20))&&(X1<(Xb+20)))  
          {
						if((Y1>(Yb-20))&&(Y1<(Yb+20)))
						{
							XVb=X1; YVb=Y1; state=0;							
						}
          }
           Delay1ms(200);				
          }
         while(state);	
				OFFSET_X = (Xa-XVa)+(Xb-XVb); //―X熬簿
				OFFSET_X /= 2;

				OFFSET_Y = (Ya-YVa)+(Yb-YVb); //―Y熬簿
				OFFSET_Y /= 2;	
				TP_Save_Adjdata();					
}*/
//指定位置画校准点
void drawpoint(u16 x,u16 y)
{			
	 Text_Foreground_Color(color_blue);
	 Circle_Coordinate_Radius(x,y,1);	//画一个实心圆点
	 Draw_circle_fill();
	 Chk_Busy();

	 Text_Foreground_Color(color_red);// 
	 Circle_Coordinate_Radius(x,y,4);//画点的外边框	
	 Draw_circle();
	 Chk_Busy();
	 Text_Foreground_Color(color_red);// 
	 Circle_Coordinate_Radius(x,y,5);//再画一个大的外边框	
	 Draw_circle();
	 Chk_Busy();
}

void XY_Calibration_Function(void)
{
	u16 outtime=0;
	u16 pos_temp[4][2];//坐标缓存值
	u8  cnt=0;
	u16 d1,d2;
	u32 tem1,tem2;
	double fac;

	//清屏
	Active_Window(0,479,0,271);
	Text_Background_Color(color_white);  
	Memory_Clear();
	Chk_Busy();	
	drawpoint(20,20);//(20,20)的位置画第一个校准点
	while(1)
	{
		outtime=0;
		TP_Scan();//进行触摸扫描
		if((sta&0xc0)==TP_CATH_PRES)//有效触摸
		{
					
			pos_temp[cnt][0]=X1;
			pos_temp[cnt][1]=Y1;			
			cnt++;		
			sta&=~(1<<6);//标记按键已经被处理过了.
			switch(cnt)
			{
				case 1:						 
					Active_Window(0,479,0,271);
					Text_Background_Color(color_white);  
					Memory_Clear();
					Chk_Busy();				//清除点1 
					drawpoint(460,20);	//画点2
					
					break;
				case 2:
					Active_Window(0,479,0,271);
					Text_Background_Color(color_white);  
					Memory_Clear();
					Chk_Busy();				//清除点2 
					drawpoint(20,252);	//画点3
					break;
				case 3:
					Active_Window(0,479,0,271);
					Text_Background_Color(color_white);  
					Memory_Clear();
					Chk_Busy();				//清除点3 
					drawpoint(460,252);	//画点4
					break;
				case 4://全部四个点都已经得到  //对边相等
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,2的距离
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到3,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0)//不合格
					{
						cnt=0;
 				    Active_Window(0,479,0,271);
						Text_Background_Color(color_white);  
						Memory_Clear();
						Chk_Busy();				//清除点4 
   	 				drawpoint(20,20);//(20,20)的位置画第一个校准点
 						//显示四个点的坐标信息
 						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,3的距离
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
 				    Active_Window(0,479,0,271);
						Text_Background_Color(color_white);  
						Memory_Clear();
						Chk_Busy();				//清除点4 
   	 				drawpoint(20,20);//(20,20)的位置画第一个校准点
 						//显示数据   
						continue;
					}
					//正确了
					//对角线相等
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,4的距离
	
					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,3的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
 				    Active_Window(0,479,0,271);
						Text_Background_Color(color_white);  
						Memory_Clear();
						Chk_Busy();				//清除点4 
   	 				drawpoint(20,20);//(20,20)的位置画第一个校准点
 						//TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//正确了
					//计算结果
					xfac=(float)(480-40)/(pos_temp[1][0]-pos_temp[0][0]);//得到xfac	X2-X1	 
					xoff=(480-xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//得到xoff
						  
					yfac=(float)(272-40)/(pos_temp[2][1]-pos_temp[0][1]);//得到yfac
					yoff=(272-yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//得到yoff 
					if(abs(xfac)>2||abs(yfac)>2)//触屏和预设的相反了.
					{
						cnt=0;
 				    Active_Window(0,479,0,271);
						Text_Background_Color(color_white);  
						Memory_Clear();
						Chk_Busy();				//清除点4 
   	 				drawpoint(20,20);//(20,20)的位置画第一个校准点
						//LCD_ShowString(40,26,lcddev.width,lcddev.height,16,"TP Need readjust!");								    
						continue;
					}
					Active_Window(0,479,0,271);
					Text_Background_Color(color_white);  
					Memory_Clear();				
					Delay1ms(1000);
					Text_Mode();
					Text_Foreground_Color(color_blue);
					Text_Background_Color(color_cyan);//设定文字的前景色和背景色	
					External_CGROM();
					External_CGROM_GB();
					Active_Window(0,480,0,60);//设定显示区域
					XY_Coordinate(100,170);
					Bold_Font();//字体加粗
					Show_String("Touch Screen Adjust OK!");
					NoBold_Font();//取消字体加粗						
					Graphic_Mode();				
					TP_Save_Adjdata();
					Delay1ms(100);
					TP_Get_Adjdata();//读取原始值
					return;//校正完成
			}

		}
		Delay1ms(10);
		outtime++;
		if(outtime>1000)//10S没有按键按下
		{
			TP_Get_Adjdata();//读取原始值
			break;//退出校准程序
	 	}
	}

}
//按键扫描 并等待松手
void KeyScan(void) 
{
	Access_TP_coordinate(1);	
	keystatus = Touch_Status(); 				
	while(keystatus)
	{
		clear_TP_interrupt();
		Delay1ms(50);
		keystatus = Touch_Status();		
	}
}

