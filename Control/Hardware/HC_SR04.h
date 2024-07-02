#ifndef _HC_SR04_H
#define _HC_SR04_H    
#include "sys.h"

#define TRIG_Send  PDout(0)
#define ECHO_Reci  PDin(1)
 
void HC_Init(void);
static void OpenTimerForHc(void);
static void CloseTimerForHc(void);
void TIM4_IRQHandler(void);
uint32_t GetEchoTimer(void);
float HC_Get(void );

#endif
