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

#include "w25q16.h"

const u8 gWrText[]={"Ph02 SPIM TEST"};
u8 gRdText[sizeof(gWrText)];



void DelayNus(u32  wDel)
{
	u32 wCnt;
    SYSC->CLKENCFG |= SYSC_CLKENCFG_TIM_PCK | SYSC_CLKENCFG_TIM1_CNT;
	TIMERS->CON &= ~(1<<0);
	wCnt = wDel * (8000000/1000000);
	TIM1->CTCG1 = wCnt;
	TIMERS->CON |= 0x01;
	while( (TIMERS->INTFLAG & (1<<0) ) == 0 );
	TIMERS->INTCLR = (1<<0);
}


int main(void)
{
    int i;
    debug_frmwrk_init();
	_DBG_("i2c write w25q16 example");
	SPI_Flash_Init();
	while(SPI_Flash_ReadID()!=W25Q16 && SPI_Flash_ReadID()!=W25Q32 && SPI_Flash_ReadID()!=W25Q64)
	{
        _DBG_("read id error");
        while(1);
    }

    SPI_Flash_Write((u8*)gWrText,0,sizeof(gWrText));
    _DBG_("write text:Ph01 SPIM TEST");
    SPI_Flash_Read(gRdText,0,sizeof(gWrText));
	_DBG_("read text:");
    for(i=0;i<sizeof(gWrText)-1;++i)
    {
        _DBH(gRdText[i]);
    }
    _DBG_("");
    _DBG_("end");
    //main loop
	while(1)
	{
	}

}


