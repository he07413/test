#include "pwm.h"

#define TRUE 1
#define FALSE 0

static uint32_t PWM_PR = 0; //period register
static float PWM_duty = 0;

TIM_HandleTypeDef TIM3_Handler;         //��ʱ��3PWM��� 
TIM_OC_InitTypeDef TIM3_CH4Handler;	    //��ʱ��3ͨ��4���

//TIM3 PWM���ֳ�ʼ�� 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM3_PWM_Init(u16 arr,u16 psc)
{ 
    TIM3_Handler.Instance=TIM3;            //��ʱ��3
    TIM3_Handler.Init.Prescaler=psc;       //��ʱ����Ƶ
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;//���ϼ���ģʽ
    TIM3_Handler.Init.Period=arr;          //�Զ���װ��ֵ
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&TIM3_Handler);       //��ʼ��PWM
    
    TIM3_CH4Handler.OCMode=TIM_OCMODE_PWM1; //ģʽѡ��PWM1
    TIM3_CH4Handler.Pulse=arr/40;            //���ñȽ�ֵ,��ֵ����ȷ��ռ�ձȣ�Ĭ�ϱȽ�ֵ2.5%
    TIM3_CH4Handler.OCPolarity=TIM_OCPOLARITY_HIGH; //����Ƚϼ���Ϊ�� 
    HAL_TIM_PWM_ConfigChannel(&TIM3_Handler,&TIM3_CH4Handler,TIM_CHANNEL_4);//����TIM3ͨ��4
	
    HAL_TIM_PWM_Start(&TIM3_Handler,TIM_CHANNEL_4);//����PWMͨ��4
}


//��ʱ���ײ�������ʱ��ʹ�ܣ���������
//�˺����ᱻHAL_TIM_PWM_Init()����
//htim:��ʱ�����
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_TIM3_CLK_ENABLE();			//ʹ�ܶ�ʱ��3
    __HAL_RCC_GPIOB_CLK_ENABLE();			//����GPIOBʱ��
	
    GPIO_Initure.Pin=GPIO_PIN_1;           	//PB1
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;  	//�����������
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
	GPIO_Initure.Alternate= GPIO_AF2_TIM3;	//PB1����ΪTIM3_CH4
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
}


//����TIMͨ��4��ռ�ձ�
//compare:�Ƚ�ֵ
void TIM_SetTIM3Compare4(u32 compare)
{
	TIM3->CCR4=compare; 
}

//--------------------------------------------------------------------------���
uint8_t SetAngle(float angleValue)//����Ƕ� 0x00-0xb4
{
  float val = 0;
  if(angleValue > 180)
    return FALSE;

  val = (2000.0/180)*angleValue + 500;
  TIM3->CCR4 = (uint32_t)val;
  return TRUE;
}
//--------------------------------------------------------------------Boost&Moto
uint8_t SetBoostPWM_Freq(uint32_t pwmHz)//Boost PWM Freq 0x01-0xf4240
{
  float val = 0;
  if(pwmHz > 1000000 || pwmHz < 1)//
    return FALSE;

  val = 1000000.0/pwmHz;
  PWM_PR = (uint32_t)val;
  TIM3->ARR = (uint32_t)val;
  SetBoostPWM_Duty(PWM_duty);
  return TRUE;
}

uint8_t SetBoostPWM_Duty(float duty)//Boost PWM Duty 0.0%-100.0%
{
  float val = 0;
  if(duty > 100 || duty < 0)
    return FALSE;

	if(0 == PWM_PR)
		PWM_PR = TIM3->ARR;
  val = duty ? (float)PWM_PR/100*duty : 0;
	
  TIM3->CCR4 = (uint32_t)val;
  PWM_duty = duty;
  return TRUE;
}
//------------------------------------------------------------------------------
void StopPWM(void)
{
   TIM3->CCR4 = 0;
}

//------------------------------------------------------------------------------
