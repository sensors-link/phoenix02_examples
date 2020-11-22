/**
 * @file main.c
 * @author bifei.tang (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2020-06-01
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */

#include "lib_include.h"

#define AT24C02_ADDR 0xa0

const u8 gchWrDat[10] = {0x01, 0x02, 0x03, 0x04, 0x05,
                         0x06, 0x07, 0x08, 0x09, 0x0a};
u8 gchRdDat[10] = {0};

void DelayNus(int del) {
    int i;
    while (del--) {
        i = 1000000;
        while (i--)
            ;
    }
}

u8 WriteDataToAt24c02(u8 chAddr) {
    int i;
    u32 wTmp;
    I2C_WriteStatusReg(0xffff);
    I2C_SendData(I2C_CMD_WRITE, chAddr);
    while ((I2C_ReadStatusReg() & I2C_ISR_R_TX_EMPTY) == 0)
        ;
    if (I2C_ReadStatusReg() & I2C_ISR_R_TX_ABRT)
        return 0;
    for (i = 0; i < 9; ++i) {
        I2C_WriteStatusReg(0xffff);
        I2C_SendData(I2C_CMD_WRITE, chAddr);
        while ((I2C_ReadStatusReg() & I2C_ISR_R_TX_EMPTY) == 0)
            ;
    }
    I2C_WriteStatusReg(0xffff);
    I2C_SendData(I2C_CMD_WRITE | I2C_CMD_STOP, chAddr);
    while ((I2C_ReadStatusReg() & I2C_ISR_R_TX_EMPTY) == 0)
        ;
    return 1;
}

u8 ReadDataFromAt24c02(u8 chAddr) {
    int i;
    u32 wTmp;
    I2C_WriteStatusReg(0xffff);
    I2C_SendData(I2C_CMD_WRITE, chAddr);
    while ((I2C_ReadStatusReg() & I2C_ISR_R_TX_EMPTY) == 0)
        ;
    if (I2C_ReadStatusReg() & I2C_ISR_R_TX_ABRT)
        return 0;
    I2C_WriteStatusReg(0xffff);
    I2C_SendData(I2C_CMD_READ | I2C_CMD_RESTART, 0);
    while ((I2C_ReadStatusReg() & I2C_ISR_R_RX_FULL) == 0)
        ;
    gchRdDat[0] = I2C->DATACMD;
    for (i = 1; i < 9; ++i) {
        I2C_WriteStatusReg(0xffff);
        I2C_SendData(I2C_CMD_READ, 0);
        while ((I2C_ReadStatusReg() & I2C_ISR_R_RX_FULL) == 0)
            ;
        gchRdDat[i] = I2C->DATACMD;
    }
    I2C_WriteStatusReg(0xffff);
    I2C_SendData(I2C_CMD_READ | I2C_CMD_STOP, 0);
    while ((I2C_ReadStatusReg() & I2C_ISR_R_RX_FULL) == 0)
        ;
    gchRdDat[i] = I2C->DATACMD;
    return 1;
}

int main(void) {
    int i;
    debug_frmwrk_init();
    printf("i2c write at24c02 example\r\n");
    I2C_Init(I2C_PIN_12_13, ENABLE, 100, 0x55, AT24C02_ADDR >> 1);
    if (WriteDataToAt24c02(0x00)) {
        DelayNus(10000);
        if (ReadDataFromAt24c02(0x00)) {
            for (i = 0; i < 10; ++i) {
                if (gchRdDat[i] != gchWrDat[i]) {
                    printf("FAIL\r\n");
                    break;
                }
            }
            if (i == 10)
                printf("PASS\r\n");
        } else {
            printf("FAIL\r\n");
        }
    } else {
        printf("Write Fail\r\n");
    }
    // main loop
    while (1) {
    }

    return 0;
}
