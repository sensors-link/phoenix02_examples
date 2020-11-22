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

#define LED_PIN GPIO_PIN8

#define LED_ON GPIO_SetPin(LED_PIN)
#define LED_OFF GPIO_ClrPin(LED_PIN)

int main(void) {
    TIM_PWMInit(TIM1, TIM_PWM_POL_PWM0_PWM1, 1, 50, TIM1_PWM_PORT_P8_P9, 0);
    while (1)
        ;
}
