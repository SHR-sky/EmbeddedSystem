#include "LD3320.h"

/************************************************************************************
//	nAsrStatus 用来在main主程序中表示程序运行的状态，不是LD3320芯片内部的状态寄存器
//	LD_ASR_NONE:			表示没有在作ASR识别
//	LD_ASR_RUNING：		表示LD3320正在作ASR识别中
//	LD_ASR_FOUNDOK:		表示一次识别流程结束后，有一个识别结果
//	LD_ASR_FOUNDZERO:	表示一次识别流程结束后，没有识别结果
//	LD_ASR_ERROR:			表示一次识别流程中LD3320芯片内部出现不正确的状态
*********************************************************************************/
uint8 nAsrStatus = 0;	
uint8 nLD_Mode = LD_MODE_IDLE;//用来记录当前是在进行ASR识别还是在播放MP3
uint8 ucRegVal;

///用户修改
void LD3320_main(void)
{
	uint8 nAsrRes=0;
	LD3320_init();	  
 	printf("1、流水灯\r\n"); 
	printf("2、闪烁\r\n"); 				
	printf("3、按键触发\r\n"); 		
	printf("4、全灭\r\n"); 			
	printf("5、状态\r\n"); 		
	nAsrStatus = LD_ASR_NONE;//初始状态：没有在作ASR

	while(1)
	{
		switch(nAsrStatus)
		{
			case LD_ASR_RUNING:
			case LD_ASR_ERROR:		
					break;
			case LD_ASR_NONE:
					nAsrStatus=LD_ASR_RUNING;
					if (RunASR()==0)								//启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算
					{		
						nAsrStatus = LD_ASR_ERROR;
					}
					break;
			case LD_ASR_FOUNDOK:
					 nAsrRes = LD_GetResult( );						//一次ASR识别流程结束，去取ASR识别结果										 
					 printf("\r\n识别码:%d", nAsrRes);			 		
								
					 switch(nAsrRes)		   						//对结果执行相关操作,客户修改
						{
							case CODE_LSD:							//命令“流水灯”
								printf(" 流水灯 指令识别成功\r\n"); 
															 break;
							case CODE_SS:	 						//命令“闪烁”
								printf(" 闪烁 指令识别成功\r\n"); 
															 break;
							case CODE_AJCF:							//命令“按键触发”
								printf(" 按键触发 指令识别成功\r\n"); 
															break;
							case CODE_QM:							//命令“全灭”
								printf(" 全灭 指令识别成功\r\n");
															break;
							case CODE_JT:							//命令“状态”
								printf(" 状态 指令识别成功\r\n");
							
							default:break;
						}	
					nAsrStatus = LD_ASR_NONE;
					break;
			case LD_ASR_FOUNDZERO:
			default:
					nAsrStatus = LD_ASR_NONE;
					break;
			}//switch
		//开发板测试
		Board_text(nAsrRes );
	}// while
}

static uint8 LD_AsrAddFixed(void)
{
	uint8 k, flag;
	uint8 nAsrAddLength;
	#define DATE_A 5    //数组二维数值
	#define DATE_B 20		//数组一维数值
	//添加关键词，用户修改
	uint8  sRecog[DATE_A][DATE_B] = {
	 			"liu shui deng",\
				"shan shuo",\
				"an jian chu fa",\
				"quan mie",\
				"zhuang tai"\
		
																	};	
	uint8  pCode[DATE_A] = {
	 															CODE_LSD,	\
																CODE_SS,	\
																CODE_AJCF,\
																CODE_QM,	\
																CODE_JT		\
															};	//添加识别码，用户修改
	flag = 1;
	for (k=0; k<DATE_A; k++)
	{			
		if(LD_Check_ASRBusyFlag_b2() == 0)
		{
			flag = 0;
			break;
		}

		LD_WriteReg(0xc1, pCode[k] );				//如果DSP为忙的状态,写入识别字的位置
		LD_WriteReg(0xc3, 0);						//添加识别命令时在前面加入0
		LD_WriteReg(0x08, 0x04);					//08: 清楚FIFO, 位0写1清除FIFO_DATA,位2写1清楚FIFO_EXT, 写完之后要清零
		printf("%x",LD_ReadReg(0x08));
		LD3320_delay(1);
		LD_WriteReg(0x08, 0x00);
		LD3320_delay(1);

		for (nAsrAddLength=0; nAsrAddLength<DATE_B; nAsrAddLength++)		//把识别字添加进LD3320
		{
			if (sRecog[k][nAsrAddLength] == 0)
				break;
			LD_WriteReg(0x5, sRecog[k][nAsrAddLength]);						//05: FIFO_EXT的数据口	
		}
		LD_WriteReg(0xb9, nAsrAddLength);									//B9: 写入当前识别字的长度
		LD_WriteReg(0xb2, 0xff);
		LD_WriteReg(0x37, 0x04);											//37: 写04通知DSP要添加一条识别句,写06通知DSP开始识别语音
	}	 
	return flag;
}

