#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "pwm.h"
#include "timer.h"

#include "mada.h"

#define SERVO 0
#define ESC 1
#define ON 1
#define OFF 0

#define ITEM_NUM 6

#define MAX_PLUS 2.4
#define MIN_PLUS 0.8


typedef struct 
{
	u16 x;
	u16 y;
	u16 width;
	u16 height;
	u8 size;
	u8 *p;
}__WM_ITEM;

typedef struct
{
	uint8_t *p;
	uint32_t freq;
	float duty;
	float angle;
	uint8_t * out_status;
	uint8_t * cal_status;
	float  plus;
}__param_show;

__param_show param_show;

__WM_ITEM Main_item[ITEM_NUM] = 
{
	{80,12,60,24,24,(u8 *)"Mode:"},//模式 舵机测试Servo 电调测试ESC
	{16,48,40,16,16,(u8 *)"FREQ:"},//当前频率
	{16,80,40,16,16,(u8 *)"DUTY:"},//当前占空比
	{16,112,48,16,16,(u8 *)"ANGLE:"},//当前角度
	{16,144,32,16,16,(u8 *)"OUT:"},//输出状态ON OFF
	{16,240,64,16,HZ_16,(u8 *)"0123"},//校准行程
};

uint32_t param[2][ITEM_NUM];
uint32_t current_FREQ;
uint32_t current_DUTY;
uint32_t DutyMin;
uint32_t DutyMax;

void Item_Refresh(u8 item)
{
	LCD_Fill(Main_item[item].x+Main_item[item].width,Main_item[item].y,240,Main_item[item].y+Main_item[item].height,BLACK);
	switch(item)
	{
		case 0:
			if(param[0][0] == SERVO)
			{
				SetBoostPWM_Freq(50);
				if(param[0][4] == ON)
					SetAngle(param_show.angle);
				else
					SetAngle(0);
				param_show.p = (uint8_t *)"Servo";
			}
			else if(param[0][0] == ESC)
			{
				SetBoostPWM_Freq(param_show.freq);
				if(param[0][4] == ON)
					SetBoostPWM_Duty(param_show.duty);
				param_show.p = (uint8_t *)"ESC";
			}
			LCD_ShowString(Main_item[0].x+Main_item[0].width,Main_item[0].y,60,Main_item[0].height,Main_item[0].size,param_show.p);
			break;
		case 1:
			param_show.freq = param[0][1];
			SetBoostPWM_Freq(param_show.freq);
			LCD_ShowNum(Main_item[1].x+Main_item[1].width,Main_item[1].y,param_show.freq,6,Main_item[1].size);
			break;
		case 2:
			param_show.duty = param[0][2]/10.0;
			if(param[0][4] == ON)
				SetBoostPWM_Duty(param_show.duty);
			LCD_ShowNum(Main_item[2].x+Main_item[2].width,Main_item[2].y,param_show.duty,4,Main_item[2].size);
			LCD_ShowString(Main_item[2].x+Main_item[2].width+4*8,Main_item[2].y,8,Main_item[2].height,Main_item[2].size,(u8 *)".");	
			LCD_ShowNum(Main_item[2].x+Main_item[2].width+4*8+8,Main_item[2].y,(uint32_t)(param_show.duty*10)%10,1,Main_item[2].size);
			
			param_show.plus = (float)param[0][2]/(float)param[0][1];//单位ms 改变了占空比，显示对应脉宽
			LCD_Fill(56,176,88,192,BLACK);
			LCD_ShowNum(56,176,param_show.plus,1,16);//显示脉宽数据
			LCD_ShowString(56+8,176,8,16,16,(u8 *)".");	
			LCD_ShowNum(56+8+8,176,(uint32_t)(param_show.plus*100)%100,2,16);
			LCD_ShowString(56+8+8+2*8,176,2*8,16,16,(u8 *)"ms");
			break;
		case 3:
			param_show.angle = param[0][3]/10.0;
			if(param[0][4] == ON)
				SetAngle(param_show.angle);
			LCD_ShowNum(Main_item[3].x+Main_item[3].width,Main_item[3].y,param_show.angle,3,Main_item[3].size);
			LCD_ShowString(Main_item[3].x+Main_item[3].width+3*8,Main_item[3].y,8,Main_item[3].height,Main_item[3].size,(u8 *)".");	
			LCD_ShowNum(Main_item[3].x+Main_item[3].width+3*8+8,Main_item[3].y,(uint32_t)(param_show.angle*10)%10,1,Main_item[3].size);
			break;
		case 4:
			if(param[0][4] == ON)
			{
				param_show.out_status = (uint8_t *)"ON";
				if(param[0][0] == SERVO)
					SetAngle(param_show.angle);
				else if(param[0][0] == ESC)
					SetBoostPWM_Duty(param_show.duty);
			}
			else
			{
				param_show.out_status = (uint8_t *)"OFF";
				if(param[0][0] == SERVO)
					SetAngle(0);
				else if(param[0][0] == ESC)
					SetBoostPWM_Duty(0);
			}
			LCD_ShowString(Main_item[4].x+Main_item[4].width,Main_item[4].y,24,Main_item[4].height,Main_item[4].size,param_show.out_status);	
			break;
		case 5:
			if(param[0][5] == ON)
				param_show.cal_status = (uint8_t *)":ON";
			else
				param_show.cal_status = (uint8_t *)":OFF";
			LCD_ShowString(Main_item[5].x+Main_item[5].width,Main_item[5].y,32,Main_item[5].height,16,param_show.cal_status);
			break;
		
	}
}

