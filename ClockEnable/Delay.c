#include "Delay.h"


void delay_us(unsigned us)
{
	  //SysTick->VAL =0;
    //SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    while (us--)
    {
      //  while (!(Systick->V & SysTick_CTRL_COUNTFLAG_Msk));
    }

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}
void delay_ms(unsigned ms)
{
    while (ms--)
    {
        delay_us(1000);
    }
}

void init_systick(void)
{
    SysTick->LOAD = 72 - 1;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk;
}
