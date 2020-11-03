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
#include "globle.h"
#include "stropt.h"


int param[20];
char cmd_buff[STR_BUFF_LEN];
char param_buff[PARM_NUM][STR_BUFF_LEN];
void (*ptest_func)(void);

void Timer0DelayNus(int del)
{
    SYSC->CLKENCFG |= SYSC_CLKENCFG_TIM1_CNT | SYSC_CLKENCFG_TIM_PCK;
    TIMERS->CON = TIM_CON_TM_TIM1;
    TIM1->CTCG1 = del<<3;
    TIMERS->INTCLR = (1<<0);
    TIMERS->CON = TIM_CON_TM_TIM1 | TIM_CON_TE_TIM1;
    while((TIMERS->INTFLAG&0x01)==0);
    TIMERS->CON = 0;
}

void printSwanRegValue(void)
{
    _DBG("CR:");_DBH32(TWC->CR);    _DBG_(" ");
    _DBG("SWCR:");_DBH32(TWC->SWCR);_DBG_(" ");
    _DBG("SWBR:");_DBH32(TWC->SWBR);_DBG_(" ");
    _DBG("GAPW:");_DBH32(TWC->GAPW);_DBG_(" ");
    _DBG("CMD1:");_DBH32(TWC->CMD1);_DBG_(" ");
    _DBG("CMD2:");_DBH32(TWC->CMD2);_DBG_(" ");
    _DBG("CMD3:");_DBH32(TWC->CMD3);_DBG_(" ");
    _DBG("CMD4:");_DBH32(TWC->CMD4);_DBG_(" ");
    _DBG("RXD:");_DBH32(TWC->RXD);_DBG_(" ");
    _DBG("TXD:");_DBH32(TWC->TXD);_DBG_(" ");
    _DBG("STS:");_DBH32(TWC->STS);_DBG_(" ");
    _DBG("AF0:");_DBH32(IOM->AF0);_DBG_(" ");
    _DBG("AF1:");_DBH32(IOM->AF1);_DBG_(" ");
    _DBG("CLEENCFG:");_DBH32(SYSC->CLKENCFG);_DBG_(" ");
}

