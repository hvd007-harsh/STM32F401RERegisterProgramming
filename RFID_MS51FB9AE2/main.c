#include "MS51_16K.H"
#include "spi.h"
#include "uart.h"
#include "sys.h"
#include "common.h"
#include "rfid.h"
#include "delay.h"
#include "adc.h"
 
 
 unsigned char status = '\0';
 unsigned char str[7]; // Max_LEN = 16

 
void sw_delay(int ms);
void SPI_Initial(void);
/*                
main function
*/
void main (void)
{
  	ALL_GPIO_QUASI_MODE;
  	/* Modify HIRC to 24MHz for UART baud rate function only */
    MODIFY_HIRC(HIRC_24);
    P06_QUASI_MODE;
    UART_Open(24000000,UART0_Timer3,115200);
    ENABLE_UART0_PRINTF;
	  SPI_Initial();
	  Timer0_Delay(24000000,200,3000);
	  MFRC522_Init();
	  Timer0_Delay(24000000,200,2000);
  	printf("\n Hello world!");   // Important! use prinft function must set TI=1;

while(1)
{
	  Timer0_Delay(24000000,200,500);
	  status = MFRC522_Request(PICC_REQIDL, str);	//MFRC522_Request(0x26, str)
   	printf("\n Hello world!");
		status = MFRC522_Anticoll(str);	//Take a collision, look up 5 bytes
	  printf("Status : %d \n",status);
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
