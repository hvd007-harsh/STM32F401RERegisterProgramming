#ifndef __HW_GPIO_H__
#define __HW_GPIO_H__

#define RADIO_NSS_PIN       GPIO_PIN_4
#define RADIO_NSS_PORT      GPIOA

//#define RADIO_MOSI_PIN      GPIO_PIN_7
//#define RADIO_MOSI_PORT     GPIOA

//#define RADIO_MISO_PIN      GPIO_PIN_6
//#define RADIO_MISO_PORT     GPIOA

//#define RADIO_SCK_PIN       GPIO_PIN_5
//#define RADIO_SCK_PORT      GPIOA

#define RADIO_nRESET_PIN    GPIO_PIN_0
#define RADIO_nRESET_PORT   GPIOA

#define RADIO_BUSY_PIN      GPIO_PIN_3
#define RADIO_BUSY_PORT     GPIOB

#define RADIO_DIOx_PIN      GPIO_PIN_4
#define RADIO_DIOx_PORT     GPIOB

//#define USART_TX_PIN        GPIO_PIN_2
//#define USART_TX_PORT       GPIOA

//#define USART_RX_PIN        GPIO_PIN_3
//#define USART_RX_PORT       GPIOA

#define ANT_SW_PIN          GPIO_PIN_0
#define ANT_SW_PORT         GPIOB

#define LED_RX_PIN          GPIO_PIN_0
#define LED_RX_PORT         GPIOC

#define LED_TX_PIN          GPIO_PIN_1
#define LED_TX_PORT         GPIOC





typedef void( GpioIrqHandler )( void );

/*!
 * GPIO IRQ handler function prototype
 */
IRQn_Type MSP_GetIRQn( uint16_t gpioPin );

void GpioInit( void );

void GpioDeInit( void );

void GpioWrite( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,  uint32_t value );

uint32_t GpioRead( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin );

uint8_t GpioGetBitPos( uint16_t GPIO_Pin );

void GpioSetIrq( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t prio,  GpioIrqHandler *irqHandler );

void GpioLaunchIrqHandler( uint16_t GPIO_Pin );

#endif // __HW_GPIO_H__
