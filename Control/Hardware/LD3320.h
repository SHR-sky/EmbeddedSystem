#ifndef _LD3320_H__
#define _LD3320_H__
#include "sys.h"
#include "string.h"
 
///以下三个状态定义用来记录程序是在运行ASR识别还是在运行MP3播放
#define LD_MODE_IDLE			0x00
#define LD_MODE_ASR_RUN			0x08
#define LD_MODE_MP3		 		0x40
///以下五个状态定义用来记录程序是在运行ASR识别过程中的哪个状态
#define LD_ASR_NONE					0x00	//表示没有在作ASR识别
#define LD_ASR_RUNING				0x01	//表示LD3320正在作ASR识别中
#define LD_ASR_FOUNDOK			0x10	//表示一次识别流程结束后，有一个识别结果
#define LD_ASR_FOUNDZERO 		0x11	//表示一次识别流程结束后，没有识别结果
#define LD_ASR_ERROR	 			0x31	//	表示一次识别流程中LD3320芯片内部出现不正确的状态
 
#define CLK_IN   					72/* user need modify this value according to clock in */
#define LD_PLL_11					(uint8_t)((CLK_IN/2.0)-1)
#define LD_PLL_MP3_19			0x0f
#define LD_PLL_MP3_1B			0x18
#define LD_PLL_MP3_1D   	(uint8_t)(((90.0*((LD_PLL_11)+1))/(CLK_IN))-1)
 
#define LD_PLL_ASR_19 		(uint8_t)(CLK_IN*32.0/(LD_PLL_11+1) - 0.51)
#define LD_PLL_ASR_1B 		0x48
#define LD_PLL_ASR_1D 		0x1f
 
#define MIC_VOL 0x43
///识别码（客户修改处）
#define CODE_LSD	1	 //唤醒
#define CODE_SS	  2	 //名字
#define LDE_1	  3	 //打开红
#define LDE_2	  4	 //打开绿
#define LDE_3		5  //打开蓝
#define LDE_All_OFF		6  //状态
#define voice_J_0  7	//提高音量

#define CODE_WEATHER 17 // 天气怎么样
#define CODE_VOICE_UP 18 // 提高音量
#define CODE_VOICE_DOWN 19 // 减小音量
#define CODE_MUSIC 20 // 播放音乐
#define CODE_INTRO 21 // 介绍
 
#define LDE_1_1	  51	 //打开红
#define LDE_2_1	  52	 //打开绿
#define LDE_3_1		53  //打开蓝
#define voice_J_1  57	//降低音量
 
#define chen		90  //娱乐
#define chen_1		91  //娱乐
 
 
/*
CS-PA4
IRQ-PC8
WR-PC9
SCK-PC10
MISO-PC11
MOSI-PC12
RST-PC13
*/
 
///LD3320引脚相关定义
#define LD3320RST_PIN					GPIO_Pin_13		
#define LD3320RST_GPIO_PORT		GPIOC
#define LD3320RST_GPIO_CLK		RCC_AHB1Periph_GPIOC
#define LD_RST_H() 						GPIO_SetBits(GPIOC, GPIO_Pin_13)
#define LD_RST_L() 						GPIO_ResetBits(GPIOC, GPIO_Pin_13)
 
///LD3320引脚相关定义
#define LD3320CS_PIN					GPIO_Pin_4		
#define LD3320CS_GPIO_PORT		GPIOA
#define LD3320CS_GPIO_CLK			RCC_AHB1Periph_GPIOA
#define LD_CS_H()							GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define LD_CS_L()							GPIO_ResetBits(GPIOA, GPIO_Pin_4)
 
#define LD3320IRQ_GPIO_CLK		RCC_AHB1Periph_GPIOC
#define LD3320IRQ_PIN					GPIO_Pin_8
#define LD3320IRQ_GPIO_PORT		GPIOC
#define LD3320IRQEXIT_PORTSOURCE		EXTI_PortSourceGPIOC
#define LD3320IRQPINSOURCE		GPIO_PinSource8
#define LD3320IRQEXITLINE			EXTI_Line8
#define LD3320IRQN						EXTI9_5_IRQn
 
#define LD3320WR_PIN					GPIO_Pin_9
#define LD3320WR_GPIO_PORT		GPIOC
#define LD3320WR_GPIO_CLK			RCC_AHB1Periph_GPIOC
#define LD_SPIS_H()  					GPIO_SetBits(GPIOC, GPIO_Pin_9)
#define LD_SPIS_L()  					GPIO_ResetBits(GPIOC, GPIO_Pin_9)
 
#define	LD3320SPI							SPI3
#define LD3320SPI_CLK					RCC_APB1Periph_SPI3						
 
#define LD3320SPIMISO_PIN					GPIO_Pin_11
#define LD3320SPIMISO_GPIO_PORT		GPIOC
#define LD3320SPIMISO_GPIO_CLK		RCC_AHB1Periph_GPIOC
 
#define LD3320SPIMOSI_PIN					GPIO_Pin_12
#define LD3320SPIMOSI_GPIO_PORT		GPIOC
#define LD3320SPIMOSI_GPIO_CLK		RCC_AHB1Periph_GPIOC
 
#define LD3320SPISCK_PIN					GPIO_Pin_13
#define LD3320SPISCK_GPIO_PORT		GPIOC
#define LD3320SPISCK_GPIO_CLK		RCC_AHB1Periph_GPIOC
 
///相关初始化
void LD3320_main(void);
void LD3320_init(void);
static void LD3320_GPIO_Cfg(void);
static void LD3320_EXTI_Cfg(void);
static void LD3320_SPI_cfg(void);
///中间层
static void LD3320_delay(unsigned long uldata);
uint8_t RunASR(void);
void LD_reset(void);
void LD_AsrStart(void);
uint8_t LD_Check_ASRBusyFlag_b2(void);
///寄存器操作
uint8_t spi_send_byte(uint8_t byte);
void LD_WriteReg(uint8_t data1,uint8_t data2);
uint8_t LD_ReadReg(uint8_t reg_add);
uint8_t LD_AsrAddFixed(void);
uint8_t LD_GetResult(void);
uint8_t LD_AsrRun(void);
void ProcessInt(void);
void LD_Init_Common(void);
void LD_Init_ASR(void);
 
#endif
