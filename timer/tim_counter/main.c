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

int main(void) {
    printf("p09 input counter pulse,press any key to print cnt value\r\n");
    TIM_CounterInit(TIM1, TIM_CNT_POLARITY_HIGH, TIM1_CNT_PORT_P8_P9);
    _DG;
    u32 cnt = TIM1->CTVAL;
    printf("cnt:%d\r\n", cnt);
    while (1)
        ;
}