static void Board_text(uint8 Code_Val)
{																					 
	switch(Code_Val)  //对结果执行相关操作
	{
		case CODE_LSD:  //命令“流水灯”
			Glide_LED();
		break;
		case CODE_SS:	  //命令“闪烁”
			Flicker_LED();
		break;
		case CODE_AJCF:	//命令“按键触发”
			Key_LED();
		break;
		case CODE_QM:		//命令“全灭”
			Off_LED();
		break;
		case CODE_JT:		//命令“状态”
			Jt_LED();
		break;
		default:break;
	}	
}

static void Delayms(uint16 i)
{
	unsigned char a,b;
	for(;i>0;i--)
	        for(b=4;b>0;b--)
	            for(a=113;a>0;a--);	
}

static void Glide_LED(void)
{
	LED1_ON();
	Delayms(0xfff);
	LED2_ON();
	Delayms(0xfff);
	LED3_ON();
	Delayms(0xfff);
	LED4_ON();
	Delayms(0xfff);
	LED1_OFF();
	Delayms(0xfff);
	LED2_OFF();
	Delayms(0xfff);
	LED3_OFF();
	Delayms(0xfff);
	LED4_OFF();
	Delayms(0xfff);
}

static void Flicker_LED(void)
{
	LED1_ON();
	LED2_ON();
	Delayms(0XFFF);
	LED1_OFF();
	LED2_OFF();
	Delayms(0XFFF);
}

static void Key_LED(void)
{
	LED3_ON();
	Delayms(0XFFF);
	LED3_OFF();
	Delayms(0XFFF);

}

static void Off_LED(void)
{
	LED1_OFF();
	LED2_OFF();
	LED3_OFF();
	LED4_OFF();
}

static void Jt_LED(void)
{
	LED4_ON();
	Delayms(0XFFF);
	LED4_OFF();
	Delayms(0XFFF);
}
///用户修改 end

///相关初始化
void LD3320_init(void)
{
	LD3320_GPIO_Cfg();			//社置复位管脚PB6,片选管脚PB9
	LD3320_EXTI_Cfg();			//设置外部中断管脚PC1,中断优先级(1,1)
	LD3320_SPI_cfg();			//WR:PB13,MISO_CLK:PA6,MOSI_CLK:PA7,SPI_CLK:PA5
	//LED_GPIO_cfg();				//设置LED灯的管脚
	LD_reset();					//激活内部DSP
}

static void LD3320_GPIO_Cfg(void)         //RST  CS
{	
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_AHB1PeriphClockCmd(LD3320RST_GPIO_CLK | LD3320CS_GPIO_CLK,ENABLE);		
	
	
		GPIO_InitStructure.GPIO_Pin =LD3320CS_PIN;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(LD3320CS_GPIO_PORT,&GPIO_InitStructure);			
		GPIO_InitStructure.GPIO_Pin =LD3320RST_PIN;
		GPIO_Init(LD3320RST_GPIO_PORT,&GPIO_InitStructure);			
}

