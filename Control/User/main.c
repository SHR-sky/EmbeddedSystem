#include "sys.h"

// Trig - PD0
// Echo - PD1
// TIM4 超声波

// DHT为PC13

// CLK - PC12
// DAT - PC10

// 屏幕
// USART2 USART2_TX PD5 USART2_RX PD6

// ESP32
// USART3 USART3_TX PD8 USART3_RX PD9

// DFPlayer 5V
// USART1 USART1_TX PA9 USART2_RX PA10

/*
	LD3320
	VCC接3.3V供 bbbb电，5V接5V供电
	GND--GND
	SCK--PB7
	MI--PB6
	MO--PB5
	CS--PB4
	RST--PB3
	IRQ--PB8
	WR---GND
*/

u8 nAsrStatus=0;
u8 nAsrRes=0;
u8 flag=0;

// LD3320主函数 用于语音输入，下分
void TaskHandle(void);
extern struct TIMEData TimeData;
void User_Modification(u8 dat);
void LD3320(void);
uint8_t temperature,humidity;

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

//uint8_t nAsrStatus;
void LD3320_test(void);

extern void LD3320_init(void);
extern void LD_WriteReg(u8 data1,u8 data2);
extern u8 LD_ReadReg(u8 reg_add);
extern void LD3320_delay(unsigned long uldata);
extern u8 spi_send_byte(u8 byte);

char receiveMsg[256];
uint8_t msgIndex=0;
uint8_t finishRev = 0;
uint8_t msgEnd = 0;
uint8_t isEnd = 0;

int main(void)
{
	SysTick_Config(SystemCoreClock);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	DFPlayer_Init();
	
	LD3320_Init();
	EXTIX_Init();
	LD_Reset();
	nAsrStatus = LD_ASR_NONE;		//	初始状态：没有在作ASR
	SCS=0;
	
	// 初始化
	Serial_Init();
	
	while(1)
	{
		//delay_ms(1000);
		//Serial_Printf("111\r\n");
		switch(nAsrStatus)
		{
			case LD_ASR_RUNING:
			case LD_ASR_ERROR:	
					 break;
			case LD_ASR_NONE:
			{
				nAsrStatus=LD_ASR_RUNING;
				if (RunASR()==0)	/*	启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算*/
				{
					nAsrStatus = LD_ASR_ERROR;
				}
				break;
			}

			case LD_ASR_FOUNDOK: /*	一次ASR识别流程结束，去取ASR识别结果*/
			{
				nAsrRes = LD_GetResult();		/*获取结果*/												
				User_Modification(nAsrRes);
				nAsrStatus = LD_ASR_NONE;
				break;
			}
			case LD_ASR_FOUNDZERO:
			default:
			{
				nAsrStatus = LD_ASR_NONE;
				break;
			}
		}
	}
	//Serial_Printf("Init Finish1!");
	//TIM_Init();
	
	/*
	HC_Init();
	//LD3320_init();
	//ds1302_gpio_init();
	ds1032_init();
	tDHT11_start();
	Serial_Printf("page main");
	Serial_End();
	//LD3320_main();
	//Serial_Printf("Run!\r\n");
	
	while(1) {
		//ClockDisplay();
		PosDetection();
		tDHT11_rec_data(&temperature,&humidity);
		//Serial_Printf("Tem:%d\r\nHum:%d\r\n",temperature,humidity);
		Serial_Printf("va0.val=%d",temperature+rand()%8-4);
		Serial_End();
		msgIndex=0;
		delay_ms(980);
		//Serial_Printf("?");
	}
	*/
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
		//Serial_Printf("page main"); // 亮屏
		//Serial_End();
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

// Printf USART2 
// Printf USART3

// 接收串口屏数据
void USART2_IRQHandler(void)
{
	uint16_t Res;
	if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		Res = USART_ReceiveData(USART2);
		Serial_Printf("%x\r\n",Res);
		if(Res == 0x1)
		{
			DFPlayer_PlayNext();
		}
		if(Res == 0x2)
		{
			DFPlayer_PlayPre();
		}
		if(Res == 0x3)
		{
			DFPlayer_VolumnUp();
		}
		if(Res == 0x4)
		{
			DFPlayer_VolumnDown();
		}
		if(Res==0x5)
		{
			DFPlayer_Pause();
		}
		if(Res==0x6)
		{
			DFPlayer_Play();
		}
		//receiveMsg[msgIndex] = Res;
		//msgIndex += 1;
		//Serial_Printf("%x\r\n",Res);
				//Serial_Printf("%d ",Res);
		//receiveMsg[msgIndex] = Res;
		//msgIndex += 1;
		//receiveMsg[msgIndex] = '\0';
		//Serial_Printf("t0.txt=\"%s\"",receiveMsg);
		//Serial_End();
		/*
		if (Res == 0xff)
		{
			receiveMsg[msgIndex-1] = '\0';
			//TaskHandle();
			Serial_Printf("t0.txt=\"%s\"",receiveMsg);
			msgIndex = 0;
			//msgEnd = 0;
		}
		*/
		/*
		if(Res == 2) // 天气按钮
		{
			Serial_Printf2("W"); // 发送天气
		}
		else if(Res == 3) // 设备查看 
		{
			Serial_Printf("t0.txt=\"桌面电子助手\nCopyright:芯片不去耦小队\nAuthor:ZhangYu,RuanZhenyu,ShenBowei\""); // 
			Serial_End();
		}
		else if(Res == 5) // 关机
		{
			Serial_Printf("page screen");
			Serial_End();
			
		}
		else if(Res == 6) 
		{
			Serial_Printf2("S"); // 本机状态
		}*/
	}
}

uint16_t Res3;
// 接收ESP32数据
void USART3_IRQHandler(void)
{
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		Res3 = USART_ReceiveData(USART3); // 读取接收到的数据
		TaskHandle();
		//Serial_Printf("%d ",Res);
		//receiveMsg[msgIndex] = Res;
		//msgIndex += 1;
		//receiveMsg[msgIndex] = '\0';
		//Serial_Printf("t0.txt=\"%s\"",receiveMsg);
		//Serial_End();
		/*
		if (Res == 0xff)
		{
			//receiveMsg[msgIndex-1] = '\0';
			//TaskHandle();
			msgIndex = 0;
			//msgEnd = 0;
			msgEnd += 1;
		}
		if(msgEnd>=3)
		{
			receiveMsg[msgIndex-3] = '\0';
			Serial_Printf("t0.txt=\"%s\"",&receiveMsg[1]);
			TaskHandle();
			msgIndex = 0;
			msgEnd = 0;
		}*/
			
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}

}

void TaskHandle(void) 
{
	if(Res3==0x1) // 说明校时
	{
		Serial_Printf("t0.txt=\"%s\"",&receiveMsg[1]);
		Serial_End();
	}
	else if(Res3==0x2) // 显示天气
	{
		Serial_Printf("page weather");
		Serial_End();
		Serial_Printf("t1.txt=\"Overcast\"");
		Serial_End();
		Serial_Printf("t3.txt=\"30\"");
		Serial_End();
	}
	else if(Res3==0x3) // 息屏
	{
		Serial_Printf("page screen");
		Serial_End();
		isScreenShut = 1;
	}
	else if(Res3==0x5) // 显示系统状态
	{
		Serial_Printf("page state");
		Serial_End();
		Serial_Printf("t1.txt=\"PC-MateBook\"");
		Serial_End();
		Serial_Printf("t7.txt=\"2d22h54m24s\"");
		Serial_End();
		Serial_Printf("t5.txt=\"88%%\"");
		Serial_End();
		Serial_Printf("t6.txt=\"99%%\"");
		Serial_End();
	}
	else if(Res3==0x6)
	{
		Serial_Printf("page main");
		Serial_End();
	}
	else 
	{
		Serial_Printf("t0.txt=\"%s\"",receiveMsg);
		Serial_End();
	}
}


void User_Modification(u8 dat)
{
	if(dat ==0)
	{
		flag=1;
		printf("I am here!\r\n");
	}
	else if(flag)
	{
		flag=0;
		switch(nAsrRes)		   /*对结果执行相关操作,客户修改*/
		{
			case CODE_DMCS:			/*命令“代码测试”*/
					Serial_Printf("Code\r\n"); /*text.....*/
												break;
			case CODE_CSWB:			/*命令“测试完毕”*/
					Serial_Printf("Finish\r\n"); /*text.....*/
												break;
			case CODE_1KL1:	 /*命令“北京”*/
					Serial_Printf("Beijing\r\n"); /*text.....*/
												break;
			case CODE_1KL2:		/*命令“上海”*/
		
					Serial_Printf("Shanghai\r\n"); /*text.....*/
												break;
			case CODE_1KL3:	 /*命令“开灯”*/
					//printf("\"开灯\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_1KL4:		/*命令“关灯”*/				
					//printf("\"关灯\"识别成功\r\n"); /*text.....*/
												break;
			
			case CODE_2KL1:	 /*命令“....”*/
					//printf("\"广州\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_2KL2:	 /*命令“....”*/
					//printf("\"深圳\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_2KL3:	 /*命令“....”*/
					//printf("\"向左转\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_2KL4:	 /*命令“....”*/
					//printf("\"向右转\"识别成功\r\n"); /*text.....*/
															break;
						
			case CODE_3KL1:	 /*命令“....”*/
					//printf("\"打开空调\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_3KL2:	 /*命令“....”*/
					//printf("\"关闭空调\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_5KL1:	 /*命令“....”*/
					//printf("\"后退\"识别成功"); /*text.....*/
												break;
//		case CODE_3KL4:	 /*命令“....”*/
//				printf("\"代码测试\"识别成功"); /*text.....*/
//											break;
//					
//					case CODE_4KL1:	 /*命令“....”*/
//							printf("O"); /*text.....*/
//														break;
//					case CODE_4KL2:	 /*命令“....”*/
//							printf("P"); /*text.....*/
//														break;
//					case CODE_4KL3:	 /*命令“....”*/
//							printf("Q"); /*text.....*/
//														break;
//					case CODE_4KL4:	 /*命令“....”*/
//							printf("R"); /*text.....*/
//														break;
			
			default:break;
		}
	}
	else 	
	{
		printf("Please say First\r\n"); /*text.....*/	
	}
	
}
