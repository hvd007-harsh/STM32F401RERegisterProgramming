#include "spi.h"
#include "MS51_16K.H"


void SPI_Initial(void)
{      
    P15_QUASI_MODE;                                  // P15 (SS) Quasi mode
    P10_QUASI_MODE;                                  // P10 (SPCLK) Quasi mode
    P00_QUASI_MODE;                                  // P00 (MOSI) Quasi mode
    P01_QUASI_MODE;                                  // P01 (MISO) Quasi mode                                // P01 (MISO) Quasi mode
    
    clr_SPSR_DISMODF;                                // SS General purpose I/O ( No Mode Fault ) 
    clr_SPCR_SSOE;
   
    clr_SPCR_LSBFE;                                  // MSB first

    clr_SPCR_CPOL;                                   // The SPI clock is low in idle mode
    set_SPCR_CPHA;                                   // The data is sample on the second edge of SPI clock 
    
    set_SPCR_MSTR;                                   // SPI in Master mode 
    SPICLK_FSYS_DIV2;                                    // Select SPI clock 
    set_SPCR_SPIEN;                                  // Enable SPI function 
    clr_SPSR_SPIF;
}


void SPI_Tx(unsigned char Data)
{ 
	 SPDR = Data;
	 while(!(SPSR & 0x80));
	 clr_SPSR_SPIF;
}

unsigned char SPI_Rx(unsigned char Rx_Buf){
	  unsigned char u8SpiRB;
	  SPDR = Rx_Buf;
	  while(!(SPSR & 0x80));
    u8SpiRB = SPDR;
    clr_SPSR_SPIF;
		return u8SpiRB;
}
