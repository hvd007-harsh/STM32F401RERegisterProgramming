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
bit receiveFlag,bufOverFlag;

void Serial_ISR(void) interrupt 4
{
    _push_(SFRS);
  
    if (RI)
    {
        uart0_receive_flag = 1;
        uart0_receive_data = SBUF;
			  clr_SCON_RI;                                         // Clear RI (Receive Interrupt).
    }

    if (TI)
    {
        if (!PRINTFG)
        {
            TI = 0;
        }
    }
    _pop_(SFRS);
}

unsigned char status = '\0';
float ADCDataIN6, ADCDataIN7;
unsigned char str[7];
unsigned char CardAUTH[7];
unsigned char i,j, k;
unsigned char ArrayData[50],ReceivedData1[5],ReceivedData2[5],ArrRead1[5],ArrRead2[5] ;

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
	  ENABLE_UART0_INTERRUPT;
	  ENABLE_GLOBAL_INTERRUPT;
		ENABLE_UART0_PRINTF;
	  SPI_Initial();
	  P12 = 1;
	  P13 = P14 = 0;
	  sw_delay(3000);
	  MFRC522_Init();
		Read_DATAFLASH_ARRAY(0x38E0,&ArrRead1,5); 
		Read_DATAFLASH_ARRAY(0x38F0,&ArrRead2,5);
while(1)
{
		if(uart0_receive_flag){
				i++;
				ArrayData[i]  = uart0_receive_data;
				uart0_receive_flag = 0;
		}
		if(i >= 20)
		{
		  for(j = 0; j <= 20; j++){
         if((ArrayData[j] == 'C') && (ArrayData[j+1] == 'O') && (ArrayData[j+2] == 'R') && (ArrayData[j+3]=='F') && (ArrayData[j+4]=='#'))
         {
					 if(ArrayData[j+5] == '1')
					 {
						 ReceivedData1[0] = ArrayData[j+6];
						 ReceivedData1[1] = ArrayData[j+7];
						 ReceivedData1[2] = ArrayData[j+8];
						 ReceivedData1[3] = ArrayData[j+9];
						 ReceivedData1[4] = ArrayData[j+10];
					 }
					 if(ArrayData[j+5] == '2')
					 {
						 ReceivedData2[0] = ArrayData[j+6];
						 ReceivedData2[1] = ArrayData[j+7];
						 ReceivedData2[2] = ArrayData[j+8];
						 ReceivedData2[3] = ArrayData[j+9];
						 ReceivedData2[4] = ArrayData[j+10];
					 }
					 if( ArrayData[j+5] == '1'){
					   Write_DATAFLASH_ARRAY(0x38E0,ReceivedData1,5); //write 5 bytes 					
					 }
					 if(ArrayData[j+5] == '2'){
					   Write_DATAFLASH_ARRAY(0x38F0,ReceivedData2,5);
					 }
					 i = 0;
					 memset(ArrayData ,'\0',sizeof(ArrayData));
					 Read_DATAFLASH_ARRAY(0x38E0,&ArrRead1,5); 
					 Read_DATAFLASH_ARRAY(0x38F0,&ArrRead2,5);
					 
					 for( k = 0; k <= 5; k++)
					 {
						 printf("%x",ArrRead1[i]);
					 }
					 for(k = 0; k <= 5; k++)
					 {
						 printf("%x",ArrRead2[i]); 
			     } 
				 }
			}
		}
		
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