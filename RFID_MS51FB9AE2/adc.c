#include "adc.h"
#include "MS51_16K.H"
 /**
  * @brief This configures ADC module compare mode 
  * @param[u8ADCCMPEN]  Decides the ADC Compare mode enable / disable
  * @param[u16ADCCMPVALUE] u16ADCCMPVALUE Decides the ADC comapre value. 
  * @return  None
  * @note  for MS51 Series ONLY when the compare value is same as the ADCMPH+ADCMPL the ADCF flag will be set to 1.
  * @example ADC_ComapreMode(ENABLE,0x3FF);
  */
void ADC_ComapreMode(unsigned char u8ADCCMPEN, unsigned int u16ADCCMPVALUE)
{
    SFRS=0;
    ADCMPL = u16ADCCMPVALUE&0x000F;
    ADCMPH = u16ADCCMPVALUE>>4;
    if (u8ADCCMPEN)
    {
      set_ADCCON2_ADCMPEN;
    }
    else
    {
      clr_ADCCON2_ADCMPEN;
    }
}


 /**
  * @brief This configures ADC Sampling time 
  * @param[u8ADCDIV] Decides the ADC clock devider value. Value from 0 ~ 3, devider is from 1 ~ 128, default value is 0 means Fadc = Fsys/1 
  * @param[u8ADCAQT] Decides the ADC acquisition time base to add sampling time for ADC input, value is from 0 ~ 7, time = (4*u8ADCAQT+6)/Fadc, default value is 10/Fsys = 417ns.
  * @return  None
  * @note 
  * @example ADC_ConvertTime(ADC_ADCDIV2,ADC_ADCAQT5);
  */
void ADC_ConvertTime(unsigned char u8ADCDIV, unsigned char u8ADCAQT)
{
    SFRS=0;
    ADCCON1 &= 0x8F;
    ADCCON1 |= (u8ADCDIV&0x07)<<4;
    ADCCON2&=0xF1;
    ADCCON2|=(u8ADCAQT&0x07)<<1;
}


/**
  * @brief Read the bandgap value base on Vref = 3.072V storage address after UID area.
  * @param[in] none
  * @return 12bit bandgap value
  * @example temp = READ_BANDGAP();
  */
unsigned int READ_BANDGAP()
{
    unsigned char BandgapHigh,BandgapLow;
    unsigned int u16bgvalue;
    set_CHPCON_IAPEN;
    IAPCN = READ_UID;
    IAPAL = 0x0d;
    IAPAH = 0x00;
    set_IAPTRG_IAPGO;
    BandgapLow = IAPFD&0x0F;
    IAPAL = 0x0C;
    IAPAH = 0x00;
    set_IAPTRG_IAPGO;
    BandgapHigh = IAPFD;
    u16bgvalue = (BandgapHigh<<4)+BandgapLow;
    clr_CHPCON_IAPEN;
    return (u16bgvalue);
}
unsigned int ADC_CH6_Read(void)
{
	unsigned int ADCdataAIN6;
	ENABLE_ADC_AIN6; 
	ADCCON1 |= 0x30;//clock divider 00110000 
	ADCCON2 |= 0x0E; // AQT Time 00001110
	clr_ADCCON0_ADCF; 
	set_ADCCON0_ADCS; 
	while(!(ADCCON0&SET_BIT7));
	ADCdataAIN6 = ADCRH <<4; 
	ADCdataAIN6 |= ADCRL&0x0F; 
	DISABLE_ADC;
	return ADCdataAIN6; 
}

unsigned int ADC_CH7_Read(void)
{
	unsigned int ADCdataAIN7; 
	ENABLE_ADC_AIN7; 
	ADCCON1 |= 0x30; 
	ADCCON2 |= 0xE0; 
  clr_ADCCON0_ADCF; 
	set_ADCCON0_ADCS; 
	while(!(ADCCON0 & SET_BIT7));
	ADCdataAIN7 = ADCRH <<4; 
	ADCdataAIN7 |= ADCRL&0x0F ; 
	DISABLE_ADC;
	return ADCdataAIN7;
}