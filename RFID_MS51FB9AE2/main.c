#include <MS51_16K.H>
#include "spi.h"
#include "uart.h"
#include "sys.h"
#include "common.h"
#include "rfid.h"
#include "delay.h"
#include "adc.h"
#include "eeprom.h"

unsigned char uart_receive_data;
bit receiveFlag,bufOverFlag,millisecondRoutine = 0;

unsigned char status = '\0',ADC_Index = 0;
float ADCDataIN6, ADCDataIN7;
unsigned char str[7];
unsigned char CardAUTH[7];
unsigned char i,j, k;
unsigned int ADCMinVal_Current, ADCMaxVal_Current, ADCMaxVal_Voltage, ADCMinVal_Voltage; 
unsigned int ADCData_Current[50] , ADCData_Voltage[50];
unsigned char ArrayData[50],ReceivedData1[5],ReceivedData2[5],ArrRead1[5]={0xD3, 0x02,0x03,0xAD},ArrRead2[5]={0x63, 0x6F,0x66,0xAC};

bit flag = 0;
void sw_delay(int ms);
void SPI_Initial(void);


/* if define TIMER0_FSYS_DIV12, timer = (0x1FFF-0x1000)*12/24MHz = 4.08ms */
/* if define TIMER0_FSYS, timer = (0x1FFF-0x0010)/24MHz = 340us */
#define TH0_INIT        0x00 
#define TL0_INIT        0x10

/************************************************************************************************************
*    TIMER 0 interrupt subroutine
************************************************************************************************************/
unsigned int MsCounter = 0;
void Timer0_ISR (void) interrupt 1           /*interrupt address is 0x000B */
{
    _push_(SFRS);
  
    TH0 = TH0_INIT;
    TL0 = TL0_INIT;
    TF0 = 0 ;

	
	  if(MsCounter >= 1)
		{
			//P14 = ~P14;
		millisecondRoutine = 1;
		ADCDataIN6 = ADC_CH6_Read();
		ADCDataIN7 = ADC_CH7_Read();
		//Current 
		ADCData_Current[ADC_Index] = ADCDataIN6;
		//Voltage
		ADCData_Voltage[ADC_Index] = ADCDataIN7; 

			if (ADC_Index < 48)
			{
				ADC_Index++;
			}
		MsCounter = 0;
		ADCDataIN6 = 0; 
		ADCDataIN7 = 0;
		}
		MsCounter++;
    _pop_(SFRS);
}

unsigned int  GetMaxValue(unsigned int * Arr, unsigned char StartIndex,unsigned char EndIndex)
{
   unsigned int  MaxValue;
   unsigned int temp[40];
		for(i = StartIndex ; i <= EndIndex; i++)
   {
		 temp[i] = Arr[i];
		 temp[i+1] = Arr[i+1];
	   if( temp[i] >= temp[i+1]){
		   MaxValue = temp[i];
		   temp[i+1] = temp[i];
	   }
   }
   return MaxValue;
}
unsigned int  GetMinValue(unsigned int * Arr, unsigned char StartIndex,unsigned char EndIndex)
{
	unsigned int  temp[40];
	unsigned int  MinValue;
	for( i = StartIndex ; i <= EndIndex; i++)
	{
		temp[i] = Arr[i];
		temp[i+1] = Arr[i+1];
		if(temp[i] < temp[i+1]){
			MinValue = temp[i];
			temp[i+1] = temp[i];
		}
	}
	return MinValue;
}

void App__AdcDataSamplingHandling(void)
{ 


	 if ( ADC_Index > 40)
	 {
     ADCMaxVal_Current = 0; 
		 ADCMaxVal_Voltage = 0; 
		 ADCMinVal_Current = 0;
		 ADCMinVal_Voltage = 0; 
		 ADCMaxVal_Current= GetMaxValue(ADCData_Current,0,ADC_Index);
		 ADCMaxVal_Voltage= GetMaxValue(ADCData_Voltage,0,ADC_Index);
		 ADCMinVal_Current= GetMinValue(ADCData_Current,0,ADC_Index); 
		 ADCMinVal_Voltage= GetMinValue(ADCData_Voltage,0,ADC_Index);

     ADC_Index = 0;// Making index is equal to 0
	 }

}
void CardFiller(unsigned char *Card,unsigned char *Filler){
  int i;   
	for( i = 0; i <= 4 ; i++){
	   Card[i] = Filler[i];
	}
}

