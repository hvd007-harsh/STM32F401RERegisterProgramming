#include "MS51_16K.h"

#define MGS4 P05
#define MGS3 P06 
#define MGS2 P07
#define MGS1 P30

#define SW4 P17
#define SW3 P16
#define SW2 P15
#define SW1 P14

#define LD4 P10
#define LD3 P11
#define LD2 P12 
#define LD1 P13

#define BZ  P00

#define RL1 P04 
#define RL2 P03
#define RL3 P02
#define RL4 P01


unsigned char temp _at_ 0x08;
unsigned char idata itemp _at_ 0x90;
unsigned char xdata xtemp _at_ 0x80;

unsigned long counter=0, RelayCounter =0;
bit sw_flag = 1;

void sw_delay(int ms);
void setup(void);


/* if define TIMER0_FSYS_DIV12, timer = (0x1FFF-0x1000)*12/24MHz = 4.08ms */
/* if define TIMER0_FSYS, timer = (0x1FFF-0x0010)/24MHz = 340us */
#define TH0_INIT        0x00
#define TL0_INIT        0x10 

/************************************************************************************************************
*    TIMER 0 interrupt subroutine
************************************************************************************************************/
void Timer0_ISR (void) interrupt 1           /*interrupt address is 0x000B */
{
    _push_(SFRS);
  
    TH0 = TH0_INIT;
    TL0 = TL0_INIT;
    TF0 = 0 ;
	  RelayCounter++;
	if( MGS1 == 1 || MGS2==1 || MGS3==1 || MGS4 ==1)
  {		
  	counter++;
	}
	else
	{
		counter=0;
	}
	
	if((counter > 0x993) && ((LD4 == 1)||(LD3 == 1) ||(LD2 == 1) ||(LD1 == 1)))
	 {
			BZ = 1;
			sw_delay(500);
			BZ = 0;
			sw_delay(500); 
			BZ = 1;
			sw_delay(500);
			BZ = 0;
	//						}
	}else
	{
			counter =0;
			BZ = 0;
	}
    _pop_(SFRS);
}


