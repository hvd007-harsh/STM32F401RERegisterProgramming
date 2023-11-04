#include <MS51_16K.H>
#include "spi.h"
#include "uart.h"
#include "sys.h"
#include "common.h"
#include "rfid.h"
#include "delay.h"
 
 
 unsigned char status = '\0';
 unsigned int ADCDataIN6, ADCDataIN7;
 unsigned char str[7]; // Max_LEN = 16

 
void sw_delay(int ms);
void SPI_Initial(void);
/*                
main function
*/
void main (void)
{
  	/* Modify HIRC to 24MHz for UART baud rate function only */
	  ALL_GPIO_QUASI_MODE;
    MODIFY_HIRC(HIRC_24);
    P06_QUASI_MODE;
    UART_Open(24000000,UART0_Timer3,115200);
    ENABLE_UART0_PRINTF;
	  SPI_Initial();
	  printf("RFID is starting\r\n");
	  printf("......Initialization\n");
	  sw_delay(3000);
	  MFRC522_Init();
	  sw_delay(2000);
	printf("let's go\r\n");
while(1)
{
	
	  Timer0_Delay(24000000,200,500);
	  status = MFRC522_Request(PICC_REQIDL, str);	//MFRC522_Request(0x26, str)
		status = MFRC522_Anticoll(str);	//Take a collision, look up 5 bytes
	  //ADCDataIN6 = ADC_CH6_Read();
	  //ADCDataIN7 = ADC_CH7_Read();
	 // printf("ADCValue : 0x%x 0x%x\n",ADCDataIN6,ADCDataIN7);
	  if (status == MI_OK)
		{
		   printf(" UID : 0x%x 0x%x 0x%x 0x%x 0x%x \r\n",str[0],str[1],str[2],str[3],str[4],str[5]);
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