/*                
main function
*/
void main (void)
{
  	/* Modify HIRC to 24MHz for UART baud rate function only */
	  ALL_GPIO_QUASI_MODE;
	  P04= 0;
	  P05= 0;
    MODIFY_HIRC(HIRC_24);
    P06_QUASI_MODE;
	  ENABLE_TIMER1_MODE0;                           /* Timer 0 mode configuration */
    TIMER0_FSYS_DIV12;
    UART_Open(24000000,UART0_Timer3,115200);
	  ENABLE_TIMER0_INTERRUPT;                       /* enable Timer0 interrupt  */ 
    ENABLE_GLOBAL_INTERRUPT;                       /* enable interrupts */
    set_TCON_TR0;                                  /* Timer0 run */
		SPI_Initial();
	  P12 = 1;
	  P13 = P14 = 0;
	  sw_delay(3000);
	  MFRC522_Init();
	//	Read_DATAFLASH_ARRAY(0x38E0,&ArrRead1,5); 
	//	Read_DATAFLASH_ARRAY(0x38F0,&ArrRead2,5);
while(1)
{
		/*if(uart0_receive_flag){
				i++;
				ArrayData[i]  = uart0_receive_data;
				uart0_receive_flag = 0;
		}*/
	  status = MFRC522_Request(PICC_REQIDL, str);	//MFRC522_Request(0x26, str)
		status = MFRC522_Anticoll(str);	//Take a collision, look up 5 bytes
	  
   if (millisecondRoutine == 1)
		{
			millisecondRoutine = 0;
			App__AdcDataSamplingHandling();
		}
	  if (status == MI_OK)
		{
		   if(((str[0] == ArrRead1[0] ) &&
						(str[1] == ArrRead1[1]) &&
						(str[2] == ArrRead1[2]) &&
						(str[3] == ArrRead1[3]))||
						((str[0] == ArrRead2[0]) &&
						(str[1] == ArrRead2[1]) &&
						(str[2] == ArrRead2[2]) &&
						(str[3] == ArrRead2[3])))
			 {
				 if(flag == 0){
						CardFiller(&CardAUTH,&str);
				 }
				 
				 if((str[0] == CardAUTH[0]) 
					&&(str[1] == CardAUTH[1])
				  &&(str[2] == CardAUTH[2])
				  &&(str[3] == CardAUTH[3])){
			   if((ADCDataIN6 >= 0.065) ||
						(ADCDataIN6 <=0.085))
				 {
						if((ADCDataIN7 >= 1.49) || (ADCDataIN7 <= 2.8)){
							if(flag == 0){
					
								P13 = 1;
								P12 = P14 = 0;
								sw_delay(2000);
								P04 = 1;
								P05 = 1;
								sw_delay(2000);
							}
							if(flag == 1){
						    P13 = 0;
								P12 = 1;
                sw_delay(500);
								P13 = 1;
                P12 = 0;								
								sw_delay(500);
								P13= 1;
								P12= 0;
								sw_delay(500);
								P13 = P14 = 0;
								P12 = 1; 
								P04 = 0;
								P05 = 0;
							}
							flag = ~flag;
						}						
				 }
				 else{
				   P14 = 1 ; 
					 P13 = P12 =0;
					 P04 = 0;
					 P05 = 0;
				 }
			  }else{
					printf("UNAUTHORIZED CARD \r\n");
			}
		}
	}
		else
	{
		 MFRC522_Init();
		 sw_delay(25);
	}
	
}
 
}

// Software based delay. Time is not accurate.
void sw_delay (int ms){
    int a, b;
    for (a=0; a<1296; a++){
             for (b=0; b<ms; b++);
		}
}