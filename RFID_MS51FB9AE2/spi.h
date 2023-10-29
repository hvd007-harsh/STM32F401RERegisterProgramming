#ifdef __SPI_H
#define __SPI_H

void SPI_Initial(void);
void SPI_Tx(unsigned char Data);
unsigned char SPI_Rx(unsigned char Rx_Buf);

#endif
