#include "stm32f401xe.h"
#include "Delay.h"
#include "RCC_Clock.h"
#include <stdint.h>



static void GPIO_Config(void)
{
	/************ STEPS FOLLOWED ***********
	1. Enable the GPIO CLOCK
	2. Set the Pin as OUTPUT
	3. Configure the OUTPUT MODE
	***************************************/
	
	// 1. Enable the GPIO CLOCK
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  
	
	// 2. Set the Pin as OUTPUT
	GPIOA->MODER &=  ~ GPIO_MODER_MODE5;  // pin PA5(bits 11:10) as Output (01)
	GPIOA->MODER |= GPIO_MODER_MODE5_0;
	
	// 3. Configure the OUTPUT MODE
    GPIOB->OTYPER &= ~GPIO_OTYPER_OT_5;    // Output push-pull
    GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED5; // Low-speed output
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR5;    // No pull-up or pull-down
}




int main (void)
{
	
	SysClockConfig ();
	GPIO_Config ();
	
	while (1)
	{
		GPIOA->BSRR = GPIO_BSRR_BS5;  // Set the pin PA5
		delay_ms(4000); 
		GPIOA->BSRR = GPIO_BSRR_BR5;  // Reset pin PA5
		delay_ms(4000); 
	}
}

