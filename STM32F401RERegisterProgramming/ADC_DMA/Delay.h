
#ifndef __DELAY_H
#define __DELAY_H
#include <stdint.h>
#include "stm32f401xe.h"

typedef enum
{
	FREQ_10Hz = 100U,
	FREQ_100Hz = 10U,
	FREQ_1KHZ = 1U,
	FREQ_DEFAULT = 1U
} FreqTypeDef; 

extern volatile uint32_t uwTick; 
extern uint32_t uwTickPrio; 
extern FreqTypeDef uwTickFreq; 

uint32_t GetTick(void);
void Delay( int delay); 
void delay_ms(unsigned ms);
void delay_us(unsigned us);
void init_systick(void);

#endif
