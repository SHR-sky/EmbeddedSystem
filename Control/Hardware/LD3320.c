#include "LD3320.h"
#include "string.h"
 
/************************************************************************************
//	nAsrStatus ������main�������б�ʾ�������е�״̬������LD3320оƬ�ڲ���״̬�Ĵ���
//	LD_ASR_NONE:			��ʾû������ASRʶ��
//	LD_ASR_RUNING��		��ʾLD3320������ASRʶ����
//	LD_ASR_FOUNDOK:		��ʾһ��ʶ�����̽�������һ��ʶ����
//	LD_ASR_FOUNDZERO:	��ʾһ��ʶ�����̽�����û��ʶ����
//	LD_ASR_ERROR:			��ʾһ��ʶ��������LD3320оƬ�ڲ����ֲ���ȷ��״̬
���ȣ�
LD3320
SPI����
 
RST��PB15��
CS--PA4��
IRQ��PB12��
WR��PB13��
MISO��PA6��
MOSI��PA7��
SCK��PA5

CS-PA4
IRQ-PC8
WR-PC9
SCK-PC10
MISO-PC11
MOSI-PC12
RST-PC13

*********************************************************************************/
extern uint8_t nAsrStatus;	
uint8_t nLD_Mode = LD_MODE_IDLE;//������¼��ǰ���ڽ���ASRʶ�����ڲ���MP3
uint8_t ucRegVal;
 
 
static void LD3320_GPIO_Cfg(void)
{	
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(LD3320RST_GPIO_CLK | LD3320CS_GPIO_CLK,ENABLE);
		//LD_CS	/RSET
		GPIO_InitStructure.GPIO_Pin =LD3320CS_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(LD3320CS_GPIO_PORT,&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin =LD3320RST_PIN;
		GPIO_Init(LD3320RST_GPIO_PORT,&GPIO_InitStructure);
}
static void LD3320_EXTI_Cfg(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);	
 
	RCC_APB2PeriphClockCmd(LD3320IRQ_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin =LD3320IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LD3320IRQ_GPIO_PORT, &GPIO_InitStructure);
	//�ⲿ�ж�������
	SYSCFG_EXTILineConfig(LD3320IRQEXIT_PORTSOURCE, LD3320IRQPINSOURCE);
	EXTI_InitStructure.EXTI_Line = LD3320IRQEXITLINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger =EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	//�ж�Ƕ������
  NVIC_InitStructure.NVIC_IRQChannel = LD3320IRQN;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
static void LD3320_SPI_cfg(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
  //spi�˿�����
	RCC_APB2PeriphClockCmd(LD3320SPI_CLK,ENABLE);		
	RCC_APB2PeriphClockCmd(LD3320WR_GPIO_CLK | LD3320SPIMISO_GPIO_CLK | LD3320SPIMOSI_GPIO_CLK | LD3320SPISCK_GPIO_CLK,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = LD3320SPIMISO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LD3320SPIMISO_GPIO_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = LD3320SPIMOSI_PIN;
	GPIO_Init(LD3320SPIMOSI_GPIO_PORT,&GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = LD3320SPISCK_PIN;
	GPIO_Init(LD3320SPISCK_GPIO_PORT,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = LD3320WR_PIN;				
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LD3320WR_GPIO_PORT, &GPIO_InitStructure);
	
	LD_CS_H();
	
	SPI_Cmd(LD3320SPI, DISABLE);
 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;   	//ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						   						//��ģʽ
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					   					//8λ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;						   							//ʱ�Ӽ��� ����״̬ʱ��SCK���ֵ͵�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;						   						//ʱ����λ ���ݲ����ӵ�һ��ʱ�ӱ��ؿ�ʼ
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;							   							//�������NSS
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;   //�����ʿ��� SYSCLK/128
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   					//���ݸ�λ��ǰ
	SPI_InitStructure.SPI_CRCPolynomial = 7;							   							//CRC����ʽ�Ĵ�����ʼֵΪ7
	SPI_Init(LD3320SPI, &SPI_InitStructure);
 
	SPI_Cmd(LD3320SPI, ENABLE);
}
 
///�м��
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(LD3320IRQEXITLINE)!= RESET ) 
	{
		ProcessInt(); 
	
		EXTI_ClearFlag(LD3320IRQEXITLINE);
		EXTI_ClearITPendingBit(LD3320IRQEXITLINE);//���LINE�ϵ��жϱ�־λ  
	} 
}
static void LD3320_delay(unsigned long uldata)
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
uint8_t RunASR(void)
{
	uint8_t i=0;
	uint8_t asrflag=0;
	for (i=0; i<5; i++)		//��ֹ����Ӳ��ԭ����LD3320оƬ����������������һ������5������ASRʶ������
	{
		LD_AsrStart();			//��ʼ��ASR
		LD3320_delay(100);
		if (LD_AsrAddFixed()==0)	//��ӹؼ����ﵽLD3320оƬ��
		{
			LD_reset();				//LD3320оƬ�ڲ����ֲ���������������LD3320оƬ
			LD3320_delay(50);	//���ӳ�ʼ����ʼ����ASRʶ������
			continue;
		}
		LD3320_delay(10);
		if (LD_AsrRun() == 0)
		{
			LD_reset();			 //LD3320оƬ�ڲ����ֲ���������������LD3320оƬ
			LD3320_delay(50);//���ӳ�ʼ����ʼ����ASRʶ������
			continue;
		}
		asrflag=1;
		break;						//ASR���������ɹ����˳���ǰforѭ������ʼ�ȴ�LD3320�ͳ����ж��ź�
	}	
	return asrflag;
}
 
void LD_reset(void)
{
	uint8_t i;
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
	
	i=LD_ReadReg(0x6);
	LD_WriteReg(0x35, 0x33);
	LD_WriteReg(0x1b, 0x55);
	LD_WriteReg(0xb3, 0xaa);
	i=LD_ReadReg(0x35);
	i=LD_ReadReg(0x1b);
	i=LD_ReadReg(0xb3);
	i=i;
	LD_RST_H();
	LD3320_delay(100);
	LD_RST_L();
	LD3320_delay(100);
	LD_RST_H();
	LD3320_delay(100);
	LD_CS_L();
	LD3320_delay(100);
	LD_CS_H();		
	i=LD_ReadReg(0x6);
	LD3320_delay(100);
	i=LD_ReadReg(0x35);
	i=LD_ReadReg(0x1b);
	i=LD_ReadReg(0xb3);
	i=i;
}
void LD_AsrStart(void)
{
	LD_Init_ASR();
}
 
uint8_t LD_Check_ASRBusyFlag_b2(void)
{
	uint8_t j;
	uint8_t flag = 0;
	for (j=0; j<10; j++)
	{
		if (LD_ReadReg(0xb2) == 0x21)
		{
			flag = 1;
			break;
		}
		LD3320_delay(10);		
	}
	return flag;
}
 
 
///�Ĵ�������
uint8_t spi_send_byte(uint8_t byte)
{
	while (SPI_I2S_GetFlagStatus(LD3320SPI, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(LD3320SPI,byte);
	while (SPI_I2S_GetFlagStatus(LD3320SPI,SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(LD3320SPI);
}
 
void LD_WriteReg(uint8_t data1,uint8_t data2)
{
	LD_CS_L();
	LD_SPIS_L();
	spi_send_byte(0x04);
	spi_send_byte(data1);
	spi_send_byte(data2);
	LD_CS_H();
}
 
uint8_t LD_ReadReg(uint8_t reg_add)
{
	uint8_t i;
	LD_CS_L();
	LD_SPIS_L();
	spi_send_byte(0x05);
	spi_send_byte(reg_add);
	i=spi_send_byte(0x00);
	LD_CS_H();
	return(i);
}
 
uint8_t LD_GetResult(void)
{
	return LD_ReadReg(0xc5);
}
 
uint8_t LD_AsrRun(void)
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
void ProcessInt(void)
{
	uint8_t nAsrResCount=0;
 
	ucRegVal = LD_ReadReg(0x2B);
 
// ����ʶ��������ж�
//�����������룬����ʶ��ɹ���ʧ�ܶ����жϣ�
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
		nAsrStatus=LD_ASR_FOUNDZERO;//ִ��û��ʶ��
	}
 
	LD_WriteReg(0x2b,0);
	LD_WriteReg(0x1C,0);//д0:ADC������
	LD_WriteReg(0x29,0);
	LD_WriteReg(0x02,0);
	LD_WriteReg(0x2B,0);
	LD_WriteReg(0xBA,0);	
	LD_WriteReg(0xBC,0);	
	LD_WriteReg(0x08,1);//���FIFO_DATA
	LD_WriteReg(0x08,0);//���FIFO_DATA�� �ٴ�д0
}
 
void LD_Init_Common(void)
{
	LD_ReadReg(0x06);  
	LD_WriteReg(0x17, 0x35); 
	LD3320_delay(5);
	LD_ReadReg(0x06);  
 
	LD_WriteReg(0x89, 0x03);  
	LD3320_delay(5);
	LD_WriteReg(0xCF, 0x43);   
	LD3320_delay(5);
	LD_WriteReg(0xCB, 0x02);
	
	/*PLL setting*/
	LD_WriteReg(0x11, LD_PLL_11);       
	if (nLD_Mode == LD_MODE_MP3)
	{
		LD_WriteReg(0x1E, 0x00); 
		LD_WriteReg(0x19, LD_PLL_MP3_19);   
		LD_WriteReg(0x1B, LD_PLL_MP3_1B);   
		LD_WriteReg(0x1D, LD_PLL_MP3_1D);
	}
	else
	{
		LD_WriteReg(0x1E,0x00);
		LD_WriteReg(0x19, LD_PLL_ASR_19); 
		LD_WriteReg(0x1B, LD_PLL_ASR_1B);		
	  LD_WriteReg(0x1D, LD_PLL_ASR_1D);
	}
	LD3320_delay(5);
	
	LD_WriteReg(0xCD, 0x04);
	LD_WriteReg(0x17, 0x4c); 
	LD3320_delay(1);
	LD_WriteReg(0xB9, 0x00);
	LD_WriteReg(0xCF, 0x4F); 
	LD_WriteReg(0x6F, 0xFF); 
}
 
void LD_Init_ASR(void)
{
	nLD_Mode=LD_MODE_ASR_RUN;
	LD_Init_Common();
 
	LD_WriteReg(0xBD, 0x00);
	LD_WriteReg(0x17, 0x48);	
	LD3320_delay(5);
	LD_WriteReg(0x3C, 0x80);    
	LD_WriteReg(0x3E, 0x07);
	LD_WriteReg(0x38, 0xff);    
	LD_WriteReg(0x3A, 0x07);
	LD_WriteReg(0x40, 0);          
	LD_WriteReg(0x42, 8);
	LD_WriteReg(0x44, 0);    
	LD_WriteReg(0x46, 8); 
	LD3320_delay( 1 );
}
 
///��س�ʼ��
void LD3320_init(void)
{
	LD3320_GPIO_Cfg();	
	LD3320_EXTI_Cfg();
	LD3320_SPI_cfg();
	LD_reset();
}
static uint8_t LD_AsrAddFixed(void)
{
	uint8_t k, flag;
	uint8_t nAsrAddLength;
	#define DATE_A 20    //�����ά��ֵ
	#define DATE_B 50		//����һά��ֵ
	//��ӹؼ��ʣ��û��޸�
	uint8_t  sRecog[DATE_A][DATE_B] = { 
	 			"tian qi zen me yang",\
				"ni jiao shen me",\
				"ni shi shei",\
				"bo fang yin yue",\
				"ti gao yin liang",\
				"jiang di yin liang",\
				"guan bi hong deng",\
				"guan bi lv deng",\
				"guan bi lan deng",\
				"chen meng die shi shei",\
				"chen xiang xin shi shei",\
				"yin liang ti gao",\
				"yin liang jiang da",\
				"guan bi suo you deng" \
		
		
																	};	
	uint8_t  pCode[DATE_A] = {
	 															CODE_WEATHER,	\
																CODE_INTRO,	\
																CODE_INTRO,\
																CODE_MUSIC,   \
																CODE_VOICE_UP,	\
																CODE_VOICE_DOWN,		\
																LDE_1_1,   \
																LDE_2_1,	\
																LDE_3_1,		\
																chen,		\
																chen_1,		\
																voice_J_0,		\
																voice_J_1,		\
																LDE_All_OFF		\
															};	//���ʶ���룬�û��޸�
	flag = 1;
	for (k=0; k<DATE_A; k++)
	{			
		if(LD_Check_ASRBusyFlag_b2() == 0)
		{
			flag = 0;
			break;
		}
 
		LD_WriteReg(0xc1, pCode[k] );
		LD_WriteReg(0xc3, 0);
		LD_WriteReg(0x08, 0x04);
		LD3320_delay(1);
		LD_WriteReg(0x08, 0x00);
		LD3320_delay(1);
 
		for (nAsrAddLength=0; nAsrAddLength<DATE_B; nAsrAddLength++)
		{
			if (sRecog[k][nAsrAddLength] == 0)
				break;
			LD_WriteReg(0x5, sRecog[k][nAsrAddLength]);
		}
		LD_WriteReg(0xb9, nAsrAddLength);
		LD_WriteReg(0xb2, 0xff);
		LD_WriteReg(0x37, 0x04);
	}	 
	return flag;
}
 
 
//�û��޸�
