#include "ADC.h"


void ADC_Init(void){
   /*************>>>>>>>>>>>>><<<<<<<<<<<<<<<********
	1. Enable ADC and GPIO clock 
	2. Set the prescalar in the common Control Register (CCR)
	3. Set the scan mode and resolution in the Control Register (CR1)
	4. Set the Continuous Conversion , EOC, and Data Alignment in Control Register 2 
	5. Set the Sampling Time for the channels in ADC_SMPRx 
	6. Set the Regular Channel sequence length in ADC_SQR1
	7. Set the Respective GPIO PINs in the Analog Mode
	**************************************************
	*/
	//Enabling ADC and GPIO Clock 
	RCC->APB2ENR |= (1<<8); //Enabling ADC Clock 
	RCC->AHB1ENR |= (1<<0); //Enabling GPIOA Clock 
	//2. Set the Prescalar in the Common Control Register 
	ADC->CCR |= (2 << 16);
	//3. Setting the Scan Mode and Resolution in the Control Register
	ADC1->CR1 = (1<<8); //SCAN mode enabled 
	ADC1->CR2 &=  (~(1<<24));//12 Bit RES
	//4. Set the Continuous Conversion , EOC, and Data Alignment in Control
	ADC1->CR2 = (1<<1);
	ADC1->CR2 |= (1<<10); 
	ADC1->CR2 &= ~(1<<11);
	//Enable DMA for ADC 
	ADC1->CR2 |= (1<<8);
	//Enable Continuous Request 
	ADC1->CR2 |= (1<<9); 
	//5. Setting the sampling time
}