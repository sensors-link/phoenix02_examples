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

#define TEST_PIN GPIO_PIN8

#define PIN_ON GPIO_SetPin(TEST_PIN)
#define PIN_OFF GPIO_ClrPin(TEST_PIN)

int main(void) {
    int i;
    GPIO_PinConfigure(TEST_PIN, DISABLE, ENABLE, ENABLE, DISABLE, DISABLE);
    while (1) {
        PIN_OFF;
        for (i = 10000; i > 0; i--)
            ;
        PIN_ON;
        for (i = 10000; i > 0; i--)
            ;
    }
}
