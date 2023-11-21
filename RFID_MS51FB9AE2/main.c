#include <MS51_16K.H>
#include "spi.h"
#include "uart.h"
#include "sys.h"
#include "common.h"
#include "rfid.h"
#include "delay.h"
#include "adc.h"
#include "eeprom.h"


#define RED_LED			P14		
#define GREEN_LED		P13
#define BLUE_LED		P12
#define RELAY_1			P04


unsigned char uart_receive_data;
bit receiveFlag,bufOverFlag,millisecondRoutine = 0;

unsigned char status = '\0',ADC_Index = 0,ADC_Avg_Index = 0;
float ADCDataIN6, ADCDataIN7;
unsigned char str[7];
unsigned char CardAUTH[7];
unsigned char i,j, k;
unsigned int ADCMinVal_Current, ADCMaxVal_Current, ADCMaxVal_Voltage, ADCMinVal_Voltage,Average_ADC_Current  = 0, Average_ADC_Voltage = 0; 
unsigned int ADCData_Current[50] , ADCData_Voltage[50];
unsigned char ArrayData[50],ReceivedData1[5],ReceivedData2[5],ArrRead1[5]={0xD3,0x02,0x03,0xAD},ArrRead2[5]={0x63,0x6F,0x66,0xAC};//ArrRead1[5]={0xA3,0xE7,0xF1,0xAC},ArrRead2[5]={0x23,0xFF,0x0C,0xAD};

bit flag = 0, Overload_flag = 0 , Auth_flag = 0, Voltage_Over_flag = 0 , Voltage_Under_flag =0 ,Emergency_flag =0 ;
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

	
	
	  if((MsCounter >= 2 && ADC_Index < 40))
			{
		
      			
					//P14 = ~P14;
					millisecondRoutine = 1;
					ADCDataIN6 = ADC_CH6_Read();
					ADCDataIN7 = ADC_CH7_Read();
					//Current 
					ADCData_Current[ADC_Index] = ADCDataIN6;
					//Voltage
			   ADCData_Voltage[ADC_Index] = ADCDataIN7; 
				ADC_Index++;
			MsCounter = 0;
			ADCDataIN6 = 0; 
			ADCDataIN7 = 0;
		}
		MsCounter++;
    _pop_(SFRS);
}

unsigned int  GetMaxValue(unsigned int * Arr, unsigned char StartIndex,unsigned char EndIndex)
{
   unsigned int  MaxValue = 0;
   unsigned int temp;
		for(i = StartIndex ; i <= EndIndex; i++)
   {
		 temp = Arr[i];
	   if( temp > MaxValue){
		   MaxValue = temp;
	   }
   }
   return MaxValue;
}
unsigned int  GetMinValue(unsigned int * Arr, unsigned char StartIndex,unsigned char EndIndex)
{
	unsigned int  temp;
	unsigned int  MinValue = 0x1FFF;
	for( i = StartIndex ; i <= EndIndex; i++)
	{
		temp = Arr[i];
	  if( temp < MinValue){
		   MinValue = temp;
	  }
	}
	return MinValue;
}