static void LD3320_EXTI_Cfg(void)        //IRQ
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//使能SYSCFG时钟
	RCC_AHB1PeriphClockCmd(LD3320IRQ_GPIO_CLK, ENABLE);//使能GPIOC
	
	GPIO_InitStructure.GPIO_Pin =LD3320IRQ_PIN;					//中断管脚IRQ:PB8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//无上拉下拉
	GPIO_Init(LD3320IRQ_GPIO_PORT, &GPIO_InitStructure);
	//外部中断线配置
	SYSCFG_EXTILineConfig(LD3320IRQEXIT_PORTSOURCE, LD3320IRQPINSOURCE);	//配置PC1映射到中断线1
	EXTI_InitStructure.EXTI_Line = LD3320IRQEXITLINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger =EXTI_Trigger_Falling;		
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	//中断嵌套配置
	NVIC_InitStructure.NVIC_IRQChannel = LD3320IRQN;				//中断通道10-15,中断通道1
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

static void LD3320_SPI_cfg(void)               //  PC2   PC3   PB13
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	//spi端口配置
	RCC_APB1PeriphClockCmd(LD3320SPI_CLK,ENABLE);				//使能SPI1时钟	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);	//使能SPI对应GPIO的时钟
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
                                                                             
	GPIO_InitStructure.GPIO_Pin = LD3320SPIMISO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(LD3320SPIMISO_GPIO_PORT,&GPIO_InitStructure);		
	
	GPIO_InitStructure.GPIO_Pin = LD3320SPIMOSI_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(LD3320SPIMOSI_GPIO_PORT,&GPIO_InitStructure);		

	GPIO_InitStructure.GPIO_Pin = LD3320SPISCK_PIN;
	GPIO_Init(LD3320SPISCK_GPIO_PORT,&GPIO_InitStructure);		
	//复用引脚映射
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

	LD_CS_H();																		//PB9高电平
	
	SPI_Cmd(LD3320SPI, DISABLE);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;   			//全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						   			//主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					   			//8位
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;						   				//时钟极性 空闲状态时，SCK保持高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;						   			//时钟相位 数据采样从第一个时钟边沿开始
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;							   			//软件产生NSS
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;   			//波特率控制 SYSCLK/128
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   				//数据高位在前
	SPI_InitStructure.SPI_CRCPolynomial = 7;							   			//CRC多项式寄存器初始值为7
	SPI_Init(LD3320SPI, &SPI_InitStructure);

	SPI_Cmd(LD3320SPI, ENABLE);
	spi_send_byte(0xFF);			//启动传输,可以不要
}
/*
static void LED_GPIO_cfg(void)              //    PF9,PF10
{	
		GPIO_InitTypeDef GPIO_InitStructure;
	
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);			//使能小灯时钟
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);			//使能WR的时钟
	
		GPIO_InitStructure.GPIO_Pin = LED1_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = LED2_PIN;
		GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = LED3_PIN;
		GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = LED4_PIN;
		GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStructure);	
	
		GPIO_InitStructure.GPIO_Pin = LD3320WR_PIN;				
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(LD3320WR_GPIO_PORT, &GPIO_InitStructure);			//WR: PB7
	
		LED1_OFF();
		LED2_OFF();
		LED3_OFF();
		LED4_OFF();
}
*/
///相关初始化 end 
///中间层
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(LD3320IRQEXITLINE)!= RESET )
	{
		ProcessInt(); 
		printf("进入中断1\r\n");	
		EXTI_ClearFlag(LD3320IRQEXITLINE);
		EXTI_ClearITPendingBit(LD3320IRQEXITLINE);//清除LINE上的中断标志位   
	}
}

void LD3320_delay(unsigned long uldata)
{
	unsigned int i  =  0;
	unsigned int j  =  0;
	unsigned int k  =  0;
	for (i=0;i<5;i++)
	{
		for (j=0;j<uldata;j++)
		{
			k = 200;
			while(k--);
		}
	}
}

