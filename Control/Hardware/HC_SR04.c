#include "HC_SR04.h"

// 红外测距


// Trig - PD0
// Echo - PD1
// TIM4 超声波

#define HCSR04_PORT     GPIOD
#define HCSR04_CLK      RCC_AHB1Periph_GPIOD
#define HCSR04_TRIG     GPIO_Pin_0
#define HCSR04_ECHO     GPIO_Pin_1
#define TRIG_H	GPIO_SetBits(GPIOD,GPIO_Pin_0)
#define TRIG_L	GPIO_ResetBits(GPIOD,GPIO_Pin_0)
#define ECHO_IN GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_1)

u16 msHcCount = 0; 
 
void HC_Init(void)
{  
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;   
    RCC_APB1PeriphClockCmd(HCSR04_CLK, ENABLE);
 
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; // 推挽输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure); // 初始化GPIOC.11
	GPIO_ResetBits(GPIOD,GPIO_Pin_0);
 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(HCSR04_PORT, &GPIO_InitStructure);  
    //GPIO_ResetBits(HCSR04_PORT,HCSR04_ECHO);    
 
 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   
 
    TIM_TimeBaseStructure.TIM_Period = (1000-1); 
    TIM_TimeBaseStructure.TIM_Prescaler =(84-1); 
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);          
 
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);  
    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);    
 
    NVIC_InitTypeDef NVIC_InitStructure;
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
 
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;             
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;       
    NVIC_Init(&NVIC_InitStructure);
 
    TIM_Cmd(TIM4,DISABLE);     
}

 
static void OpenTimerForHc()  
{
    TIM_SetCounter(TIM4,0);
    msHcCount = 0;
    TIM_Cmd(TIM4, ENABLE); 
}
 
 
static void CloseTimerForHc()    
{
    TIM_Cmd(TIM4, DISABLE); 
}
 
 
void TIM4_IRQHandler(void)  
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  
   {
       TIM_ClearITPendingBit(TIM4, TIM_IT_Update); 
       msHcCount++;
	   //Serial_Printf("%d\r\n",msHcCount);
   }
}
 
 
 
u32 GetEchoTimer(void)
{
    u32 t = 0;
    t = msHcCount*1000;
    t += TIM_GetCounter(TIM4);
    TIM4->CNT = 0;	
    delay_ms(50);
    return t;
}
 
float HC_Get(void)
{
    int t = 0;
    int i = 0;
    float lengthTemp = 0;
    float sum = 0;
    while(i!=5)
   {   
	   TRIG_H;
       delay_us(30);
       TRIG_L;   
	   OpenTimerForHc();
	   while(ECHO_IN == 0);       
       i = i + 1;
       while(ECHO_IN == 1);
       CloseTimerForHc();  
		
       t = GetEchoTimer();
       lengthTemp = ((float)t/ (float)58.0);//cm
       sum = lengthTemp + sum ;
 
   }
    lengthTemp = sum / (float)5.0;
    return lengthTemp;
}



