#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"

// LD3320主函数 用于语音输入，下分
static void LD3320(void); 
// 姿态检测主函数 用于检测到人的时候亮屏等功能
static void PosDetection(void); 
// 与EPS32进行通信，获取信息
static void ESP32Commute(void); // 通信主函数
// 时钟显示主函数，显示时间
static void ClockDisplay(void);
// 播放音乐主函数 ，做到允许暂停播放,停止播放（通过语音控制）
static void PlayMusic(void);

static TaskHandle_t LD3320App_Handle = NULL;
static TaskHandle_t PosDetectionApp_Handle = NULL;
static TaskHandle_t ESP32CommuteApp_Handle = NULL;
static TaskHandle_t ClockDisplayApp_Handle = NULL;
static TaskHandle_t PlayMusicApp_Handle = NULL;

uint8_t temperature, dec;
uint8_t nAsrStatus;

int main(void)
{
	// 初始化
	Data_Out_Init();
	Data_In_Init();
	tDHT11_start();
	Serial_Init();
	//LD3320_main();

	// 创建多线程
	BaseType_t xReturn = pdPASS;
	taskENTER_CRITICAL();
	xReturn = xTaskCreate((TaskFunction_t )LD3320,  /* 任务入口函数---即任务函数的名称，需要我们自己定义并且实现。*/
		(const char*    )"LD3320",/* 任务名字---字符串形式， 最大长度由 FreeRTOSConfig.h 中定义的configMAX_TASK_NAME_LEN 宏指定，多余部分会被自动截掉，这里任务名字最好要与任务函数入口名字一致，方便进行调试。*/
        (uint16_t       )512,  /* 任务栈大小---字符串形式， 最大长度由 FreeRTOSConfig.h 中定义的configMAX_TASK_NAME_LEN 宏指定，多余部分会被自动截掉，这里任务名字最好要与任务函数入口名字一致，方便进行调试。*/
        (void*          )NULL,/* 任务入口函数参数---字符串形式， 最大长度由 FreeRTOSConfig.h 中定义的configMAX_TASK_NAME_LEN 宏指定，多余部分会被自动截掉，这里任务名字最好要与任务函数入口名字一致，方便进行调试。*/
        (UBaseType_t    )1, /* 任务的优先级---优先级范围根据 FreeRTOSConfig.h 中的宏configMAX_PRIORITIES 决定， 如果使能 configUSE_PORT_OPTIMISED_TASK_SELECTION，这个宏定义，则最多支持 32 个优先级；如果不用特殊方法查找下一个运行的任务，那么则不强制要求限制最大可用优先级数目。在 FreeRTOS 中， 数值越大优先级越高， 0 代表最低优先级。*/
        (TaskHandle_t*  )&LD3320App_Handle);/* 任务控制块指针---在使用内存的时候，需要给任务初始化函数xTaskCreateStatic()传递预先定义好的任务控制块的指针。在使用动态内存的时候，任务创建函数 xTaskCreate()会返回一个指针指向任务控制块，该任务控制块是 xTaskCreate()函数里面动态分配的一块内存。*/ 
	
		xReturn = xTaskCreate((TaskFunction_t )PosDetection,
		(const char*    )"PosDetection",
        (uint16_t       )512,  
        (void*          )NULL,
        (UBaseType_t    )2, 
        (TaskHandle_t*  )&PosDetectionApp_Handle);
		
		xReturn = xTaskCreate((TaskFunction_t )ESP32Commute,
		(const char*    )"ESP32Commute",
        (uint16_t       )512,  
        (void*          )NULL,
        (UBaseType_t    )3, 
        (TaskHandle_t*  )&ESP32CommuteApp_Handle);
		
		xReturn = xTaskCreate((TaskFunction_t )ClockDisplay,
		(const char*    )"ClockDisplay",
        (uint16_t       )512,  
        (void*          )NULL,
        (UBaseType_t    )4, 
        (TaskHandle_t*  )&ClockDisplayApp_Handle);
		
		xReturn = xTaskCreate((TaskFunction_t )PlayMusic,
		(const char*    )"PlayMusic",
        (uint16_t       )512,  
        (void*          )NULL,
        (UBaseType_t    )5, 
        (TaskHandle_t*  )&PlayMusicApp_Handle);
		
		/* 启动任务调度。*/          
	if(pdPASS == xReturn)
		vTaskStartScheduler();   /* 启动任务，开启调度 */
	else
		return -1;
	
	taskEXIT_CRITICAL(); 
}

void TIM2_IRQHandler(void)
{
	tDHT11_rec_data(&temperature, &dec);
	Serial_Printf("%d", temperature);
	Serial_Printf("%d", dec);
}

void LD3320(void)
{
	uint8_t nAsrRes = 0;

	nAsrStatus = LD_ASR_NONE; // 初始状态：没有在作ASR

	while (1)
	{
		switch (nAsrStatus)
		{
		case LD_ASR_RUNING:

		case LD_ASR_ERROR:
			break;
		case LD_ASR_NONE:
			nAsrStatus = LD_ASR_RUNING;
			if (RunASR() == 0) // 启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算
			{
				nAsrStatus = LD_ASR_ERROR;
			}
			break;
		case LD_ASR_FOUNDOK:
			nAsrRes = LD_GetResult(); // 一次ASR识别流程结束，去取ASR识别结果
			//printf("\r\n识别码:%d", nAsrRes);

			switch (nAsrRes) // 对结果执行相关操作,客户修改
			{
			case CODE_WEATHER: // 命令“天气” 
			{
				// Weahter func 
				break;
			}
			case CODE_INTRO: // 命令“介绍”
			{
				// Intro func
				break;
			}
			case CODE_MUSIC: // 命令“播放音乐”
			{
				// Music func // 开启多线程播放
				break;
			}

			case CODE_VOICE_UP: // 命令“提高音量”
			{
				// Voice up
				break;
			}
			case CODE_VOICE_DOWN: // 命令“降低音量”
			{
				// Voice down
				break;
			}
			default:
				break;
			}
			nAsrStatus = LD_ASR_NONE;
			break;
		case LD_ASR_FOUNDZERO:
		default:
			nAsrStatus = LD_ASR_NONE;
			break;
		}
	}
}

void PosDetection(void)
{
}
	
void ESP32Commute(void)
{
}
	
void ClockDisplay(void)
{
}

void PlayMusic(void)
{
}
