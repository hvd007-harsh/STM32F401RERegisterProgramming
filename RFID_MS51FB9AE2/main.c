#include <MS51_16K.H>
#include "spi.h"
#include "uart.h"
#include "sys.h"
#include "common.h"
#include "rfid.h"
#include "delay.h"
#include "adc.h"
 
unsigned char temp _at_ 0x08;
unsigned char idata itemp _at_ 0x90;
unsigned char xdata xtemp _at_ 0x80;
 
 
 unsigned char status = '\0';
 float ADCDataIN6, ADCDataIN7;
 unsigned char str[7];
 unsigned char CardAUTH[7]; 
 bit flag = 0;
 
void sw_delay(int ms);
void SPI_Initial(void);


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
    UART_Open(24000000,UART0_Timer3,115200);
    ENABLE_UART0_PRINTF;
	  SPI_Initial();
	  P12 = 1;
	  P13 = P14 = 0;
	  sw_delay(3000);
	  MFRC522_Init();
	  sw_delay(2000);
while(1)
{
	  sw_delay(500);
	  status = MFRC522_Request(PICC_REQIDL, str);	//MFRC522_Request(0x26, str)
		status = MFRC522_Anticoll(str);	//Take a collision, look up 5 bytes
	  ADCDataIN6 = ADC_CH6_Read();
	  ADCDataIN7 = ADC_CH7_Read();
	  //Current 
	  ADCDataIN6 = ADCDataIN6 * 0.00003662109;
	  //Voltage
	  ADCDataIN7 = ADCDataIN7 * 0.00078125;
	  if (status == MI_OK)
		{
		   if(((str[0] == 0XD3 ) &&
						(str[1] == 0x2) &&
						(str[2] == 0x3) &&
						(str[3] == 0xAD))||
						((str[0] == 0X43 ) &&
						(str[1] == 0x35) &&
						(str[2] == 0x05) &&
						(str[3] == 0xAD)))
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
								P04 = 1;
								P05 = 1;
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
}
 
}

// Software based delay. Time is not accurate.
void sw_delay (int ms){
    int a, b;
    for (a=0; a<1296; a++){
             for (b=0; b<ms; b++);
		}
}