void CAL_CountDown(uint16_t SegmentNum,__WM_ITEM CAL_Item_Location,uint32_t Second,uint8_t Turn)
{
	uint32_t circumference,LineSegment,Line_Interval,Length_Drawed = 0;
	uint16_t x1,y1,x2,y2;
	uint8_t i;
	
	
	CAL_Item_Location.x = CAL_Item_Location.x + CAL_Item_Location.width + 40;
	CAL_Item_Location.y = CAL_Item_Location.y - 48;
	CAL_Item_Location.height += 48;
	
	circumference = CAL_Item_Location.width*2 + CAL_Item_Location.height*2;
	Line_Interval = circumference/2/SegmentNum;//空白总长度=SegmentNum*Line_Interval = 1/2circumference
	LineSegment = (circumference -(Line_Interval*SegmentNum))/SegmentNum;//线段长度
	
	if(OFF == Turn)
	{
		LCD_Fill(CAL_Item_Location.x,CAL_Item_Location.y,CAL_Item_Location.x+CAL_Item_Location.width,CAL_Item_Location.y+CAL_Item_Location.height,BACK_COLOR);
		return;
	}
	
	for(i = 0; i<SegmentNum;i++)
	{
		if(((Length_Drawed+Line_Interval+LineSegment)>CAL_Item_Location.width)&&((Length_Drawed+Line_Interval+LineSegment)<=(CAL_Item_Location.width+CAL_Item_Location.height)))
		{
			if((Length_Drawed+Line_Interval)>=CAL_Item_Location.width)
			{
				x1 = CAL_Item_Location.x + CAL_Item_Location.width;
				y1 = CAL_Item_Location.y + (Length_Drawed + Line_Interval - CAL_Item_Location.width);
				x2 = x1;
				y2 = y1 + LineSegment;
			}
			else
			{
				x1 = CAL_Item_Location.x + Length_Drawed + Line_Interval;
				y1 = CAL_Item_Location.y;
				x2 = CAL_Item_Location.x + CAL_Item_Location.width;
				y2 = y1;
				LCD_DrawLine(x1,y1,x2,y2);
				x1 = x2;
				y1 = y2;
				x2 = x1;
				y2 = y1 + (Length_Drawed + Line_Interval + LineSegment - CAL_Item_Location.width);
				
			}
		}
		else if(((Length_Drawed+Line_Interval+LineSegment)>(CAL_Item_Location.width+CAL_Item_Location.height))&&((Length_Drawed+Line_Interval+LineSegment)<=(CAL_Item_Location.width*2+CAL_Item_Location.height)))
		{
			if((Length_Drawed+Line_Interval)>=(CAL_Item_Location.width+CAL_Item_Location.height))
			{
				x1 = CAL_Item_Location.x + (CAL_Item_Location.width*2 + CAL_Item_Location.height - Length_Drawed -(Line_Interval + LineSegment));
				y1 = CAL_Item_Location.y + CAL_Item_Location.height;
				x2 = x1 + LineSegment;
				y2 = y1;
			}
			else
			{
				x1 = CAL_Item_Location.x + CAL_Item_Location.width;
				y1 = CAL_Item_Location.y + Length_Drawed + Line_Interval - CAL_Item_Location.width;
				x2 = x1;
				y2 = CAL_Item_Location.y + CAL_Item_Location.height;
				LCD_DrawLine(x1,y1,x2,y2);
				x1 = CAL_Item_Location.x + (CAL_Item_Location.width*2 + CAL_Item_Location.height - Length_Drawed -(Line_Interval + LineSegment));
				y1 = y2;
				x2 = CAL_Item_Location.x + CAL_Item_Location.width;
				y2 = y1;
			}
		}
		else if((Length_Drawed+Line_Interval+LineSegment)>(CAL_Item_Location.width*2+CAL_Item_Location.height))
		{
			if((Length_Drawed+Line_Interval)>=(CAL_Item_Location.width*2+CAL_Item_Location.height))
			{
				x1 = CAL_Item_Location.x;
				y1 = CAL_Item_Location.y + (circumference - Length_Drawed - (Line_Interval + LineSegment));
				x2 = x1;
				y2 = y1 + LineSegment;
			}
			else
			{
				x1 = CAL_Item_Location.x;
				y1 = CAL_Item_Location.y + (circumference - Length_Drawed - (Line_Interval + LineSegment));
				x2 = x1;
				y2 = CAL_Item_Location.y + CAL_Item_Location.height;
				LCD_DrawLine(x1,y1,x2,y2);
				x1 = x2;
				y1 = y2;
				x2 = CAL_Item_Location.x + (CAL_Item_Location.width*2 + CAL_Item_Location.height - Length_Drawed - Line_Interval);
				y2 = y1;
			}
		}
		else
		{
			x1 = CAL_Item_Location.x + Length_Drawed + Line_Interval*(i?1:0);
			y1 = CAL_Item_Location.y;
			x2 = x1 + LineSegment;
			y2 = y1;
		}
		LCD_DrawLine(x1,y1,x2,y2);
		
		Length_Drawed = LineSegment*(i+1) + Line_Interval*i;
		delay_ms(Second*1000/SegmentNum);
	}
}

