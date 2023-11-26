#include "hw.h"



/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef SpiHandle;
volatile bool blockingDmaFlag;

void SPIx_MspInit(void)
{
  GPIO_InitTypeDef  gpioinitstruct = {0};
  
  /*** Configure the GPIOs ***/  
  /* Enable GPIO clock */
  SPIn_SCK_GPIO_CLK_ENABLE();
  SPIn_MISO_MOSI_GPIO_CLK_ENABLE();

  /* Configure SPI SCK */
  gpioinitstruct.Pin        = SPIn_SCK_PIN;
  gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
  gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPIn_SCK_GPIO_PORT, &gpioinitstruct);

  /* Configure SPI MISO and MOSI */ 
  gpioinitstruct.Pin        = SPIn_MOSI_PIN;
  HAL_GPIO_Init(SPIn_MISO_MOSI_GPIO_PORT, &gpioinitstruct);
  
  gpioinitstruct.Pin        = SPIn_MISO_PIN;
  gpioinitstruct.Mode       = GPIO_MODE_INPUT;
  HAL_GPIO_Init(SPIn_MISO_MOSI_GPIO_PORT, &gpioinitstruct);

  gpioinitstruct.Pin        = SPIn_CS_PIN;
  gpioinitstruct.Mode       = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(SPIn_CS_GPIO_PORT, &gpioinitstruct);
  SPIn_CS_HIGH();
  
  /*** Configure the SPI peripheral ***/ 
  /* Enable SPI clock */
  SPIn_CLK_ENABLE();
}

void SpiInit( void )
{
	if(HAL_SPI_GetState(&SpiHandle) == HAL_SPI_STATE_RESET)
	{
		/* SPI Config */
		SpiHandle.Instance = SPIn;
		SpiHandle.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_256;
		SpiHandle.Init.Direction          = SPI_DIRECTION_2LINES;
		SpiHandle.Init.CLKPhase           = SPI_PHASE_1EDGE;
		SpiHandle.Init.CLKPolarity        = SPI_POLARITY_LOW;
		SpiHandle.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE;
		SpiHandle.Init.CRCPolynomial      = 7;
		SpiHandle.Init.DataSize           = SPI_DATASIZE_8BIT;
		SpiHandle.Init.FirstBit           = SPI_FIRSTBIT_MSB;
		SpiHandle.Init.NSS                = SPI_NSS_SOFT;
		SpiHandle.Init.TIMode             = SPI_TIMODE_DISABLE;
		SpiHandle.Init.Mode               = SPI_MODE_MASTER;
    
		SPIx_MspInit();
		HAL_SPI_Init(&SpiHandle);
	}
}
void SpiDeInit( void )
{
    HAL_SPI_DeInit( &SpiHandle );
}

#define WAIT_FOR_BLOCKING_FLAG         while( blockingDmaFlag ) { }
/*!
 * @brief Sends txBuffer and receives rxBuffer
 *
 * @param [IN] txBuffer Byte to be sent
 * @param [OUT] rxBuffer Byte to be sent
 * @param [IN] size Byte to be sent
 */
void SpiInOut( uint8_t *txBuffer, uint8_t *rxBuffer, uint16_t size )
{
    //HAL_SPIEx_FlushRxFifo( &SpiHandle );
    HAL_SPI_TransmitReceive( &SpiHandle, txBuffer, rxBuffer, size, HAL_MAX_DELAY );
}

void SpiIn( uint8_t *txBuffer, uint16_t size )
{
    HAL_SPI_Transmit( &SpiHandle, txBuffer, size, HAL_MAX_DELAY );
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    blockingDmaFlag = false;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    blockingDmaFlag = false;
}
