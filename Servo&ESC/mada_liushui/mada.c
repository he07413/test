#include "mada.h"
#include "led.h"
#include "sys.h"
unsigned char P1_1 = 0;//??
unsigned char P1_2 = 0;
unsigned char P1_5 = 0;
unsigned char P0_5 = 0;
unsigned char mode = 0;

__MADA_NUM Mada_nums = MD;

__MADA_PARAM Mada_param[MD + 1] = 
{
  {100,50,ON},
  {100,40,ON},
  {100,30,ON},
  {100,20,ON},
};//马达频率,占空比（强度）,开关状态
__MADA_PORT Mada_port[MD + 1] = 
{
  {(volatile int *)0x90,1},
  {(volatile int *)0x90,2},
  {(volatile int *)0x90,5},
  {(volatile int *)0x80,5},
};

void Mada_out0()//1ms调用一次
{
  unsigned char i;
  static unsigned int TimeCnt[MD + 1];
  for(i=0;i<=Mada_nums;i++)
  {
      if(Mada_param[i].Status == OFF)
      {
        MADA_PIN_OUT(Mada_port[i].Port,Mada_port[i].PinNum,Mada_param[i].Status);
        TimeCnt[i] = 0;
      }
      else
      {
        TimeCnt[i]++;
        
        if(TimeCnt[i] > ((Mada_param[i].Duty*10)/Mada_param[i].Freq))
          MADA_PIN_OUT(Mada_port[i].Port,Mada_port[i].PinNum,OFF);
        else
          MADA_PIN_OUT(Mada_port[i].Port,Mada_port[i].PinNum,ON);  
        if(TimeCnt[i] > (1000/Mada_param[i].Freq))
          TimeCnt[i] = 0;
      }
  }
}
  
void Mada_out()//1ms调用一次
{
  unsigned char i;
  static unsigned int TimeCnt[4];
  for(i=0;i<4;i++)
  {
      if(Mada_param[i].Status == 0)
      {
        if(i == 0)
					P1_1 = 0;
        else if(i == 1)
          P1_2 = 0;
        else if(i == 2)
          P1_5 = 0;
        else if(i == 3)
          P0_5 = 0;
        TimeCnt[i] = 0;
      }
      else
      {
        TimeCnt[i]++;
        if(TimeCnt[i] > ((Mada_param[i].Duty*10)/Mada_param[i].Freq))
        {
          if(i == 0)
						P1_1 = 0;
					else if(i == 1)
						P1_2 = 0;
					else if(i == 2)
						P1_5 = 0;
					else if(i == 3)
						P0_5 = 0;
        }
        else
        {
					if(i == 0)
						P1_1 = 1;
					else if(i == 1)
						P1_2 = 1;
					else if(i == 2)
						P1_5 = 1;
					else if(i == 3)
						P0_5 = 1;
        }  
        if(TimeCnt[i] > (1000/Mada_param[i].Freq))
          TimeCnt[i] = 0;
      }
  }

}

void Mada_execute(unsigned char mode)
{
  unsigned char i;
  static unsigned long time;
  
  time++;
  switch(mode)
  {
    case 1:
      for(i=0;i<=Mada_nums;i++)
        Mada_param[i].Status = ON;
      time = 0;
      break;
    case 2:
      if(time == 1)
      {
        Mada_param[MA].Status = ON;
        Mada_param[MB].Status = OFF;
        Mada_param[MC].Status = OFF;
        Mada_param[MD].Status = OFF;
      }
      if(time == 100)
      {
        Mada_param[MA].Status = OFF;
        Mada_param[MB].Status = ON;
        Mada_param[MC].Status = OFF;
        Mada_param[MD].Status = OFF; 
      }
      if(time == 200)
      {
        Mada_param[MA].Status = OFF;
        Mada_param[MB].Status = OFF;
        Mada_param[MC].Status = ON;
        Mada_param[MD].Status = OFF;  
      }
      if(time == 300)
      {
        Mada_param[MA].Status = OFF;
        Mada_param[MB].Status = OFF;
        Mada_param[MC].Status = OFF;
        Mada_param[MD].Status = ON;  
      }
      if(time >= 400)
        time = 0;
      break;
    case 3:
      if(time == 1)
      {
        Mada_param[MA].Status = OFF;
        Mada_param[MB].Status = OFF;
        Mada_param[MC].Status = OFF;
        Mada_param[MD].Status = ON;
      }
      if(time == 200)
      {
        Mada_param[MA].Status = OFF;
        Mada_param[MB].Status = OFF;
        Mada_param[MC].Status = ON;
        Mada_param[MD].Status = OFF; 
      }
      if(time == 300)
      {
        Mada_param[MA].Status = OFF;
        Mada_param[MB].Status = ON;
        Mada_param[MC].Status = OFF;
        Mada_param[MD].Status = OFF;  
      }
      if(time == 400)
      {
        Mada_param[MA].Status = ON;
        Mada_param[MB].Status = OFF;
        Mada_param[MC].Status = OFF;
        Mada_param[MD].Status = OFF;  
      }
      if(time >= 1200)
        time = 0;
      break;
    case 4:
      if(time == 1)
      {
        Mada_param[0].Status = 1;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 0;
      }
      if(time == 2000)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 1;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 0; 
      }
      if(time == 4000)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 1;
        Mada_param[3].Status = 0;  
      }
      if(time == 6000)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 1;  
      }
      if(time >= 8000)
        time = 0;
      break;
    case 5:
      if(time == 1)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 1;
      }
      if(time == 1000)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 0;
      }
      if(time == 3000)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 1;
        Mada_param[3].Status = 0; 
      }
      if(time == 4000)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 0;
      }
      if(time == 6000)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 1;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 0;  
      }
      if(time == 7000)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 0;
      }
      if(time == 9000)
      {
        Mada_param[0].Status = 1;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 0;  
      }
      if(time == 10000)
      {
        Mada_param[0].Status = 0;
        Mada_param[1].Status = 0;
        Mada_param[2].Status = 0;
        Mada_param[3].Status = 0;
      }
      if(time >= 12000)
        time = 0;
      break;
    default:
        Mada_param[MA].Status = OFF;
        Mada_param[MB].Status = OFF;
        Mada_param[MC].Status = OFF;
        Mada_param[MD].Status = OFF;
        time = 0;
      break;
  }
  
  Mada_out();
}
