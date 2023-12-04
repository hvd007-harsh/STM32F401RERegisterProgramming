/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* SPDX-License-Identifier: Apache-2.0                                                                     */
/* Copyright(c) 2020 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************
//  File Function: MS51 UART0 receive and transmit loop test
/***********************************************************************************************************/
#include "MS51_16K.H"

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

/************************************************************************************************************/
/*  Main function                                                                                           */
/************************************************************************************************************/
 void main(void)
{
	
unsigned char i,j;
unsigned char ArrayData[50],ReceivedData1[5],ReceivedData2[5] ;
	
	  ALL_GPIO_QUASI_MODE;
/* Modify HIRC to 24MHz for UART printf function only */
    MODIFY_HIRC(HIRC_24);
    P06_QUASI_MODE;
    UART_Open(24000000,UART0_Timer3,115200);
    ENABLE_UART0_INTERRUPT;                                  /* Enable UART0 interrupt */
    ENABLE_GLOBAL_INTERRUPT;
    ENABLE_UART0_PRINTF;
	/* Global interrupt enable */
/* while receive data from RXD, send this data to TXD */
  while(1)
  {
    if (uart0_receive_flag)
    {
			i++;
      GPIO_LED ^= 1;
      ArrayData[i]  = uart0_receive_data;
      uart0_receive_flag = 0;
    }
		if(i >= 50)
		{
		  for(j = 0; j <= 50; j++){
         if((ArrayData[i] == 'C') && (ArrayData[i+1] == 'O') && (ArrayData[i+2] == 'R') && (ArrayData[i+3]=='F') && (ArrayData[i+4]=='#'))
         {
					 if(ArrayData[i+5] == '1')
					 {
						 ReceivedData1[0] = ArrayData[i+6];
						 ReceivedData1[1] = ArrayData[i+7];
						 ReceivedData1[2] = ArrayData[i+8];
						 ReceivedData1[3] = ArrayData[i+9];
						 ReceivedData1[4] = ArrayData[i+10];
					 }
					 if(ArrayData[i+5] == '2')
					 {
						 ReceivedData2[0] = ArrayData[i+6];
						 ReceivedData2[1] = ArrayData[i+7];
						 ReceivedData2[2] = ArrayData[i+8];
						 ReceivedData2[3] = ArrayData[i+9];
						 ReceivedData2[4] = ArrayData[i+10];
					 }
				 }
			}
			i = 0;
		}
		
  }
}