uint8_t Key_Aciton(uint8_t key_val)
{
	uint8_t i;
	unsigned int min,max;
	
	static uint8_t item = ITEM_NUM;
	if(key_val == 0)
		return(ITEM_NUM);

	for(i = 0;i<ITEM_NUM;i++)
	{
		if(param[0][i] != param[1][i])
		{
			param[1][i] = param[0][i];
		}
	}
	switch(key_val)
	{
		case WKUP_PRES:
			if(0 == item||4 == item)
				param[1][item] = (param[1][item] + 1)%2;
			else if(1 == item&&param[0][0] == ESC)//舵机控制不允许调节PWM频率
				param[1][item] = (param[1][item] + 10)%(500+10)<50? 50:(param[1][item] + 10)%(500+10);//步进10Hz,MAX 500Hz
			else if(2 == item&&param[0][0] == ESC)//仅电调可使用占空比调节方式
			{
				max = MAX_PLUS*param[1][1]>1000? 1000:MAX_PLUS*param[1][1];
				min = MIN_PLUS*param[1][1];
				param[1][item] = ((param[1][item] + 1)%(max+1))<min? min:(param[1][item] + 1)%(max+1);//步进0.01%,MAX 2.4ms MIN 0.8ms
			}
			else if(3 == item&&param[0][0] == SERVO)//仅舵机可使用角度调节方式
				param[1][item] = (param[1][item] + 1)%(1800+1);//步进0.1°,MAX 180°
			else if(5 == item && param[0][0] == ESC && param[0][4] == ON)
				param[1][item] = (param[1][item] + 1)%2;
			break;
		case DOWN_PRES:
			if(0 == item||4 == item)
				param[1][item] = (param[1][item] + 1)%2;
			else if(1 == item&&param[0][0] == ESC)//舵机控制不允许调节PWM频率
				param[1][item] = (param[1][item] - 10) >= 50? param[1][item] - 10 : 500;//步进10Hz,MIN 50Hz
			else if(2 == item&&param[0][0] == ESC)//仅电调可使用占空比调节方式
			{
				max = MAX_PLUS*param[1][1]>1000? 1000:MAX_PLUS*param[1][1];
				min = MIN_PLUS*param[1][1];
				param[1][item] = (param[1][item] - 1) >= min? param[1][item] - 1 : max;//步进0.01%,MAX 2.4ms MIN 0.8ms
			}
			else if(3 == item&&param[0][0] == SERVO)//仅舵机可使用角度调节方式
				param[1][item] = (param[1][item] - 1) < 1800? param[1][item] - 1 : 1800;//步进0.1°,MAX 180°
			else if(5 == item && param[0][0] == ESC && param[0][4] == ON)
				param[1][item] = (param[1][item] + 1)%2;
			break;
		case LEFT_PRES:
			POINT_COLOR = WHITE;
			if(item != ITEM_NUM)
				LCD_ShowString(Main_item[item].x,Main_item[item].y,Main_item[item].width,Main_item[item].height,Main_item[item].size,Main_item[item].p);
			item = ((int8_t)item - 1)<0? ITEM_NUM : item-1;
			if(item == ITEM_NUM)
			{
				LCD_ShowString(Main_item[item-1].x,Main_item[item-1].y,Main_item[item-1].width,Main_item[item-1].height,Main_item[item-1].size,Main_item[item-1].p);
				break;
			}
			POINT_COLOR = RED;
			LCD_ShowString(Main_item[item].x,Main_item[item].y,Main_item[item].width,Main_item[item].height,Main_item[item].size,Main_item[item].p);
			POINT_COLOR = WHITE;
			break;
		case RIGHT_PRES:
			POINT_COLOR = WHITE;
			if(item != ITEM_NUM)
				LCD_ShowString(Main_item[item].x,Main_item[item].y,Main_item[item].width,Main_item[item].height,Main_item[item].size,Main_item[item].p);
			item = (item + 1)%(ITEM_NUM + 1);
			if(item == ITEM_NUM)
			{
				LCD_ShowString(Main_item[item-1].x,Main_item[item-1].y,Main_item[item-1].width,Main_item[item-1].height,Main_item[item-1].size,Main_item[item-1].p);
				break;
			}
			POINT_COLOR = RED;
			LCD_ShowString(Main_item[item].x,Main_item[item].y,Main_item[item].width,Main_item[item].height,Main_item[item].size,Main_item[item].p);
			POINT_COLOR = WHITE;
			break;
		default :
			break;
	}
	if(item != ITEM_NUM)//设置过程中，更新数据
	{
		for(i = 0;i<ITEM_NUM;i++)
		{
			if(param[0][i] != param[1][i])
			{
				param[0][i] = param[1][i];
			}
		}
	}
	return(item);
}

