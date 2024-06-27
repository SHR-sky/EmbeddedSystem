#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"

void LD3320_main(void);
uint8_t temperature, dec;
uint8_t nAsrStatus;

int main(void)
{
	// 初始化
	Data_Out_Init();
	Data_In_Init();
	tDHT11_start();
	Serial_Init();
	LD3320_main();

	//while (1);
}

void TIM2_IRQHandler(void)
{
	tDHT11_rec_data(&temperature, &dec);
	Serial_Printf("%d", temperature);
	Serial_Printf("%d", dec);
}

void LD3320_main(void)
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
				// Music func
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
