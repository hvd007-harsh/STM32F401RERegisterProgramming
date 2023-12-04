/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

#include <string.h>
#include "main.h"
#include "hw.h"
#include "radio.h"
#include "sx1280.h"

/*!
 * \brief Used to display firmware version UART flow
 */
#define FIRMWARE_VERSION    ( ( char* )"Firmware Version: 170919A" )

/*!
 * Select mode of operation for the Ping Ping application
 */
//#define MODE_BLE
#define MODE_LORA
//#define MODE_GFSK
//#define MODE_FLRC


#define RF_BL_ADV_CHANNEL_38                        2410000000 // Hz

/*!
 * \brief Defines the nominal frequency
 */
#define RF_FREQUENCY                                RF_BL_ADV_CHANNEL_38 // Hz

/*!
 * \brief Defines the output power in dBm
 *
 * \remark The range of the output power is [-18..+13] dBm
 */
#define TX_OUTPUT_POWER                             10//13

/*!
 * \brief Defines the buffer size, i.e. the payload size
 */
#define BUFFER_SIZE                                 10//20

/*!
 * \brief Number of tick size steps for tx timeout
 */
#define TX_TIMEOUT_VALUE                            20000//10000 // ms

/*!
 * \brief Number of tick size steps for rx timeout
 */
#define RX_TIMEOUT_VALUE                            1000 // ms

/*!
 * \brief Size of ticks (used for Tx and Rx timeout)
 */
#define RX_TIMEOUT_TICK_SIZE                        RADIO_TICK_SIZE_1000_US

/*!
 * \brief Defines the size of the token defining message type in the payload
 */
#define PINGPONGSIZE                                4


/*!
 * \brief Defines the states of the application
 */
typedef enum
{
    APP_LOWPOWER,
    APP_RX,
    APP_RX_TIMEOUT,
    APP_RX_ERROR,
    APP_TX,
    APP_TX_TIMEOUT,
}AppStates_t;


/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( void );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( IrqErrorCode_t );

void OnRangingDone( IrqRangingCode_t val );

/*!
 * \brief Define the possible message type for this application
 */
// const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";
uint8_t PingMsg[10]={'s','e','n','d','_','t','e','s','t'};
/*!
 * \brief All the callbacks are stored in a structure
 */
RadioCallbacks_t Callbacks =
{
    &OnTxDone,        // txDone
    &OnRxDone,        // rxDone
    NULL,             // syncWordDone
    NULL,             // headerDone
    &OnTxTimeout,     // txTimeout
    &OnRxTimeout,     // rxTimeout
    &OnRxError,       // rxError
    &OnRangingDone,   // rangingDone
    NULL,             // cadDone
};

/*!
 * \brief The size of the buffer
 */
uint8_t BufferSize = BUFFER_SIZE;

/*!
 * \brief The buffer
 */
uint8_t Buffer[BUFFER_SIZE];

/*!
 * \brief Mask of IRQs to listen to in rx mode
 */
uint16_t RxIrqMask = IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT;

/*!
 * \brief Mask of IRQs to listen to in tx mode
 */
uint16_t TxIrqMask = IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT;

uint16_t Ranging_M_IrqMask = IRQ_RANGING_MASTER_RESULT_VALID|IRQ_RANGING_MASTER_RESULT_TIMEOUT;


/*!
 * \brief The State of the application
 */
AppStates_t AppState = APP_LOWPOWER;

#if defined( MODE_BLE )
/*!
 * \brief In case of BLE, the payload must contain the header
 */
typedef union
{
    struct BleAdvHeaderField_s
    {
        uint8_t pduType: 4;
        uint8_t rfu1:2;
        uint8_t txAddr:1;
        uint8_t rxAddr:1;
        uint8_t length:6;
        uint8_t rfu2:2;
    } Fields;
    uint8_t Serial[ 2 ];
}BleAdvHeaders_t;
BleAdvHeaders_t ble_header_adv;
#endif // MODE_BLE

PacketParams_t packetParams;

PacketStatus_t packetStatus;

ModulationParams_t modulationParams;

uint8_t restart = 1;
uint8_t retry=1;
uint8_t error;
double distance = 0.0;
#define MAX_ERROR  250
void Ranging_Master(void);