int main(void)
{
	  u8 lcd_id[12],item,key_val,i;
	
    HAL_Init();                     //初始化HAL库   
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz
    delay_init(180);                //初始化延时函数
    uart_init(115200);              //初始化USART
    LED_Init();                     //初始化LED 
    LCD_Init();	                    //初始化LCD 
		TIM3_PWM_Init(20000-1,90-1);		//1M频率,50Hz
		TIM4_Init(10-1,9000-1);
		SetAngle(0);
		KEY_Init();
	
    POINT_COLOR=WHITE; 
		sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。	
		BACK_COLOR = BLACK;
		LCD_Clear(BACK_COLOR);
		LCD_ShowString(240 - 12*8,320 - 26,240,16,16,lcd_id);
		LCD_ShowString(16,176,40,16,16,(u8 *)"PLUS:");
//-------------------------------------------------------------------------------
		param[0][0] = SERVO;//舵机测试
		param[0][1] = 50;//频率
		param[0][2] = 40;//占空比*10
		param[0][3] = 0;//角度*10
		param[0][4] = OFF;
		param[0][5] = OFF;
		
		DutyMax = MAX_PLUS*param[0][1]>1000? 1000:MAX_PLUS*param[0][1];//最大高电平脉宽2.4ms 对应最大占空比*10
		DutyMin = MIN_PLUS*param[0][1];//最小高电平脉宽0.8ms 对应最小占空比*10
		param_show.plus = (float)param[0][2]/(float)param[0][1];//单位ms
			

		for(i = 0;i<ITEM_NUM;i++)
			param[1][i] = param[0][i];
			
		if(param[0][0] == SERVO)
			param_show.p = (uint8_t *)"Servo";
		else if(param[0][0] == ESC)
			param_show.p = (uint8_t *)"ESC";
		param_show.freq = param[0][1];
		param_show.duty = param[0][2]/10.0;
		param_show.angle = param[0][3]/10.0;
		if(param[0][4] == ON)
			param_show.out_status = (uint8_t *)"ON";
		else
			param_show.out_status = (uint8_t *)"OFF";
		if(param[0][5] == ON)
			param_show.cal_status = (uint8_t *)":ON";
		else
			param_show.cal_status = (uint8_t *)":OFF";
//-------------------------------------------------------------------------------
		for(i = 0;i<ITEM_NUM;i++)//显示条目
			LCD_ShowString(Main_item[i].x,Main_item[i].y,Main_item[i].width,Main_item[i].height,Main_item[i].size,Main_item[i].p);
		
		LCD_ShowString(Main_item[0].x+Main_item[0].width,Main_item[0].y,60,Main_item[0].height,Main_item[0].size,param_show.p);//显示数据	
		LCD_ShowNum(Main_item[1].x+Main_item[1].width,Main_item[1].y,param_show.freq,6,Main_item[1].size);
		LCD_ShowNum(Main_item[2].x+Main_item[2].width,Main_item[2].y,param_show.duty,4,Main_item[2].size);
		LCD_ShowString(Main_item[2].x+Main_item[2].width+4*8,Main_item[2].y,8,Main_item[2].height,Main_item[2].size,(u8 *)".");	
		LCD_ShowNum(Main_item[2].x+Main_item[2].width+4*8+8,Main_item[2].y,(uint32_t)(param_show.duty*10)%10,1,Main_item[2].size);	
		LCD_ShowNum(Main_item[3].x+Main_item[3].width,Main_item[3].y,param_show.angle,3,Main_item[3].size);
		LCD_ShowString(Main_item[3].x+Main_item[3].width+3*8,Main_item[3].y,8,Main_item[3].height,Main_item[3].size,(u8 *)".");	
		LCD_ShowNum(Main_item[3].x+Main_item[3].width+3*8+8,Main_item[3].y,(uint32_t)(param_show.angle*10)%10,1,Main_item[3].size);
		LCD_ShowString(Main_item[4].x+Main_item[4].width,Main_item[4].y,24,Main_item[4].height,Main_item[4].size,param_show.out_status);	
		LCD_ShowString(Main_item[5].x+Main_item[5].width,Main_item[5].y,32,Main_item[5].height,16,param_show.cal_status);
		
		LCD_ShowNum(56,176,param_show.plus,1,16);//显示脉宽数据
		LCD_ShowString(56+8,176,8,16,16,(u8 *)".");	
		LCD_ShowNum(56+8+8,176,(uint32_t)(param_show.plus*100)%100,2,16);
		LCD_ShowString(56+8+8+2*8,176,2*8,16,16,(u8 *)"ms");
//-------------------------------------------------------------------------------	
    while(1)
    { 
			key_val = KEY_Scan(0);
			
			if(key_val)//??
			{
				mode = (mode +1)%6;
				LCD_Fill(16,216,24,232,BLACK);//??
				LCD_ShowNum(16,216,mode,1,16);
			}
			
			item = Key_Aciton(key_val);
			if(item != ITEM_NUM)//设置过程中，更新显示
			{
				Item_Refresh(item);//更新设置的条目
				
				if(item == 1)//如果更改了频率，保持脉宽不变
				{
					param[0][2] = param_show.plus*param[0][1];
					Item_Refresh(2);
				}
			}
//-------------------------------------------------------------------------------
			if(param[0][5] == ON)//行程设定
			{
				DutyMax = MAX_PLUS*param[0][1]>1000? 1000:MAX_PLUS*param[0][1];//最大高电平脉宽2.4ms 对应最大占空比*10
				DutyMin = MIN_PLUS*param[0][1];//最小高电平脉宽0.8ms
				param[0][2] = DutyMax;
				Item_Refresh(2);
				CAL_CountDown(40,Main_item[5],5,ON);
				
				for(i = 0;i<80;i++)
				{
					param[0][2] = DutyMax-i*((DutyMax-DutyMin)/80);
					Item_Refresh(2);
					delay_ms(5);
				}
				
				param[0][2] = DutyMin;
				Item_Refresh(2);
				delay_ms(3000);
				param[0][5] = OFF;
				param_show.cal_status = (uint8_t *)":OFF";
				CAL_CountDown(20,Main_item[5],5,OFF);
				Item_Refresh(5);
			}
//-------------------------------------------------------------------------------			
			POINT_COLOR = BLACK;
			if(P1_1 == 0)
				LCD_Draw_Circle(64,216,16);
			if(P1_2 == 0)
				LCD_Draw_Circle(112,216,16);
			if(P1_5 == 0)
				LCD_Draw_Circle(160,216,16);
			if(P0_5 == 0)
				LCD_Draw_Circle(208,216,16);
			
			POINT_COLOR = WHITE;
			if(P1_1 == 1)
				LCD_Draw_Circle(64,216,16);
			if(P1_2 == 1)
				LCD_Draw_Circle(112,216,16);
			if(P1_5 == 1)
				LCD_Draw_Circle(160,216,16);
			if(P0_5 == 1)
				LCD_Draw_Circle(208,216,16);
		}
}
