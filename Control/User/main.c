#include "sys.h"
#include "FreeRTOSConfig.h"

// LD3320主函数 用于语音输入，下分
extern struct TIMEData TimeData;
void User_Modification(u8 dat);
void LD3320(void);
uint8_t temperature,humidity;
u8 nAsrStatus = 0;
u8 nAsrRes = 0;
u8 flag = 0;
static u32 cnt = 0;

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

TaskHandle_t LD3320App_Handle = NULL;
TimerHandle_t PosDetectionApp_Handle = NULL;
TaskHandle_t ESP32CommuteApp_Handle = NULL;
TimerHandle_t ClockDisplayApp_Handle = NULL;
TimerHandle_t TemperatureGetApp_Handle = NULL;
TaskHandle_t PlayMusicApp_Handle = NULL;

uint8_t nAsrStatus;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	// 初始化

	//LD3320_Init();
	//EXTIX_Init();
	//LD_Reset();

	ds1302_gpio_init();
	ds1032_init();
	//tDHT11_start();
	Serial_Init();
	Serial_Printf("Init Finish1!");
	//while(1) {
		//tDHT11_rec_data(&temperature,&humidity);
		//Serial_Printf("Tem:%d\r\nHum:%d\r\n",temperature,humidity);
		//delay_s(1);
	//}
	// 创建多线程
	BaseType_t xReturn = pdPASS;
	xReturn = xTaskCreate((TaskFunction_t)TemperatureGet,			  /* 任务入口函数---即任务函数的名称，需要我们自己定义并且实现。*/
						  (const char *)"LD3320",			  /* 任务名字---字符串形式， 最大长度由 FreeRTOSConfig.h 中定义的configMAX_TASK_NAME_LEN 宏指定，多余部分会被自动截掉，这里任务名字最好要与任务函数入口名字一致，方便进行调试。*/
						  (uint16_t)512,					  /* 任务栈大小---字符串形式， 最大长度由 FreeRTOSConfig.h 中定义的configMAX_TASK_NAME_LEN 宏指定，多余部分会被自动截掉，这里任务名字最好要与任务函数入口名字一致，方便进行调试。*/
						  (void *)NULL,						  /* 任务入口函数参数---字符串形式， 最大长度由 FreeRTOSConfig.h 中定义的configMAX_TASK_NAME_LEN 宏指定，多余部分会被自动截掉，这里任务名字最好要与任务函数入口名字一致，方便进行调试。*/
						  (UBaseType_t)1,					  /* 任务的优先级---优先级范围根据 FreeRTOSConfig.h 中的宏configMAX_PRIORITIES 决定， 如果使能 configUSE_PORT_OPTIMISED_TASK_SELECTION，这个宏定义，则最多支持 32 个优先级；如果不用特殊方法查找下一个运行的任务，那么则不强制要求限制最大可用优先级数目。在 FreeRTOS 中， 数值越大优先级越高， 0 代表最低优先级。*/
						  (TaskHandle_t *)&LD3320App_Handle); /* 任务控制块指针---在使用内存的时候，需要给任务初始化函数xTaskCreateStatic()传递预先定义好的任务控制块的指针。在使用动态内存的时候，任务创建函数 xTaskCreate()会返回一个指针指向任务控制块，该任务控制块是 xTaskCreate()函数里面动态分配的一块内存。*/
	//xReturn = xTaskCreate((TaskFunction_t)ESP32Commute,
		//				  (const char *)"ESP32Commute",
			//			  (uint16_t)512,
				//		  (void *)NULL,
					//	  (UBaseType_t)3,
						//  (TaskHandle_t *)&ESP32CommuteApp_Handle);

	// 姿态定时任务1s
	PosDetectionApp_Handle = xTimerCreate("PosDetection",
						  pdMS_TO_TICKS(1000),
						  pdTRUE,
						  NULL,
						  (TimerCallbackFunction_t)PosDetection
						  );
						  
	// 时钟定时任务1s
	ClockDisplayApp_Handle = xTimerCreate( "ClockDisplay",
						  pdMS_TO_TICKS(1000),
						  pdTRUE,
						  NULL,
						  (TimerCallbackFunction_t)ClockDisplay);
	
	// 温度定时任务1s
	//TemperatureGetApp_Handle = xTimerCreate("TemperatureGet",
		//				  pdMS_TO_TICKS(5000),
			//			  pdTRUE,
				//		  NULL,
					//	  (TimerCallbackFunction_t)TemperatureGet); 
						  
	// = xTaskCreate((TaskFunction_t )PlayMusic,
	//(const char*    )"PlayMusic",
	//(uint16_t       )512,
	//(void*          )NULL,
	//(UBaseType_t    )5,
	//(TaskHandle_t*  )&PlayMusicApp_Handle);
	/* 启动任务调度。*/
	Serial_Printf("Hi\n");
	if (pdPASS == xReturn)
	{
		Serial_Printf("In!");
		//xTimerStart(TemperatureGetApp_Handle,0); 
		//xTimerStart(ClockDisplayApp_Handle,0); 
		vTaskStartScheduler(); /* 启动任务，开启调度 */
	}
	while (1)
	{
		cnt += 1;
		//ds1032_read_realTime();
		//Serial_Printf("hi %d\r\n",TimeData.second);
		//delay_s(1);
		//vTaskDelay(1000);
	}
	// //taskEXIT_CRITICAL();
}

void TIM2_IRQHandler(void)
{
	//tDHT11_rec_data(&temperature, &dec);
	//Serial_Printf("%d", temperature);
	//Serial_Printf("%d", dec);
}

