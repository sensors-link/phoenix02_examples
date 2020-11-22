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

void IOM_IrqHandler(void) {
    if (GPIO_GetIntFlag() == TEST_PIN)
        printf("iom int\r\n");
    GPIO_ClrIntFlag(TEST_PIN);
};

int main(void) {
    int i;
    GPIO_PinConfigure(TEST_PIN, DISABLE, DISABLE, ENABLE, DISABLE,
                      DISABLE); // gpio pd in
    GPIO_PinIntConfig(TEST_PIN, PIN_INT_TYPE_EDGE, PIN_INT_POL_LOW);
    PLIC_EnableIRQ(IOM_IRQn);
    PLIC_SetPriority(IOM_IRQn, 1);
    GPIO_GlobleIRQControl(DISABLE, DISABLE, ENABLE);
    GPIO_ClrIntFlag(TEST_PIN);
    GPIO_PinIRQControl(TEST_PIN, ENABLE);
    printf("poll test pin gp08 generate int\r\n");

    while (1) {
    }
}
