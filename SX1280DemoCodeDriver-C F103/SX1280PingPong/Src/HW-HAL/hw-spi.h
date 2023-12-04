#ifndef __HW_SPI_H__
#define __HW_SPI_H__

#include <stdint.h>

#define SPIn                                 SPI1
#define SPIn_CLK_ENABLE()                    __HAL_RCC_SPI1_CLK_ENABLE()

#define SPIn_SCK_GPIO_PORT                   GPIOA
#define SPIn_SCK_PIN                  		 GPIO_PIN_5
#define SPIn_SCK_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIn_SCK_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

#define SPIn_MISO_MOSI_GPIO_PORT             GPIOA
#define SPIn_MISO_MOSI_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIn_MISO_MOSI_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define SPIn_MISO_PIN                        GPIO_PIN_6
#define SPIn_MOSI_PIN                        GPIO_PIN_7

#define SPIn_CS_PIN                          GPIO_PIN_4
#define SPIn_CS_GPIO_PORT                    GPIOA
#define SPIn_CS_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIn_CS_GPIO_CLK_DISABLE()           __HAL_RCC_GPIOA_CLK_DISABLE()

#define SPIn_CS_LOW()       HAL_GPIO_WritePin(SPIn_CS_GPIO_PORT, SPIn_CS_PIN, GPIO_PIN_RESET)
#define SPIn_CS_HIGH()      HAL_GPIO_WritePin(SPIn_CS_GPIO_PORT, SPIn_CS_PIN, GPIO_PIN_SET)

void SpiInit( void );

void SpiDeInit( void );

void SpiInOut( uint8_t *txBuffer, uint8_t *rxBuffer, uint16_t size );

void SpiIn( uint8_t *txBuffer, uint16_t size );

#endif // __HW_SPI_H__