static uint8 RunASR(void)
{
	uint8 i=0;
	uint8 asrflag=0;
	for (i=0; i<5; i++)				//防止由于硬件原因导致LD3320芯片工作不正常，所以一共尝试5次启动ASR识别流程
	{
		LD_AsrStart();				//初始化ASR
		LD3320_delay(100);
		if (LD_AsrAddFixed()==0)	//添加关键词语到LD3320芯片中
		{
			LD_reset();				//LD3320芯片内部出现不正常，立即重启LD3320芯片
			LD3320_delay(50);		//并从初始化开始重新ASR识别流程
			continue;
		}
		LD3320_delay(10);
		if (LD_AsrRun() == 0)
		{
			LD_reset();			 	//LD3320芯片内部出现不正常，立即重启LD3320芯片
			LD3320_delay(50);		//并从初始化开始重新ASR识别流程
			continue;
		}
		asrflag=1;
		break;						//ASR流程启动成功，退出当前for循环。开始等待LD3320送出的中断信号
	}	
	return asrflag;
}

void LD_reset(void)
{
	LD_RST_H();				
	LD3320_delay(100);
	LD_RST_L();
	LD3320_delay(100);
	LD_RST_H();
	LD3320_delay(100);
	LD_CS_L();
	LD3320_delay(100);
	LD_CS_H();		
	LD3320_delay(100);
}

static void LD_AsrStart(void)
{
	LD_Init_ASR();
}

uint8 LD_Check_ASRBusyFlag_b2(void)
{
	uint8 j;
	uint8 flag = 0;
	for (j=0; j<10; j++)
	{
		if (LD_ReadReg(0xb2) == 0x21)		//如果DSP为闲状态就有错误
		{
			flag = 1;
			break;
		}
		LD3320_delay(10);		
	}
	return flag;
}
///中间层end


///寄存器操作
uint8 spi_send_byte(uint8 byte)			//SPI1发送字节和接收字节
{
	while (SPI_I2S_GetFlagStatus(LD3320SPI, SPI_I2S_FLAG_TXE) == RESET);
	
	SPI_I2S_SendData(LD3320SPI,byte);
	
	while (SPI_I2S_GetFlagStatus(LD3320SPI,SPI_I2S_FLAG_RXNE) == RESET);
	
	return SPI_I2S_ReceiveData(LD3320SPI);
}

void LD_WriteReg(uint8 data1,uint8 data2)
{
	LD_CS_L();
	__nop();
	__nop();
	__nop();
	__nop();
	LD_SPIS_L();
	spi_send_byte(0x04);
	spi_send_byte(data1);
	spi_send_byte(data2);
	__nop();
	LD_CS_H();
}

uint8 LD_ReadReg(uint8 reg_add)
{
	uint8 i;
	LD_CS_L();
	__nop();
	__nop();
	__nop();
	__nop();
	LD_SPIS_L();
	spi_send_byte(0x05);				//FIFO_EXT数据口
	spi_send_byte(reg_add);
	i=spi_send_byte(0x00);
//	printf("%d",i);
	__nop();
	LD_CS_H();
	return(i);
}

static uint8 LD_GetResult(void)
{
	return LD_ReadReg(0xc5);
}

static uint8 LD_AsrRun(void)
{
	LD_WriteReg(0x35, MIC_VOL);
	LD_WriteReg(0x1C, 0x09);
	LD_WriteReg(0xBD, 0x20);
	LD_WriteReg(0x08, 0x01);
	LD3320_delay( 5 );
	LD_WriteReg(0x08, 0x00);
	LD3320_delay( 5);

	if(LD_Check_ASRBusyFlag_b2() == 0)
	{
		return 0;
	}

	LD_WriteReg(0xB2, 0xff);	
	LD_WriteReg(0x37, 0x06);
	LD_WriteReg(0x37, 0x06);
	LD3320_delay(5);
	LD_WriteReg(0x1C, 0x0b);
	LD_WriteReg(0x29, 0x10);
	LD_WriteReg(0xBD, 0x00);   
	return 1;
}

