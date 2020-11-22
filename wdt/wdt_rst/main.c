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

int main(void) {
    int i, j;
    printf("test\r\n");
    GPIO_PinConfigure(LED_PIN, DISABLE, ENABLE, ENABLE, DISABLE, DISABLE);
    for (i = 0; i < 4; ++i) {
        LED_ON;
        for (j = 1000000; j > 0; j--)
            ;
        LED_OFF;
        for (j = 1000000; j > 0; j--)
            ;
    }
    WDT_Init(4, PMU_CR_LPTCLKSEL_LRC, WDT_OV_RST);
    while (1)
        ;
}