float AC_Current = 0, AC_Voltage = 0;
unsigned int AC_Under_Counter=0 , AC_Over_Counter = 0;
char OverloadRetry=0;
bit RelayState = 0;
void App__AdcDataSamplingHandling(void)
{ 


	 if ( ADC_Index > 39)
	 {
     ADCMaxVal_Current = 0; 
		 ADCMaxVal_Voltage = 0; 
		 ADCMinVal_Current = 0;
		 ADCMinVal_Voltage = 0; 
		 ADCMaxVal_Current= GetMaxValue(ADCData_Current,0,ADC_Index-2);
		 ADCMaxVal_Voltage= GetMaxValue(ADCData_Voltage,0,ADC_Index-2);
		 ADCMinVal_Current= GetMinValue(ADCData_Current,0,ADC_Index-2); 
	//	 ADCMinVal_Voltage= GetMinValue(ADCData_Voltage,0,ADC_Index-2);

     ADC_Index = 0;// Making index is equal to 0
		 Average_ADC_Current += (ADCMaxVal_Current- ADCMinVal_Current);
		 Average_ADC_Voltage = ADCMaxVal_Voltage; //+(ADCMaxVal_Voltage- ADCMinVal_Voltage); 
		 ADC_Avg_Index++;
	 }
	 if(ADC_Avg_Index >= 32)
	 {
		  Average_ADC_Current = Average_ADC_Current / ADC_Avg_Index;
		 // Average_ADC_Voltage = Average_ADC_Voltage / ADC_Avg_Index;
		 ADC_Avg_Index = 0;
		 AC_Current = ((float)Average_ADC_Current) / 180.0;//  AC_Current = 12 ,i.e Actual current 1.2A 
	   AC_Voltage = ((float)Average_ADC_Voltage) * 0.0008056641;
		 
		if (AC_Current > 15.7)// || (AC_Voltage > 2.59 )|| (AC_Voltage < 1.39)
		{
					OverloadRetry++;
					Overload_flag = 1;				 
		}
		else if ( OverloadRetry < 3)
		{
					 Overload_flag = 0;
		}
		/////////////////Over AC Check///////////////////////////////////
		if(AC_Voltage > 2.59)
		{
			if ( AC_Over_Counter++ >= 5 )
			{
				AC_Over_Counter = 0;
				Voltage_Over_flag = 1;
			}
		}
		else if ( Voltage_Over_flag == 1 && AC_Voltage < 2.4)
		{
			Voltage_Over_flag = 0; 
		}
		else 
		{
			AC_Over_Counter = 0;
			Voltage_Over_flag = 0;
		}
		/////////////////////////// AC Under check /////////////////////
		
		if(AC_Voltage < 1.40)
		{
			if ( AC_Under_Counter++ >= 5 )
			{
				AC_Under_Counter = 0;
				Voltage_Under_flag = 1;
			}
		}
		else if ( AC_Under_Counter == 1 && AC_Voltage > 1.50)
		{
			Voltage_Under_flag = 0; 
		}
		else 
		{
			AC_Under_Counter = 0;
			Voltage_Under_flag = 0;
		}
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
	  //ALL_GPIO_QUASI_MODE;
	P04_PUSHPULL_MODE;
	P12_PUSHPULL_MODE;
	P13_PUSHPULL_MODE;
	P14_PUSHPULL_MODE;
	  RELAY_1= 0;
    MODIFY_HIRC(HIRC_24);
    P06_QUASI_MODE;
	  ENABLE_TIMER1_MODE0;                           /* Timer 0 mode configuration */
    TIMER0_FSYS;
    UART_Open(24000000,UART0_Timer3,115200);
	  ENABLE_TIMER0_INTERRUPT;                       /* enable Timer0 interrupt  */ 
    ENABLE_GLOBAL_INTERRUPT;                       /* enable interrupts */
    set_TCON_TR0;                                  /* Timer0 run */
		SPI_Initial();
		RED_LED = GREEN_LED = BLUE_LED = 0;
	  BLUE_LED = 1;
	  RED_LED = GREEN_LED = 0;
	  sw_delay(3000);
	  MFRC522_Init();
	//	Read_DATAFLASH_ARRAY(0x38E0,&ArrRead1,5); 
	//	Read_DATAFLASH_ARRAY(0x38F0,&ArrRead2,5);
while(1)
{
	   if (millisecondRoutine == 1)
	 {
			millisecondRoutine = 0;
			App__AdcDataSamplingHandling();
	 }
		/*if(uart0_receive_flag){
				i++;
				ArrayData[i]  = uart0_receive_data;
				uart0_receive_flag = 0;
		}
	 */
	  status = MFRC522_Request(PICC_REQIDL, str);	//MFRC522_Request(0x26, str)
		status = MFRC522_Anticoll(str);	//Take a collision, look up 5 bytes
	  

	  if (status == MI_OK)
		{
					BLUE_LED = GREEN_LED = RED_LED = 0;	
					sw_delay(200);
					BLUE_LED = 1;//	
					sw_delay(200);
					BLUE_LED = 0;//	
					sw_delay(200);
					BLUE_LED = 1;//	
			
		   if(((str[0] == ArrRead1[0] ) &&
						(str[1] == ArrRead1[1]) &&
						(str[2] == ArrRead1[2]) &&
						(str[3] == ArrRead1[3]))||
						((str[0] == ArrRead2[0]) &&
						(str[1] == ArrRead2[1]) &&
						(str[2] == ArrRead2[2]) &&
						(str[3] == ArrRead2[3])))
			   {
				    if(flag == 0)
						{
						  CardFiller(&CardAUTH,&str);
				    }
				 
				 if((str[0] == CardAUTH[0]) 
					&&(str[1] == CardAUTH[1])
				  &&(str[2] == CardAUTH[2])
				  &&(str[3] == CardAUTH[3]))
				 {
					 OverloadRetry = 0;
						   	if(flag == 0){
									Auth_flag = 1;
									
							  }
							if(flag == 1){
								  Auth_flag = 0;
							 }
							flag = ~flag;
					  }	
            sw_delay(2000);				 
				 }
				 else
				 {
					printf("UNAUTHORIZED CARD \r\n");
				 }
		 }
	   else
	   {
		   MFRC522_Init();
			 sw_delay(25);
		 }
	///////////////// Relay Control Section ///////////////
		 if(Auth_flag == 1 && Overload_flag==0 && Voltage_Over_flag==0 && Voltage_Under_flag==0)
     {
				//P13 = 1;
				//P12 = P14 = 0;
				RELAY_1 = 1;
				//P05 = 1;
			 RelayState = 1;
			//	sw_delay(2000);
     }
     else
     {
			// P13 = P12 = 0;
			// P14 = 1; 
			 RELAY_1 = 0;
			// P05 = 0; 
			 RelayState = 0;
     }
////////////////////////End ////////////////
		 
//////////////////////LED Indication ///////////

 if(RelayState == 1)
 {
	 GREEN_LED = 1;
	 BLUE_LED = RED_LED = 0;
	 
 }
 else 
 {
	 if ( Emergency_flag == 1 )
	 {
			GREEN_LED = 0;
			RED_LED = 1;
			BLUE_LED = 0;
		 
	 }
	 else if (Overload_flag==1 ||  Voltage_Over_flag == 1 ||  Voltage_Under_flag == 1)
	 {
			GREEN_LED = 0;
			BLUE_LED = 0;
			GREEN_LED = 0;
		 sw_delay(500);
			RED_LED = 1;
			sw_delay(500);
			RED_LED = 0;	 
	 }
	 else //if ( Auth_flag == 0 )
	 {
		 GREEN_LED = 0;
			RED_LED = 0;
			BLUE_LED = 1;
		 
	 }
	 
	 
	 
	 
	 
 }


//////////////		 
	
}
 
}

// Software based delay. Time is not accurate.
void sw_delay (int ms){
    int a, b;
    for (a=0; a<1296; a++){
             for (b=0; b<ms; b++);
		}
}