static void ProcessInt(void)
{
	uint8 nAsrResCount=0;

	ucRegVal = LD_ReadReg(0x2B);

// 语音识别产生的中断
//（有声音输入，不论识别成功或失败都有中断）
	LD_WriteReg(0x29,0) ;
	LD_WriteReg(0x02,0) ;

	if((ucRegVal & 0x10) && LD_ReadReg(0xb2)==0x21 && LD_ReadReg(0xbf)==0x35)		
	{	 
			nAsrResCount = LD_ReadReg(0xba);

			if(nAsrResCount>0 && nAsrResCount<=4) 
			{
				nAsrStatus=LD_ASR_FOUNDOK; 				
			}
			else
			{
				nAsrStatus=LD_ASR_FOUNDZERO;
			}	
	}
	else
	{
		nAsrStatus=LD_ASR_FOUNDZERO;//执行没有识别
	}

	LD_WriteReg(0x2b,0);
	LD_WriteReg(0x1C,0);//写0:ADC不可用
	LD_WriteReg(0x29,0);
	LD_WriteReg(0x02,0);
	LD_WriteReg(0x2B,0);
	LD_WriteReg(0xBA,0);	
	LD_WriteReg(0xBC,0);	
	LD_WriteReg(0x08,1);//清除FIFO_DATA
	LD_WriteReg(0x08,0);//清除FIFO_DATA后 再次写0
}

static void LD_Init_Common(void)
{
	LD_ReadReg(0x06);  				//FIFO状态: 位3为1:FIFO_DATA已满  位5:FIFO_EXT已满
	LD_WriteReg(0x17, 0x35); 		//对LD3320进行软复位
	LD3320_delay(5);
	LD_ReadReg(0x06);  

	LD_WriteReg(0x89, 0x03); 		//89: 模拟电路控制,初始化写0x03,MP3播放写0xFF 
	LD3320_delay(5);
	LD_WriteReg(0xCF, 0x43);   		//CF: 内部省电模式设置,0x43初始化,MP3,ASR初始化0x4F
	LD3320_delay(5);
	LD_WriteReg(0xCB, 0x02);		//CB: 读取ASR结果(不准确4)
	
	/*PLL setting*/
	LD_WriteReg(0x11, LD_PLL_11);  	//11: 时钟频率设置  
	if (nLD_Mode == LD_MODE_MP3)
	{
		LD_WriteReg(0x1E, 0x00); 	//ADC初始化
		LD_WriteReg(0x19, LD_PLL_MP3_19);   
		LD_WriteReg(0x1B, LD_PLL_MP3_1B);   
		LD_WriteReg(0x1D, LD_PLL_MP3_1D);
	}
	else		
	{
		LD_WriteReg(0x1E,0x00);		//ADC初始化
		LD_WriteReg(0x19, LD_PLL_ASR_19); 
		LD_WriteReg(0x1B, LD_PLL_ASR_1B);		
		LD_WriteReg(0x1D, LD_PLL_ASR_1D);
	}
	LD3320_delay(5);
	
	LD_WriteReg(0xCD, 0x04);		//允许DSP休眠
	LD_WriteReg(0x17, 0x4c); 		//进入DSP休眠
	LD3320_delay(1);
	LD_WriteReg(0xB9, 0x00);
	LD_WriteReg(0xCF, 0x4F); 		//初始化ASR,MP3
	LD_WriteReg(0x6F, 0xFF); 		//对芯片进行初始化
}

static void LD_Init_ASR(void)
{
	nLD_Mode=LD_MODE_ASR_RUN;			//定义的全局变量表示当前正在执行ASR的检测
	LD_Init_Common();

	LD_WriteReg(0xBD, 0x00);
	LD_WriteReg(0x17, 0x48);			//17寄存器:写入0x48激活DSP,写0x35进行软复位,写0x4C进入休眠
	LD3320_delay(5);
	LD_WriteReg(0x3C, 0x80);   	
	//3C: FIFO_EXT:下限低八位
	LD_WriteReg(0x3E, 0x07);
	//3E: FIFO_EXT:下限高八位
	LD_WriteReg(0x38, 0xff);  			//38: FIFO_EXT:上限低八位
	LD_WriteReg(0x3A, 0x07);			//3A: FIFO_EXT:上线高八位
	LD_WriteReg(0x40, 0);          		//40: FIFO_EXT MCU水线低八位 
	LD_WriteReg(0x42, 8);				//42: FIFO_EXT MCU水线高八位
	LD_WriteReg(0x44, 0);   			//44: FIFO_EXT DSP水线低八位
	LD_WriteReg(0x46, 8); 				//46: FIFO_EXT DSP水线高八位
	LD3320_delay( 1 );
}
///寄存器操作 end
/*********************************************END OF FILE**********************/
