/*
 ============================================================================
 Name        : main.c
 Author      : tbf
 Version     :
 Copyright   : Your copyright notice
 Description : Hello RISC-V World in C
 ============================================================================
 */


#include "lib_include.h"

u8 SPI_Receive(void)
{
    while((SPI_GetStatus()&SPI_SR_DONESR)==0);
    SPI_ClrIntFlag();
    return SPI_RecieveData();
}

void SPI_Send(u8 dat)
{
    SPI_SendData(dat);
    while((SPI_GetStatus()&SPI_SR_DONESR)==0);
}

//u8 SPI_Receive_TO(u8 *pdat)
//{
//    int i = 8000000;
//    while(((SPI_GetStatus()&SPI_SR_DONESR)==0) && (--i>0));
//    SPI_ClrIntFlag();
//    if( i>0)
//    {
//        *pdat = SPI_RecieveData();
//        return 1;
//    }
//    else
//    {
//        return 0;
//    }
//
//}

u8 CheckSum(u8 *pbyData,u32 len)
{
	u8 bySum = 0;
	while(len--)
	{
		bySum += *pbyData++;
	}
	return bySum+0xcc;
}

void CmdHander(u8 *pbyData)
{
	u8 byParm0,byParm1,byParm2,byParm3,byAA=0;
	u32 wAddr,wData;
	u8 * pTmp = pbyData;
	volatile u32 *pwReg;
	byParm0 = *pTmp++;
	byParm1 = *pTmp++;
	byParm2 = *pTmp++;
	byParm3 = *pTmp++;
	wAddr = *(u32*)pTmp;
	pTmp += 4;
	wData = *(u32*)pTmp;
	if( byParm0 == 0)
	{
		pwReg = (volatile u32*)wAddr;
		switch(byParm1)
		{
			case 0:
				*pwReg &= ~(((1<<byParm3)-1)<<byParm2);
				*pwReg |= wData<<byParm2;
			break;
			case 1:
				*pwReg |= wData;
			break;
			case 2:
				*pwReg &= ~wData;
			break;
			case 3:
				*pwReg = wData;
			break;
			default:byAA = 1;
		}
	}
	else if( byParm0 == 1)
	{
		pwReg = (volatile u32*)wAddr;
		switch(byParm1)
		{
			case 0:
				wData = (*pwReg & (((1<<byParm3)-1)<<byParm2) )>>byParm2;
			break;
			case 1:
				wData = *pwReg & wData;
			break;
			case 2:
				wData = *pwReg & (~wData);
			break;
			case 3:
				wData = *pwReg;
			break;
			default:byAA = 1;
		}
	}else if(byParm0 == 2)
	{
		switch(byParm1)
		{
			case 0:
                PMU_SoftChipReset();
			break;
			case 1:
				PMU_SoftDigitalReset();
			break;
			case 2:
                SYSC_ResetPeripher(1<<byParm2);
			break;
			default:byAA=1;
		}
	}
	pbyData[8] = wData&0xff;
	pbyData[9] = (wData>>8)&0xff;
	pbyData[10] = (wData>>16)&0xff;
	pbyData[11] = (wData>>24)&0xff;
	SPI_Send(0xcc);
	pbyData[0] = byAA;
	int i;
	for(i=0;i<12;++i)
	{
		SPI_Send(pbyData[i]);
	}
	u8 bySum=CheckSum(pbyData,12);
	if(byAA == 1)
		bySum += 1;
	SPI_Send(bySum);
}




int main(void)
{
	debug_frmwrk_init();
	_DBG_("register read write");
    SPI_Init(SPI_PIN_19_18_16_17,SPI_SLAVE,SPI_CPOL_LOW,SPI_CPHA_FIST,1000);
    _DBH32(SPI->CR0);
    SPI_ClrIntFlag();
    u8 byCmd,byData[13];
    int i;
    while(1){
ReStart:
    byCmd = SPI_Receive();
    if( byCmd == 0xcc)
    {
        for(i=0;i<13;++i)
        {
//            if( !SPI_Receive_TO(&(byData[i])) )
//                goto ReStart;
        	byData[i] = SPI_Receive();

        }
        if( CheckSum(byData,12) == byData[12])
        {
            CmdHander(byData);
        }
    }

    }


}



