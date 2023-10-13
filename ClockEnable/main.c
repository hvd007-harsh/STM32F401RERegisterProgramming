#include "stm32f401xe.h"
#include <stdint.h>


/****************>>>>>>> STEPS FOLLOWED <<<<<<<***********
1. ENABLE HSE and wait for the HSE to become ready.
2. Set the POWER ENABLE CLOCK and VOLTAGE REGULATOR.
3. Configure the FLASH PREFETCH and the LATENCY Related Settings.
4. Configure the PRESCALARS HCLK, PCLK1 , PCLK2.
5. Configure the MAIN PLL 
6. Enable the PLL and wait for it to become ready.
7. Select the clock source and wait for it to be set.
************************************************************/
/*
void SysClockConfig(void)
{
	
#define PLL_M  4 
#define PLL_N  180
#define PLL_P  0  //PLL_P = 2 

// 1.ENABLE HSE and wait for the HSE to become ready.
RCC->CR |= RCC_CR_HSEON;
while(!(RCC->CR & RCC_CR_HSERDY));
//2. Set the POWER ENABLE CLOCK and VOLTAGE REGULATOR
	 RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	 PWR->CR |= PWR_CR_VOS;
//3.Configure the FLASH PREFETCH and the LATENCY Related Settings
	 FLASH->ACR |= FLASH_ACR_ICEN | FLASH_ACR_PRFTEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS ;
//4.Configure the PRESCALARS HCLK, PCLK1, PCLK2
//AHB PR 
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
//APB1 PR 
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
//APB2 PR 
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
//5. Configure the Main PLL
RCC->PLLCFGR = (PLL_M << 0) | (PLL_N << 6) | (PLL_P << 16)| (RCC_PLLCFGR_PLLSRC_HSE);

//6.Enable the PLL and wait for it to become ready 
RCC->CR |= RCC_CR_PLLON;
while(!(RCC->CR & RCC_CR_PLLRDY));

//7. Select the clock source and wait for it to be set.
RCC->CFGR |= RCC_CFGR_SW_PLL;
while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	
	
}

void GPIO_Config (void)
{
	********* STEPS FOLLOWED *************
	1. Enable the GPIO Clock 
	2. Set the Pin as Output 
	3. Configure the Output Mode
	************************************
	//1. Enable the GPIO Clock 
	RCC->AHB1ENR |= (1<<0);
	//2. Set the Pin as Output 
	GPIOA->MODER |= (1<<10); //Pin PA5 (bits 11:10) as Output (01)
	//3. Configure the Output Mode
	GPIOA->OTYPER =0;
  GPIOA->OSPEEDR =0;
	
}

*/
/************* For Turning On the GPIO We have to set/reset register (GPIOx_BSRR
****************
void delay(uint32_t time){
 while(time--);
}
int main(void){
   SysClockConfig();
	 GPIO_Config();
	
	while(1){
		GPIOA->BSRR |= (1<<5);// Set the Pin A5 
		delay(2000000);
		GPIOA->BSRR |= ((1<<5)<<16); // Reset pin PA5
		delay(2000000);
	}
}
*/



void SysClockConfig (void)
{
		/*************>>>>>>> STEPS FOLLOWED <<<<<<<<************
	
	1. ENABLE HSE and wait for the HSE to become Ready
	2. Set the POWER ENABLE CLOCK and VOLTAGE REGULATOR
	3. Configure the FLASH PREFETCH and the LATENCY Related Settings
	4. Configure the PRESCALARS HCLK, PCLK1, PCLK2
	5. Configure the MAIN PLL
	6. Enable the PLL and wait for it to become ready
	7. Select the Clock Source and wait for it to be set
	
	********************************************************/
	
	

	
	#define PLL_M 	4
	#define PLL_N 	84
	#define PLL_P 	2  

	// 1. ENABLE HSE and wait for the HSE to become Ready
	RCC->CR |= RCC_CR_HSEON;  // RCC->CR |= 1<<16; 
	while (!(RCC->CR & RCC_CR_HSERDY));  // while (!(RCC->CR & (1<<17)));
	
	// 2. Set the POWER ENABLE CLOCK and VOLTAGE REGULATOR
	RCC->AHB1ENR |= (1<<0);  // RCC->APB1ENR |= 1<<28;
	PWR->CR |= PWR_CR_VOS;  // PWR->CR |= 3<<14; 
	
	
	// 3. Configure the FLASH PREFETCH and the LATENCY Related Settings
	FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_5WS;  // FLASH->ACR = (1<<8) | (1<<9)| (1<<10)| (5<<0);
	
	// 4. Configure the PRESCALARS HCLK, PCLK1, PCLK2
	// AHB PR
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;  // RCC->CFGR &= ~(1<<4);
	
	// APB1 PR
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;  // RCC->CFGR |= (5<<10);
	
	// APB2 PR
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;  // RCC->CFGR |= (4<<13);
	
	
	// 5. Configure the MAIN PLL
	RCC->PLLCFGR = (PLL_M <<0) | (PLL_N << 6) | (PLL_P <<16) | (RCC_PLLCFGR_PLLSRC_HSE);  // (1<<22);

	// 6. Enable the PLL and wait for it to become ready
	RCC->CR |= RCC_CR_PLLON;  // RCC->CR |= (1<<24);
	while (!(RCC->CR & RCC_CR_PLLRDY));  // while (!(RCC->CR & (1<<25)));
	
	// 7. Select the Clock Source and wait for it to be set
	RCC->CFGR |= RCC_CFGR_SW_PLL;  // RCC->CFGR |= (2<<0);
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // while (!(RCC->CFGR & (2<<2)));
}


void GPIO_Config (void)
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

void delay (uint32_t time)
{
	while(time--){
	}
}



int main (void)
{
	
	SysClockConfig ();
	GPIO_Config ();
	
	while (1)
	{
		GPIOA->BSRR = GPIO_BSRR_BS5;  // Set the pin PA5
		delay (100); 
		GPIOA->BSRR = GPIO_BSRR_BR5;  // Reset pin PA5
		delay (100); 
	}
}