int main( void )
{
	bool isMaster = true;
    

    HwInit( );
    HAL_Delay( 500 );                   // let DC/DC power ramp up

    Radio.Init( &Callbacks );
    Radio.SetRegulatorMode( USE_DCDC ); // Can also be set in LDO mode but consume more power
    memset( &Buffer, 0x00, BufferSize );

    printf( "\nSX1280 Ping Pong Demo Application. %s\n", FIRMWARE_VERSION );
    printf( "\nRadio firmware version 0x%x\n", Radio.GetFirmwareVersion( ) );

#if defined( MODE_BLE )

    printf( "\nPing Pong running in BLE mode\n\r" );
    modulationParams.PacketType = PACKET_TYPE_BLE;
    modulationParams.Params.Ble.BitrateBandwidth = GFS_BLE_BR_1_000_BW_1_2;
    modulationParams.Params.Ble.ModulationIndex = GFS_BLE_MOD_IND_0_50;
    modulationParams.Params.Ble.ModulationShaping = RADIO_MOD_SHAPING_BT_0_5;

    packetParams.PacketType = PACKET_TYPE_BLE;
    packetParams.Params.Ble.BlePacketType = BLE_EYELONG_1_0;
    packetParams.Params.Ble.ConnectionState = BLE_ADVERTISER;
    packetParams.Params.Ble.CrcField = BLE_CRC_3B;
    packetParams.Params.Ble.Whitening = RADIO_WHITENING_ON;

#elif defined( MODE_GFSK )

    printf( "\nPing Pong running in GFSK mode\n\r" );
    modulationParams.PacketType = PACKET_TYPE_GFSK;
    modulationParams.Params.Gfsk.BitrateBandwidth = GFS_BLE_BR_0_125_BW_0_3;
    modulationParams.Params.Gfsk.ModulationIndex = GFS_BLE_MOD_IND_1_00;
    modulationParams.Params.Gfsk.ModulationShaping = RADIO_MOD_SHAPING_BT_1_0;

    packetParams.PacketType = PACKET_TYPE_GFSK;
    packetParams.Params.Gfsk.PreambleLength = PREAMBLE_LENGTH_32_BITS;
    packetParams.Params.Gfsk.SyncWordLength = GFS_SYNCWORD_LENGTH_5_BYTE;
    packetParams.Params.Gfsk.SyncWordMatch = RADIO_RX_MATCH_SYNCWORD_1;
    packetParams.Params.Gfsk.HeaderType = RADIO_PACKET_VARIABLE_LENGTH;
    packetParams.Params.Gfsk.PayloadLength = BUFFER_SIZE;
    packetParams.Params.Gfsk.CrcLength = RADIO_CRC_3_BYTES;
    packetParams.Params.Gfsk.Whitening = RADIO_WHITENING_ON;

#elif defined( MODE_LORA )

    printf( "\nPing Pong running in LORA mode\n\r" );
    modulationParams.PacketType = PACKET_TYPE_RANGING;//PACKET_TYPE_LORA;
    modulationParams.Params.LoRa.SpreadingFactor = LORA_SF10;//LORA_SF12;
    modulationParams.Params.LoRa.Bandwidth = LORA_BW_1600;//LORA_BW_0200;//LORA_BW_1600;
    modulationParams.Params.LoRa.CodingRate = LORA_CR_4_5;//LORA_CR_LI_4_7;

    packetParams.PacketType = PACKET_TYPE_RANGING;//PACKET_TYPE_LORA;
    packetParams.Params.LoRa.PreambleLength = 12;
    packetParams.Params.LoRa.HeaderType = LORA_PACKET_VARIABLE_LENGTH;
    packetParams.Params.LoRa.PayloadLength = BUFFER_SIZE;
    packetParams.Params.LoRa.CrcMode = LORA_CRC_ON;
    packetParams.Params.LoRa.InvertIQ = LORA_IQ_NORMAL;

#elif defined( MODE_FLRC )

    printf( "\nPing Pong running in FLRC mode\n\r" );
    modulationParams.PacketType = PACKET_TYPE_FLRC;
    modulationParams.Params.Flrc.BitrateBandwidth = FLRC_BR_0_260_BW_0_3;
    modulationParams.Params.Flrc.CodingRate = FLRC_CR_1_2;
    modulationParams.Params.Flrc.ModulationShaping = RADIO_MOD_SHAPING_BT_1_0;

    packetParams.PacketType = PACKET_TYPE_FLRC;
    packetParams.Params.Flrc.PreambleLength = PREAMBLE_LENGTH_32_BITS;
    packetParams.Params.Flrc.SyncWordLength = FLRC_SYNCWORD_LENGTH_4_BYTE;
    packetParams.Params.Flrc.SyncWordMatch = RADIO_RX_MATCH_SYNCWORD_1;
    packetParams.Params.Flrc.HeaderType = RADIO_PACKET_VARIABLE_LENGTH;
    packetParams.Params.Flrc.PayloadLength = BUFFER_SIZE;
    packetParams.Params.Flrc.CrcLength = RADIO_CRC_3_BYTES;
    packetParams.Params.Flrc.Whitening = RADIO_WHITENING_OFF;

#else
#error "Please select the mode of operation for the Ping Ping demo"
#endif

    Radio.SetStandby( STDBY_RC );
    Radio.SetPacketType( modulationParams.PacketType );
    Radio.SetModulationParams( &modulationParams );
    Radio.SetPacketParams( &packetParams );
    Radio.SetRfFrequency( RF_FREQUENCY );
    Radio.SetBufferBaseAddresses( 0x00, 0x00 );
    Radio.SetTxParams( TX_OUTPUT_POWER, RADIO_RAMP_02_US );
    
    //SX1280SetPollingMode( );

    #if defined( MODE_BLE )
        // only used in GENERIC and BLE mode
        Radio.SetSyncWord( 1, ( uint8_t[] ){ 0xDD, 0xA0, 0x96, 0x69, 0xDD } );
        Radio.WriteRegister(0x9c7, 0x55 );
        Radio.WriteRegister(0x9c8, 0x55 );
        Radio.WriteRegister(0x9c9, 0x55 );
        //Radio.WriteRegister( 0x9c5, 0x33 );
        Radio.SetBleAdvertizerAccessAddress( );
        Radio.SetWhiteningSeed( 0x33 );
        ble_header_adv.Fields.length = PINGPONGSIZE + 2;
        ble_header_adv.Fields.pduType = 2;
    #endif // MODE_BLE

    GpioWrite( LED_TX_PORT, LED_TX_PIN, 0 );
    GpioWrite( LED_RX_PORT, LED_RX_PIN, 0 );

    //Radio.SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
    //Radio.SetRx( ( TickTime_t ) { RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE } );
	
    AppState = APP_LOWPOWER;

    while( 1 )
    {
		Ranging_Master();
		
		#if 0
		if( BufferSize > 0 )
		{
			printf( "...Ping\r\n" );
			memcpy( Buffer, PingMsg, BufferSize );
		
			Radio.SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
			Radio.SendPayload( Buffer, BufferSize, ( TickTime_t ){ RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE } );
		}
		HAL_Delay(2000);
		
		
		SX1280ProcessIrqs( );
        switch( AppState )
        {
            case APP_RX:
                AppState = APP_LOWPOWER;
                GpioWrite( LED_RX_PORT, LED_RX_PIN, GpioRead( LED_RX_PORT, LED_RX_PIN ) ^ 1 );
                Radio.GetPayload( Buffer, &BufferSize, BUFFER_SIZE );
                #if defined( MODE_BLE )
                    // Remove the 2st bytes that are BLE header from Buffer
                    memcpy( Buffer, Buffer+2, PINGPONGSIZE );
                #endif // MODE_BLE
                if( isMaster == true )
                {
                    if( BufferSize > 0 )
                    {
                        if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, PINGPONGSIZE ) == 0 )
                        {
                            printf( "...Pong\r\n" );
                            #if defined( MODE_BLE )
                                memcpy( Buffer, ble_header_adv.Serial, 2 );
                                memcpy( Buffer+2, PingMsg, PINGPONGSIZE );
                            #else
                                memcpy( Buffer, PingMsg, PINGPONGSIZE );
                            #endif
                            Radio.SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                            Radio.SendPayload( Buffer, BufferSize, ( TickTime_t ){ RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE } );
                        }
                        else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, PINGPONGSIZE ) == 0 )
                        {
                            // A master already exists then become a slave
                            printf( "...Ping  -  switch to Slave\r\n" );
                            isMaster = false;
                            #if defined( MODE_BLE )
                                memcpy( Buffer, ble_header_adv.Serial, 2 );
                                memcpy( Buffer+2, PongMsg, PINGPONGSIZE );
                            #else
                                memcpy( Buffer, PongMsg, PINGPONGSIZE );
                            #endif
                            Radio.SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                            Radio.SendPayload( Buffer, BufferSize, ( TickTime_t ){ RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE } );
                        }
                        else // valid reception but neither a PING or a PONG message
                        {    // Set device as master ans start again
                            isMaster = true;
                            #if defined( MODE_BLE )
                                memcpy( Buffer, ble_header_adv.Serial, 2 );
                                memcpy( Buffer+2, PongMsg, PINGPONGSIZE );
                                Radio.SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                                Radio.SendPayload( Buffer, BufferSize, ( TickTime_t ){ RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE } );
                            #else
                                Radio.SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                                Radio.SetRx( ( TickTime_t ) { RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE } );
                            #endif
                        }
                    }
                }
                else
                {
                    if( BufferSize > 0 )
                    {
                        if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, PINGPONGSIZE ) == 0 )
                        {
                            printf( "...Ping\r\n" );
                            #if defined( MODE_BLE )
                                ble_header_adv.Fields.length = PINGPONGSIZE + 2;
                                memcpy( Buffer, ble_header_adv.Serial, 2 );
                                memcpy( Buffer+2, PongMsg, PINGPONGSIZE );
                            #else
                                memcpy( Buffer, PongMsg, PINGPONGSIZE );
                            #endif
                            Radio.SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                            Radio.SendPayload( Buffer, BufferSize, ( TickTime_t ){ RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE } );
                        }
                        else // valid reception but not a PING as expected
                        {
                            printf( "...Unexpected packet  -   switch to master\r\n" );
                            isMaster = true;
                            Radio.SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                            Radio.SetRx( ( TickTime_t ) { RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE } ); 
                        }
                    }
                }
                break;

            case APP_TX:
                AppState = APP_LOWPOWER;
                GpioWrite( LED_TX_PORT, LED_TX_PIN, GpioRead( LED_TX_PORT, LED_TX_PIN ) ^ 1 );
                if( isMaster == true )
                {
                    printf( "Ping...\r\n" );
                }
                else
                {
                    printf( "Pong...\r\n" );
                }
                Radio.SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                Radio.SetRx( ( TickTime_t ) { RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE } );
                break;

            case APP_RX_TIMEOUT:
                AppState = APP_LOWPOWER;
                if( isMaster == true )
                {
                    // Send the next PING frame
                    #if defined( MODE_BLE )
                        ble_header_adv.Fields.length = PINGPONGSIZE + 2;
                        memcpy( Buffer, ble_header_adv.Serial, 2 );
                        memcpy( Buffer+2, PingMsg, PINGPONGSIZE );
                    #else
                        memcpy( Buffer, PingMsg, PINGPONGSIZE );
                    #endif
                    Radio.SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                    Radio.SendPayload( Buffer, BufferSize, ( TickTime_t ){ RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE } );
                }
                else
                {
                    Radio.SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                    Radio.SetRx( ( TickTime_t ) { RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE } );
                }
                break;

            case APP_RX_ERROR:
                AppState = APP_LOWPOWER;
                // We have received a Packet with a CRC error, send reply as if packet was correct
                if( isMaster == true )
                {
                    // Send the next PING frame
                    #if defined( MODE_BLE )
                        ble_header_adv.Fields.length = PINGPONGSIZE + 2;
                        memcpy( Buffer, ble_header_adv.Serial, 2 );
                        memcpy( Buffer+2, PingMsg, PINGPONGSIZE );
                    #else
                        memcpy( Buffer, PingMsg, PINGPONGSIZE );
                    #endif
                    Radio.SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                    Radio.SendPayload( Buffer, BufferSize, ( TickTime_t ){ RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE } );
                }
                else
                {
                    // Send the next PONG frame
                    #if defined( MODE_BLE )
                        ble_header_adv.Fields.length = PINGPONGSIZE + 2;
                        memcpy( Buffer, ble_header_adv.Serial, 2 );
                        memcpy( Buffer+2, PongMsg, PINGPONGSIZE );
                    #else
                        memcpy( Buffer, PongMsg, PINGPONGSIZE );
                    #endif
                    Radio.SetDioIrqParams( TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                    Radio.SendPayload( Buffer, BufferSize, ( TickTime_t ){ RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE } );
                }
                break;

            case APP_TX_TIMEOUT:
                AppState = APP_LOWPOWER;
                Radio.SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
                Radio.SetRx( ( TickTime_t ) { RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE } ); 
                break;

            case APP_LOWPOWER:
                break;

            default:
                // Set low power
                break;
        }
		
		#endif
	}
}

