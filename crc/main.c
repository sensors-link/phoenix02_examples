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

#define LED_PIN   GPIO_PIN8

#define LED_ON     GPIO_SetPin(LED_PIN)
#define LED_OFF    GPIO_ClrPin(LED_PIN)

u8 u8data[4] = {0x78,0x56,0x34,0x12};
u16 u16data[2] = {0x5678,0x1234};
u32 u32data = 0x12345678;
u16 crc = 0xf428;

int main(void)
{
	debug_frmwrk_init();
	_DBG_("test");
    GPIO_PinConfigure(LED_PIN,DISABLE,ENABLE,ENABLE,DISABLE,DISABLE);
    CRC_Init();
    if( (CRC_Calculate(u8data,DATA_8BIT,4)!=crc))
    {
        LED_OFF;
        _DBG_("8fail");
    }
    else
    {
        LED_ON;
        _DBG_("8success");
    }
    if( (CRC_Calculate(u16data,DATA_16BIT,2)!=crc))
    {
        LED_OFF;
        _DBG_("16fail");
    }
    else
    {
        LED_ON;
        _DBG_("16success");
    }
    if(CRC_Calculate(&u32data,DATA_32BIT,1)!=crc)
    {
        LED_OFF;
        _DBG_("32fail");
    }
    else
    {
        LED_ON;
        _DBG_("32success");
    }
    while(1);
}
