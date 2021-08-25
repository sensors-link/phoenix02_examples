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

#define LED_PIN   GPIO_PIN8

#define LED_ON    GPIO_SetPin(LED_PIN)
#define LED_OFF   GPIO_ClrPin(LED_PIN)

/**初始化ms定时器,中断周期50ms*/
static void Timer_ms_Init(void)
{
    TIM_TimerInit(TIM1, TIM_TM_AUTO_LOAD, 50000);
    PLIC_EnableIRQ(TIMER1_IRQn);
    PLIC_SetPriority(TIMER1_IRQn, 1);
    TIM_ClrIntFlag(TIM1);
    TIM_EnableIRQ(TIM1);
}

void TIMER1_IrqHandler(void) 
{
    static int tog = 0;
    if (tog) {
        LED_ON;
        tog = 0;
    } else {
        LED_OFF;
        tog = 1;
    }
    TIM_ClrIntFlag(TIM1);
}

int main(void) 
{
    GPIO_PinConfigure(LED_PIN, DISABLE, ENABLE, ENABLE, DISABLE, DISABLE);
    
    SYSC->TIMCLKDIV = (SystemCoreClock / 1000000 - 1);
    SYSC->CLKENCFG |= SYSC_CLKENCFG_TIM_PCK;

    SYSC->CLKENCFG |= SYSC_CLKENCFG_TIM1_CNT;
    TIMERS->CON |= BIT(0); // 使能TIMER1

    Timer_ms_Init();
    
    while (1)
        ;
}