void OnTxDone( void )
{
    AppState = APP_TX;
	printf( "TX done\n\r" ); 
}

void OnRxDone( void )
{
    AppState = APP_RX;
}

void OnTxTimeout( void )
{
    AppState = APP_TX_TIMEOUT;
    printf( "<>>>>>>>>TXE\n\r" ); 
}

void OnRxTimeout( void )
{
    AppState = APP_RX_TIMEOUT;
}

void OnRxError( IrqErrorCode_t errorCode )
{
    AppState = APP_RX_ERROR;
    printf( "RXE<>>>>>>>>\n\r" ); 
}

void OnRangingDone( IrqRangingCode_t val )
{
	if(val==IRQ_RANGING_MASTER_VALID_CODE)//从机请求有效
	{
		distance = SX1280GetRangingResult( RANGING_RESULT_AVERAGED );//RANGING_RESULT_RAW
		restart = 1; //Restart
		printf( "Ranging distance = %f\n\r",distance); 
		//LED_RX^=1;
	}		
	else if(val == IRQ_RANGING_MASTER_ERROR_CODE)//主机请求超时
	{
		error++;
		if(error > MAX_ERROR)//Max error reach
		{
			error = 0;
			restart = 1; //Restart
		}
		
		retry = 1; //Retry
	}
}

