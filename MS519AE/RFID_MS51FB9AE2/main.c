#include "MS51_16K.H"
#include "Delay.h"

 
void sw_delay(int ms);


//-------------------SPI_INITIAL----------------------

void SPI_Initial(void)
{

             P15_Quasi_Mode;       // P15 (SS) Quasi mode
             P10_Quasi_Mode;       // P10(SPCLK) Quasi mode
             P00_Quasi_Mode;       // P00 (MOSI) Quasi mode
            P01_Quasi_Mode;        // P22 (MISO) Quasi mode
            set_DISMODF;            // SS General purpose I/O ( No Mode Fault )
            clr_SSOE;
            clr_LSBFE;                 // MSB first      
            clr_CPOL;                  // The SPI clock is low in idle mode
            set_CPHA;                 // The data is sample on the second edge of SPI clock
            set_MSTR;                 // SPI in Master mode
            SPICLK_DIV2;         // Select SPI clock
            Enable_SPI_Interrupt;          // Enable SPI interrupt
            set_SPIEN;                           // Enable SPI function
}

void Start_Sending_SPI(unsigned char data)
{
	SS=0; 
	SPDR= data;
	clr_SPDR;
	SS=1;
	
}

/*                
main function
*/
void main (void)
{
	set_ALL_GPIO_Quasi_Mode;
	InitialUART0_Timer1(115200);
	SPI_Initial();
 while(1)
 {
	 P12 = 0xFF;
	 sw_delay(4000); 
	 P12 = 0x00;
	 sw_delay(4000);
 }
 
}
// Software based delay. Time is not accurate.
void sw_delay (int ms){
    int a, b;
    for (a=0; a<1296; a++){
             for (b=0; b<ms; b++);
}
}
