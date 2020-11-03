/**
 * @file main.c
 * @author bifei.tang (you@domain.com)
 * @brief  timer
 * @version 0.1
 * @date 2020-05-28
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */


#include "lib_include.h"






int main(void)
{
    debug_frmwrk_init();
	_DBG_("p09 input counter pulse,press any key to print cnt value");
    TIM_CounterInit(TIM1,TIM_CNT_POLARITY_HIGH,TIM1_CNT_PORT_P8_P9);
    _DG;
    u32 cnt = TIM1->CTVAL;
    _DBD32(cnt);
    while(1);
}
