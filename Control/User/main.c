#include "sys.h"

// LD3320主函数 用于语音输入，下分
extern struct TIMEData TimeData;
void User_Modification(u8 dat);
void LD3320(void);
uint8_t temperature,humidity;
u8 nAsrStatus = 0;
u8 nAsrRes = 0;
u8 flag = 0;

// 姿态检测主函数 用于检测到人的时候亮屏等功能
u8 isScreenShut = 1;
float formerDis = 10;
void PosDetection(void);

// 与EPS32进行通信，获取信息
void ESP32Commute(void); // 通信主函数

// 时钟显示主函数，显示时间
void ClockDisplay(void);

// 播放音乐主函数 ，做到允许暂停播放,停止播放（通过语音控制）
void PlayMusic(void);

static void TemperatureGet(void);

uint8_t nAsrStatus;

int main(void)
{
	SysTick_Config(SystemCoreClock);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	// 初始化

	TIM_Init();
	HC_Init();
	
	ds1302_gpio_init();
	ds1032_init();
	tDHT11_start();
	Serial_Init();
	Serial_Printf("Init Finish1!");
	while(1) {
		ClockDisplay();
		PosDetection();
		tDHT11_rec_data(&temperature,&humidity);
		Serial_Printf("Tem:%d\r\nHum:%d\r\n",temperature,humidity);
		delay_ms(500);
		//Serial_Printf("?");
	}

}

void TIM2_IRQHandler(void) 
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); 
		//PosDetection();
		//ClockDisplay();
	}
}	

void TIM5_IRQHandler(void) 
{
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)  
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update); 
		tDHT11_rec_data(&temperature,&humidity);
		Serial_Printf("Tem:%d\r\nHum:%d\r\n",temperature,humidity);
	}
}

void PosDetection(void)
{	
	float nowDis = HC_Get();
	float diffDis;
	diffDis = formerDis - nowDis;
	diffDis = diffDis < 0 ? -diffDis : diffDis;
	if (diffDis > 10 && nowDis < 100 && isScreenShut == 1) // cm
	{
		Serial_Printf("Light!"); // 亮屏
		isScreenShut = 0;
	}
	formerDis = nowDis;
	isScreenShut = 1;
}

void ESP32Commute(void)
{
	while (1)
	{
		Serial_Printf("ESP32\r\n");
	}
}

void TemperatureGet(void)
{
	tDHT11_rec_data(&temperature,&humidity);
	Serial_Printf("Tem:%d\r\nHum:%d\r\n",temperature,humidity);
}

void ClockDisplay(void)
{
	ds1032_read_realTime();
	Serial_Printf("Time: %d:%d:%d\r\n",TimeData.hour,TimeData.minute,TimeData.second);
}

void PlayMusic(void)
{
	Serial_Printf("Music\n");
}

