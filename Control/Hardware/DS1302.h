#ifndef __DS1302_H 
#define __DS1302_H  
 
#include "sys.h" 
 
#define CE_L GPIO_ResetBits(GPIOC,GPIO_Pin_11)//拉低使能位
#define CE_H GPIO_SetBits(GPIOC,GPIO_Pin_11)//拉高使能位
#define SCLK_L GPIO_ResetBits(GPIOC,GPIO_Pin_12)//拉低时钟线
#define SCLK_H  GPIO_SetBits(GPIOC,GPIO_Pin_12)//拉高时钟线
#define DATA_L  GPIO_ResetBits(GPIOC,GPIO_Pin_10)//拉低数据线
#define DATA_H  GPIO_SetBits(GPIOC,GPIO_Pin_10)//拉高数据线
 
struct TIMEData
{
	uint16_t year;
	uint8_t  month;
	uint8_t  day;
	uint8_t  hour;
	uint8_t  minute;
	uint8_t  second;
	uint8_t  week;
};//创建TIMEData结构体方便存储时间日期数据

extern struct TIMEData TimeData;//全局变量
void ds1302_gpio_init(void);//ds1302端口初始化
void ds1302_write_onebyte(uint8_t data);//向ds1302发送一字节数据
void ds1302_wirte_rig(uint8_t address,uint8_t data);//向指定寄存器写一字节数据
uint8_t ds1302_read_rig(uint8_t address);//从指定寄存器读一字节数据
void ds1032_init(void);//ds1302初始化函数
void ds1032_DATAOUT_init(void);//IO端口配置为输出
void ds1032_DATAINPUT_init(void);//IO端口配置为输入
void ds1032_read_time(void);//从ds1302读取实时时间（BCD码）
void ds1032_read_realTime(void);//将BCD码转化为十进制数据
 
#endif