void LD3320(void)
{
	uint8_t nAsrRes = 0;

	nAsrStatus = LD_ASR_NONE; // 初始状态：没有在作ASR

	// while (1)
	// {
	// 	switch (nAsrStatus)
	// 	{
	// 	case LD_ASR_RUNING:

	// 	case LD_ASR_ERROR:
	// 		break;
	// 	case LD_ASR_NONE:
	// 		nAsrStatus = LD_ASR_RUNING;
	// 		if (RunASR() == 0) // 启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算
	// 		{
	// 			nAsrStatus = LD_ASR_ERROR;
	// 		}
	// 		break;
	// 	case LD_ASR_FOUNDOK:
	// 		nAsrRes = LD_GetResult(); // 一次ASR识别流程结束，去取ASR识别结果
	// 		// printf("\r\n识别码:%d", nAsrRes);

	// 		switch (nAsrRes) // 对结果执行相关操作,客户修改
	// 		{
	// 		case CODE_WEATHER: // 命令“天气”
	// 		{
	// 			// Weahter func
	// 			break;
	// 		}
	// 		case CODE_INTRO: // 命令“介绍”
	// 		{
	// 			// Intro func
	// 			break;
	// 		}
	// 		case CODE_MUSIC: // 命令“播放音乐”
	// 		{
	// 			// Music func // 开启多线程播放
	// 			break;
	// 		}

	// 		case CODE_VOICE_UP: // 命令“提高音量”
	// 		{
	// 			// Voice up
	// 			break;
	// 		}
	// 		case CODE_VOICE_DOWN: // 命令“降低音量”
	// 		{
	// 			// Voice down
	// 			break;
	// 		}
	// 		default:
	// 		{
	// 			Serial_Printf("Default!");
	// 			break;
	// 		}
	// 		}
	// 		nAsrStatus = LD_ASR_NONE;
	// 		break;
	// 	case LD_ASR_FOUNDZERO:
	// 	default:
	// 		nAsrStatus = LD_ASR_NONE;
	// 		break;
	// 	}
	// }
	while (1)
	{
		switch (nAsrStatus)
		{
		case LD_ASR_RUNING:
		case LD_ASR_ERROR:
			break;
		case LD_ASR_NONE:
		{
			nAsrStatus = LD_ASR_RUNING;
			if (RunASR() == 0) /*	启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算*/
			{
				nAsrStatus = LD_ASR_ERROR;
			}
			break;
		}

		case LD_ASR_FOUNDOK: /*	一次ASR识别流程结束，去取ASR识别结果*/
		{
			nAsrRes = LD_GetResult(); /*获取结果*/
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
	isScreenShut = 1;
}

void ESP32Commute(void)
{
	while (1)
	{
		Serial_Printf("ESP32\r\n");
		vTaskDelay(5000);
	}
}

static void TemperatureGet(void)
{
	while(1) 
	{
		tDHT11_rec_data(&temperature,&humidity);
		Serial_Printf("Tem:%d\r\nHum:%d\r\n",temperature,humidity);
		vTaskDelay(1000);
	}
}

void ClockDisplay(void)
{
	//Serial_Printf("%d\r\n",cnt);
	ds1032_read_realTime();
	Serial_Printf("Time: %d:%d:%d\r\n",TimeData.hour,TimeData.minute,TimeData.second);
}

void PlayMusic(void)
{
	Serial_Printf("Music\n");
}

void User_Modification(u8 dat)
{
	if (dat == 0)
	{
		flag = 1;
		printf("收到\r\n");
	}
	else if (flag)
	{
		flag = 0;
		switch (nAsrRes) /*对结果执行相关操作,客户修改*/
		{
		case CODE_DMCS:						/*命令“代码测试”*/
			printf("代码测试识别成功\r\n"); /*text.....*/
			break;
		case CODE_CSWB:						/*命令“测试完毕”*/
			printf("测试完毕识别成功\r\n"); /*text.....*/
			break;

		case CODE_1KL1:					/*命令“北京”*/
			printf("北京识别成功\r\n"); /*text.....*/
			break;
		case CODE_1KL2:					/*命令“上海”*/
			printf("上海识别成功\r\n"); /*text.....*/
			break;
		case CODE_1KL3:					/*命令“开灯”*/
			printf("开灯识别成功\r\n"); /*text.....*/
			break;
		case CODE_1KL4:					/*命令“关灯”*/
			printf("关灯识别成功\r\n"); /*text.....*/
			break;

		case CODE_2KL1:					/*命令“....”*/
			printf("广州识别成功\r\n"); /*text.....*/
			break;
		case CODE_2KL2:					/*命令“....”*/
			printf("深圳识别成功\r\n"); /*text.....*/
			break;
		case CODE_2KL3:					  /*命令“....”*/
			printf("向左转识别成功\r\n"); /*text.....*/
			break;
		case CODE_2KL4:					  /*命令“....”*/
			printf("向右转识别成功\r\n"); /*text.....*/
			break;

		case CODE_3KL1:						/*命令“....”*/
			printf("打开空调识别成功\r\n"); /*text.....*/
			break;
		case CODE_3KL2:						/*命令“....”*/
			printf("关闭空调识别成功\r\n"); /*text.....*/
			break;
		case CODE_5KL1:				/*命令“....”*/
			printf("后退识别成功"); /*text.....*/
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

		default:
			break;
		}
	}
	else
	{
		printf("请说出一级口令\r\n"); /*text.....*/
	}
}