void OnCadDone( bool channelActivityDetected )
{
}

void sx1280_Ranging_Init(void)
{
	Radio.SetStandby( STDBY_RC );
    Radio.SetPacketType( modulationParams.PacketType );
    Radio.SetModulationParams( &modulationParams );
    Radio.SetPacketParams( &packetParams );
    Radio.SetRfFrequency( RF_FREQUENCY );
    Radio.SetBufferBaseAddresses( 0x00, 0x00 );
    Radio.SetTxParams( TX_OUTPUT_POWER, RADIO_RAMP_02_US );
	
	SX1280SetDeviceRangingAddress( 0x01020304 );//set for slave, the address is Master id
	SX1280SetRangingIdLength(RANGING_IDCHECK_LENGTH_32_BITS);//set for slave
	SX1280SetRangingRequestAddress( 0x01020304 );//set for master
	SX1280SetRangingCalibration(13376);//13308
	
	//#ifdef RANGING_MASTER//master
	
	Radio.SetDioIrqParams( Ranging_M_IrqMask, Ranging_M_IrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
	SX1280SetRangingRole(RADIO_RANGING_ROLE_MASTER);//设置主机角色
	SX1280RangingSetFilterNumSamples(16);
	// #else//slave
	
	// SetDioIrqParams(IRQ_RANGING_SLAVE_RESPONSE_DONE | IRQ_RANGING_SLAVE_REQUEST_DISCARDED);//从机中断
	// SetRangingRole(RADIO_RANGING_ROLE_SLAVE);//设置从机角色
	// #endif
}
void Ranging_Master(void)
{
	if(restart)
	{
		restart = 0;
		//HAL_Delay(50);
		sx1280_Ranging_Init();
		retry = 1;
	}
	
	if(retry)
	{
		retry = 0;

		SX1280SetTx(RX_TX_CONTINUOUS);
	}
}

