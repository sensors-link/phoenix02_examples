/**
 * @file main.c
 * @author DavidLin
 * @brief
 * @version 0.1
 * @date 2021-08-26
 *
 * @copyright Fanhai Data Tech. (c) 2021
 *
 */

#include "lib_include.h"

#define LED_PIN  GPIO_PIN8

#define LED_ON   GPIO_SetPin(LED_PIN)
#define LED_OFF  GPIO_ClrPin(LED_PIN)

#define TIME_DIFF(s,e) ((s)-(int)(e))

int timer_init(void)
{
	// 初始化Timer1
    SYSC->CLKENCFG |= SYSC_CLKENCFG_TIM_PCK | SYSC_CLKENCFG_TIM1_CNT;
	SYSC->TIMCLKDIV = SystemCoreClock / 1000000 - 1; // 1M
	TIMERS->INTCLR |= BIT(0); // 清TIMER1中断标记
    // 32位自由运行模式
	TIM1->CTCG1 = 0;
	TIMERS->CON |= BIT(0);    // 使能TIMER1

	return 0;
}

void delay_us(u32 delay)
{
    unsigned int start = TIM1->CTVAL;
    while(TIME_DIFF(start, TIM1->CTVAL) < delay) __asm("NOP");
}

void gpio_out_example(void)
{
    int i;

    timer_init();

    GPIO_PinConfigure(LED_PIN, DISABLE, ENABLE, ENABLE, DISABLE, DISABLE);

    while (1)
    {
    	LED_OFF;
    	delay_us(100000);
        LED_ON;
        delay_us(100000);
    }
}