void main(void){
  /* UART0 initial for printf */
	setup();
	ENABLE_TIMER1_MODE0;                           /* Timer 0 mode configuration */
	TIMER0_FSYS_DIV12;
	TH0 = TH0_INIT;
  TL0 = TL0_INIT;
	ENABLE_TIMER0_INTERRUPT;			/* Timer 0 mode configuration */
	ENABLE_GLOBAL_INTERRUPT;                       /* enable interrupts */

	WDT_Open(256);
  WDT_Interrupt(Disable);
	RL1 = RL2 = RL3 = RL4 = BZ=  0;
	LD4 = LD3 = LD2 = LD1 =  1 ; 
	 
	
while(1){
	//If MGS pin is not connected then switch will be 0 if it is not then it is 1
	 if((MGS4 == 1)||
			(MGS3 == 1)||
			(MGS2 == 1)||
			(MGS1 == 1))
		{
		   sw_flag = 0;
		}
		else
		{
			sw_flag = 1; 
		}

		
			//Switch 4
		 if(SW4 == 0x00)
		 { 
				//Checking all MGS 
				if((MGS4 == 0)&&
					(MGS3 == 0)&&
					(MGS2 == 0)&&
					(MGS1== 0)&&((RL1 == 0 )||(RL2 == 0 )||(RL3 == 0)||(RL4 ==0 )))
				{
					RL4 = 1;
					RelayCounter =0;
				}
				//Ring the buzzer
				BZ = 1;
				sw_delay(100);
				BZ = 0; 
				sw_delay(100); 
				BZ = 1;
				sw_delay(100); 
				BZ = 0;
				ENABLE_TIMER0_INTERRUPT;
				set_TCON_TR0; 
				while(RelayCounter < 1225){
			     if((MGS4 == 1)||
							(MGS3 == 1)||
							(MGS2 == 1)||
							(MGS1== 1)){
							    RL4 = 0;
								  break; 
							}
							
					 if((SW4==0) || (SW3==0) || (SW2==0) || (SW1==0))
					 {
						  BZ = 1;
					 }
					 else
					 {
						 BZ =0;
					 }
			 }
			 RL4 = 0;
			 BZ = 0;
		 }
		 
		 
		 if(SW2 == 0x00)
		 {
			 if((MGS4 == 0)&&
				 (MGS3 == 0)&&
				 (MGS2 == 0)&&
	  		 (MGS1== 0)&&((RL1 == 0 )||(RL2 == 0 )||(RL3 == 0)||(RL4 ==0 )))
		  {
				RL2 = 1; 
				RelayCounter =0;
			}
			 BZ = 1;
			 sw_delay(100); 
			 BZ = 0;
			 ENABLE_TIMER0_INTERRUPT;
			 set_TCON_TR0; 
			 while(RelayCounter < 1225){
			     if((MGS4 == 1)||
							(MGS3 == 1)||
							(MGS2 == 1)||
							(MGS1== 1)){
							    RL2 = 0;
									DISABLE_TIMER0_INTERRUPT; 
									clr_TCON_TR0;
								  break; 
							}
						if((SW4==0) || (SW3==0) || (SW2==0) || (SW1==0))
					 {
						  BZ = 1;
					 }
					 else
					 {
						 BZ =0;
					 }
			 }
			 RL2 = 0; 
			 BZ = 0;
		 }
		 if(SW3 == 0)
		 {
			 if((MGS4 == 0)&&
				 (MGS3 == 0)&&
				 (MGS2 == 0)&&
	  		 (MGS1== 0)&&((RL1 == 0 )||(RL2 == 0 )||(RL3 == 0)||(RL4 ==0 )))
				{
					RL3 = 1;
					RelayCounter =0;
				}
				BZ = 1;
				sw_delay(100);
				BZ = 0; 
				sw_delay(100); 
				BZ = 1;
				sw_delay(100); 
				BZ = 0;
				ENABLE_TIMER0_INTERRUPT;
				set_TCON_TR0;
			 while(RelayCounter < 1225){
			     if((MGS4 == 1)||
							(MGS3 == 1)||
							(MGS2 == 1)||
							(MGS1== 1))
							{
							    RL3 = 0;
									DISABLE_TIMER0_INTERRUPT; 
									clr_TCON_TR0;
								  break; 
							}
						if((SW4==0) || (SW3==0) || (SW2==0) || (SW1==0))
					 {
						  BZ = 1;
					 }
					 else
					 {
								BZ = 0;
					 }
			 }
			 RL3 = 0;
			 BZ = 0;
		 }
		 if(SW1 == 0)
		 {
			 if((MGS4 == 0)&&
				 (MGS3 == 0)&&
				 (MGS2 == 0)&&
	  		 (MGS1== 0)&&((RL1 == 0 )||(RL2 == 0 )||(RL3 == 0)||(RL4 ==0 )))
				{
					RL1 = 1;
					RelayCounter =0;
				}
				BZ = 1;
				sw_delay(100);
				BZ = 0; 
				sw_delay(100); 
				BZ = 1;
				sw_delay(100); 
				BZ = 0;
				ENABLE_TIMER0_INTERRUPT;
				set_TCON_TR0;
				while(RelayCounter < 1225){
						if((MGS4 == 1)||
							(MGS3 == 1)||
							(MGS2 == 1)||
							(MGS1== 1))
							{
							    RL1 = 0;
									DISABLE_TIMER0_INTERRUPT; 
									clr_TCON_TR0;
								  break; 
							}
							if((SW4==0) || (SW3==0) || (SW2==0) || (SW1==0))
							{
								BZ = 1;
						  }
							else
							{
								BZ = 0;
							}
			 } 
			 RL1 = 0;
			 BZ = 0;
		 }
			
		 
		 if(MGS4 == 0 )
		 {
       LD4 = 0; 
		 }
		 else
		 {
			 LD4 = 1; 
		 }
		 if(MGS3 == 0)
		 {
			 LD3 = 0;
		 }
		 else
		 {
			 LD3= 1; 
		 }
		 if(MGS2 == 0)
		 {
			 LD2 = 0; 
		 }
		 else
		 {
			 LD2 = 1; 
		 }
		 if(MGS1== 0)
		 {
			 LD1 = 0;
		 }
		 else
		 {
			 LD1 = 1;
		 }
		}
}

void sw_delay (int ms){
    int a, b;
    for (a=0; a<1296; a++){
         for (b=0; b<ms; b++);
		}
}


void setup (void){
	ALL_GPIO_PUSHPULL_MODE;
}