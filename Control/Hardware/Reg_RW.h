#ifndef REG_RW_H
#define REG_RW_H
#include "sys.h"

#define SDCK PBout(13)			//SPI 时钟信号
#define SDO  PCin(2)			//SPI 数据输出
#define SDI  PCout(3)			//SPI 数据输入
#define SCS  PBout(12)			//芯片片选信号
#define RSTB PBout(3)			//复位端口
#define IRQ  PBout(8)			//中断引脚


///LD3320引脚相关定义
#define LD3320_SDCK_GPIO_CLK			RCC_AHB1Periph_GPIOB
#define LD3320_SDCK_GPIO_PORT			GPIOB
#define LD3320_SDCK_PIN					GPIO_Pin_7

#define LD3320_SDO_GPIO_CLK				RCC_AHB1Periph_GPIOB
#define LD3320_SDO_GPIO_PORT			GPIOB
#define LD3320_SDO_PIN					GPIO_Pin_6

#define LD3320_SDI_GPIO_CLK				RCC_AHB1Periph_GPIOB
#define LD3320_SDI_GPIO_PORT			GPIOB
#define LD3320_SDI_PIN					GPIO_Pin_5

#define LD3320_SCS_GPIO_CLK				RCC_AHB1Periph_GPIOB
#define LD3320_SCS_GPIO_PORT			GPIOB
#define LD3320_SCS_PIN					GPIO_Pin_2

#define LD3320_RSTB_GPIO_CLK		    RCC_AHB1Periph_GPIOB
#define LD3320_RSTB_GPIO_PORT			GPIOB
#define LD3320_RSTB_PIN					GPIO_Pin_3

#define LD3320_IRQ_GPIO_CLK				RCC_AHB1Periph_GPIOB
#define LD3320_IRQ_GPIO_PORT			GPIOB
#define LD3320_IRQ_PIN					GPIO_Pin_0

#define LD3320_IRQEXIT_PORTSOURCE		EXTI_PortSourceGPIOB
#define LD3320_IRQPINSOURCE				GPIO_PinSource8
#define LD3320_IRQEXITLINE				EXTI_Line8
#define LD3320_IRQN						EXTI9_5_IRQn

#define READ_SDO()   GPIO_ReadInputDataBit(LD3320_SDO_GPIO_PORT, LD3320_SDO_PIN)



//函数声明
void LD3320_Init(void);
void EXTIX_Init(void);
void LD_WriteReg( unsigned char address, unsigned char dataout );
unsigned char LD_ReadReg( unsigned char address );


#endif
