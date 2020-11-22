/**
 * @file main.c
 * @author bifei.tang (you@domain.com)
 * @brief
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

void NMI_Handler(void) {
    static int tog = 0;
    if (tog) {
        LED_ON;
        tog = 0;
    } else {
        LED_OFF;
        tog = 1;
    }
    WDT_ClrIntFlag();
}
int main(void) {

    GPIO_PinConfigure(LED_PIN, DISABLE, ENABLE, ENABLE, DISABLE, DISABLE);
    WDT_Init(4, PMU_CR_LPTCLKSEL_LRC, WDT_OV_INT);
    while (1)
        ;
    return 0;
}