// #define SEBUS_SEND_CUR_PULSE(x)   {TWC_SendEnable();TWC_WriteData(0);Timer0DelayNus(x);TWC_WriteData(1);}
void sebus_send_data0(int level,int del)
{
    TWC_SendEnable();
    TWC_WriteData(level);
    Timer0DelayNus(del);
    TWC_WriteData(!level);
}
static u32 even_check(u32 wData)
{
    int i,iCnt=0;
    for(i=0;i<32;++i)
    {
        if( wData&0x01)
        {
            ++iCnt;
        }
        wData >>= 1;
    }
    if( iCnt&0x01 )
    {
        return 1;
    }
	else
    {
        return 0;
    }
}
void sebus_rcv_send_data(void)
{
    int i;
    if( param[0] == 0 )
        TWC_Init(TWC_PIN_18_19);
    else
        TWC_Init(TWC_PIN_10_11);
    int mode,txLelCfg,rxDecCfg,rxGltchFiltCfg;
    if( param[1] == 1)
    {
        mode = TWC_MODE_EPC;
        TWC_SetGapAndGapComp(150,60);
    }
    else
    {
        mode = TWC_MODE_9000;
        TWC_SetGapAndGapComp(120,60);
    }
    if( param[2] == 1)
    {
        txLelCfg = TWC_TX_LEVEL_HIGH_EN;
    }
    else
    {
        txLelCfg = TWC_TX_LEVEL_LOW_EN;
    }
    if( param[3] == 1)
    {
        rxDecCfg = TWC_RX_DEC_NO_MT_CMD_INT;
    }
    else
    {
        rxDecCfg = TWC_RX_DEC_MATCH_CMD_INT;
    }
    if((param[4]>0)&&(param[4]<16))
    {
        rxGltchFiltCfg = TWC_RX_FILT_2N_CYCLE(param[4]);
    }
    else
    {
        rxGltchFiltCfg = TWC_RX_FILT_NO;
    }
    TWC_SEBUSConfig(mode,txLelCfg,rxDecCfg,rxGltchFiltCfg);

    int cmdNo = param[5];
    u16 cmd = ((u16)param[6]<<9) | param[7];
    u16 cmd_msk = ( (u16)param[8]<<9) | param[9];
    TWC_SetCMDAndMask(cmdNo,cmd,cmd_msk);
    _DBG("cmdaddr:");_DBH16(cmd);_DBG("\r\n");

    printSwanRegValue();
    TWC_ClrIRQFlag(TWC_RX_FRAME_END);
    while((TWC->STS & TWC_STS_RXFRMEEND)==0);  //test no hander dead loop ,press reset key
    TWC->CR &= ~TWC_CR_RXRECEN;
    if( param[1] == 1)
    {
        for(i=0;i<16;++i)
        {
            while( (TWC->STS&TWC_STS_RXDATLEV) != 0); //test no hander
            if( cmd & 0x8000)
            {
                sebus_send_data0(txLelCfg,100);//SEBUS_SEND_CUR_PULSE(100);
            }
            cmd <<= 1;
            while( (TWC->STS&TWC_STS_RXDATLEV) == 0);
        }
        while( (TWC->STS&TWC_STS_RXDATLEV) != 0); //test no hander
        sebus_send_data0(txLelCfg,100);//SEBUS_SEND_CUR_PULSE(100);
        while( (TWC->STS&TWC_STS_RXDATLEV) == 0);
        while( (TWC->STS&TWC_STS_RXDATLEV) != 0); //test no hander
        if( even_check(cmd)==1)
            sebus_send_data0(txLelCfg,100);//SEBUS_SEND_CUR_PULSE(100);
        while( (TWC->STS&TWC_STS_RXDATLEV) == 0);
    }
    else
    {
        for(i=0;i<16;++i)
        {
            while( (TWC->STS&TWC_STS_RXDATLEV) != 0); //test no hander
            while( (TWC->STS&TWC_STS_RXDATLEV) == 0);
            if( cmd & 0x8000)
            {
                sebus_send_data0(txLelCfg,300);//SEBUS_SEND_CUR_PULSE(300);
            }
            cmd <<= 1;
        }
        while( (TWC->STS&TWC_STS_RXDATLEV) != 0); //test no hander
        while( (TWC->STS&TWC_STS_RXDATLEV) == 0);
        sebus_send_data0(txLelCfg,300);//SEBUS_SEND_CUR_PULSE(300);
        while( (TWC->STS&TWC_STS_RXDATLEV) != 0); //test no hander
        while( (TWC->STS&TWC_STS_RXDATLEV) == 0);
        if( even_check(cmd)==1)
            sebus_send_data0(txLelCfg,300);//SEBUS_SEND_CUR_PULSE(300);
    }
    _DBG_("rcv:");_DBH32(TWC_ReadData());
}
void swanbus_rcv_send_data(void)
{
	int i;
    TWC_Init(TWC_PIN_18_19);
    int txbaud,rxbaud;
    switch(param[0])
    {
        case 0:txbaud  = TWC_SWBR_TXBR_1K;break;
        case 1:txbaud  = TWC_SWBR_TXBR_2K;break;
        case 2:txbaud  = TWC_SWBR_TXBR_3K;break;
        case 3:txbaud  = TWC_SWBR_TXBR_4K;break;
        case 4:txbaud  = TWC_SWBR_TXBR_5K;break;
        case 5:txbaud  = TWC_SWBR_TXBR_6K;break;
        case 6:txbaud  = TWC_SWBR_TXBR_8K;break;
        case 7:txbaud  = TWC_SWBR_TXBR_10K;break;
        case 8:txbaud  = TWC_SWBR_TXBR_12K;break;
        case 9:txbaud  = TWC_SWBR_TXBR_15K;break;
        case 10:txbaud = TWC_SWBR_TXBR_20K;break;
        case 12:txbaud = TWC_SWBR_TXBR_25K;break;
        case 13:txbaud = TWC_SWBR_TXBR_30K;break;
        case 14:txbaud = TWC_SWBR_TXBR_40K;break;
        case 15:txbaud = TWC_SWBR_TXBR_50K;break;
        case 16:txbaud = TWC_SWBR_TXBR_60K;break;
    }
    switch(param[1])
    {
        case 0:rxbaud  = TWC_SWBR_RXBR_1K;break;
        case 1:rxbaud  = TWC_SWBR_RXBR_2K;break;
        case 2:rxbaud  = TWC_SWBR_RXBR_3K;break;
        case 3:rxbaud  = TWC_SWBR_RXBR_4K;break;
        case 4:rxbaud  = TWC_SWBR_RXBR_5K;break;
        case 5:rxbaud  = TWC_SWBR_RXBR_6K;break;
        case 6:rxbaud  = TWC_SWBR_RXBR_8K;break;
        case 7:rxbaud  = TWC_SWBR_RXBR_10K;break;
        case 8:rxbaud  = TWC_SWBR_RXBR_12K;break;
        case 9:rxbaud  = TWC_SWBR_RXBR_15K;break;
        case 10:rxbaud = TWC_SWBR_RXBR_20K;break;
        case 12:rxbaud = TWC_SWBR_RXBR_25K;break;
        case 13:rxbaud = TWC_SWBR_RXBR_30K;break;
        case 14:rxbaud = TWC_SWBR_RXBR_40K;break;
        case 15:rxbaud = TWC_SWBR_RXBR_50K;break;
        case 16:rxbaud = TWC_SWBR_RXBR_60K;break;
    }
    sSwanBusCfgParam parm;
    if( param[2] == 1)
        parm.txLelCfg = TWC_TX_LEVEL_HIGH_EN;
    else
        parm.txLelCfg = TWC_TX_LEVEL_LOW_EN;
    if( param[3] == 1)
        parm.rxDecCfg = TWC_RX_DEC_NO_MT_CMD_INT;
    else
        parm.rxDecCfg = TWC_RX_DEC_MATCH_CMD_INT;
    if( param[4] > 0)
        parm.rxGlitchFiltCfg = TWC_RX_FILT_2N_CYCLE(param[4]);
    else
        parm.rxGlitchFiltCfg = TWC_RX_FILT_NO;
    if( param[5] == 1)
        parm.rxParityCfg = TWC_RX_PARITY_HMM;
    else
        parm.rxParityCfg = TWC_RX_PARITY_EVEN;
    if( param[6] == 1)
        parm.txCodeCfg = TWC_TX_CODE_MCT;
    else
        parm.txCodeCfg = TWC_TX_CODE_NRZ;
    if( param[7] == 1)
        parm.txParityCfg = TWC_TX_PARITY_ODD;
    else
        parm.txParityCfg = TWC_TX_PARITY_EVEN;
    parm.txBitCfg = param[8]; //[0-3]==>8bit 16 24 32
    TWC_SWANBusConfig(txbaud,rxbaud,&parm);
    TWC_SetGapAndGapComp(param[9],param[10]);
    int cmdNo = param[11];
    u16 cmd = ((u16)param[12]<<8) | param[13];
    u16 cmd_msk = ( (u16)param[14]<<8) | param[15];
    TWC_SetCMDAndMask(cmdNo,cmd,cmd_msk);
    _DBG("cmdaddr:");_DBH16(cmd);_DBG("\r\n");
    u32 txdata = ((u32)cmd<<16) | ( (u16)param[16]<<8) | param[17];
    if(parm.txLelCfg)
        TWC_WriteData(txdata);
    else
        TWC_WriteData(~txdata);
    TWC_SendEnable();
    printSwanRegValue();
    while((TWC->STS & TWC_STS_RXFRMEEND)==0);  //test no hander dead loop ,press reset key
    TWC_ClrIRQFlag(TWC_RX_FRAME_END);
    _DBG_("rcv:");_DBH32(TWC_ReadData());
}

int main(void)
{
    int i;
	debug_frmwrk_init();
	_DBG_("twc test");
	while(1)
    {
        rcv_strcmd(cmd_buff);
        if( mystrcmp(cmd_buff,"twc_bus") )  //sebus
        {
            rcv_strcmd(cmd_buff);
            if( mystrcmp(cmd_buff,"sebus_rcv_send_data"))
            {
                ptest_func = sebus_rcv_send_data;
                int len = 5+5;
                rcv_param(param_buff,len);
                for(i=0;i<len;++i)
                {
                    strtohex(param_buff[i],&param[i]);
                }
            }
            else if(mystrcmp(cmd_buff,"swanbus_rcv_send_data"))
            {
                ptest_func = swanbus_rcv_send_data;
                int len = 18;
                rcv_param(param_buff,len);
                for(i=0;i<len;++i)
                {
                    strtohex(param_buff[i],&param[i]);
                }
            }
            else
            {
                ptest_func = 0;
            }
            if( ptest_func != 0)
                ptest_func();
        }
    }
	return 0;
}


