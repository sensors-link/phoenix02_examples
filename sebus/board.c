/**
 * @file board.c
 * @author bifei.tang
 * @brief pcb相关函数
 * @version 0.1
 * @date 2020-07-06
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */
#include "lib_include.h"
#include <string.h>
#include <globle.h>
#include "period_event_handler.h"
/**
 * @brief 全局变量初始化
 * @note 需要禁止启动代码里的零初始化部分
 */
static void GlobleVariableInit(void)
{
    if( PMU->RSTSTS & (PMU_RSTSTS_PORRSTSTS | PMU_RSTSTS_WDTRSTSTS))  //上电启动要清变量
    {
    	if(PMU->RSTSTS & PMU_RSTSTS_PORRSTSTS )
    	{
			printf("\r\npower rst\r\n");
			PMU->RSTSTS = PMU_RSTSTS_PORRSTSTS;
    	}
    	else
    	{
    		printf("\r\wdt rst\r\n");
        	PMU->RSTSTS = PMU_RSTSTS_WDTRSTSTS;
    	}
        gStatus |= STATUS_POWER_UP;
        memset(&gEvent100MS,0,sizeof(Event100MS_t));
        g100MSCount = 1;
        gStatus = 0;
        gDayCount = 0;
        bProtocol = -1;
        gRamCheck = 0xa5a55a5a;
        gRecordFlag |= RECORD_FLAG_WR_FW_VERSION;
    }

}

/**
 * @brief IO端口初始化
 *
 */
static void GPIOPortInit(void)
{
#ifdef _DEBUG
	#define TEST_PIN   GPIO_PIN17
	#define PIN_ON     GPIO_SetPin(TEST_PIN)
	#define PIN_OFF    GPIO_ClrPin(TEST_PIN)
    GPIO_PinConfigure(TEST_PIN,DISABLE,ENABLE,ENABLE,DISABLE,DISABLE);
    PIN_OFF;
    DelayNus(100);
    PIN_ON;
    DelayNus(100);
    PIN_OFF;
#endif
}

/**
 * @brief 系统外设硬件设备初始化
 *
 */
static void PeripheralInit(void)
{
#ifdef _DEBUG
	extern unsigned long gRcvEnalbe;
    UART_DeInit(UART1);
    UART_Init(UART1,UART1_PORT_P00_P01,UART_MODE_10B_ASYNC,9600);
    UART_EnableIRQ(UART1,UART_IRQ_TYPE_RX);  //使能中断接收
    PLIC_SetPriority(UART1_IRQn,2);
    PLIC_EnableIRQ(UART1_IRQn);gRcvEnalbe = 0;

    UART_DeInit(UART2);
    UART_Init(UART2,UART2_PORT_P02_P03,UART_MODE_10B_ASYNC,9600);
#endif

    TWC_Init(TWC_PIN_18_19);
    PLIC_SetPriority(TWC_IRQn,7);
    PLIC_EnableIRQ(TWC_IRQn);
    TWC_EnableIRQ(TWC_RX_FRAME_END);
    TWC_SetCMDAndMask(TWC_CMD_1, 0x8000, 0b0111111111111111);
    TWC_SetCMDAndMask(TWC_CMD_2, 0x2f<<9, 0b0000000111111111);
    TWC->CR |= TWC_CR_TXLELCFG;
//    if( bProtocol != -1)
//    {
//        TWC->CR |= TWC_CR_RXRECEN;
//        if( bProtocol == 1)
//        {
//            TWC_SetGapAndGapComp(150,60);
//        }
//        else
//        {
//            TWC_SetGapAndGapComp(120,60);
//        }
//        TWC_SEBUSConfig(TWC_MODE_EPC,TWC_TX_LEVEL_HIGH_EN,TWC_RX_DEC_MATCH_CMD_INT,TWC_RX_FILT_2N_CYCLE(2));
//    }
    PLIC_SetPriority(LPT_IRQn,1);
    PLIC_EnableIRQ(LPT_IRQn);
    LPT_Init(PMU_CR_LPTCLKSEL_LRC,100,LPT_PIT_CNT);
    LPT_ClrIntFlag();
    LPT_EnableIRQ();
    //timer2 init
	SYSC->CLKENCFG |= SYSC_CLKENCFG_TIM_PCK | SYSC_CLKENCFG_TIM1_CNT | SYSC_CLKENCFG_TIM2_CNT;
	SYSC->TIMCLKDIV = FSYS/1000000 - 1;    //1M
	TIMERS->INTCLR = (TIM_INTCLR_TIM2);
	TIMERS->CON |= (TIM_CON_TE_TIM2 | TIM_CON_TM_TIM2);

}

/**
 * @brief 板子资源初始化
 *
 */
void BoardInit(void)
{
    int i;
	DisableGlobleIRQ();
    PeripheralInit();
    GlobleVariableInit();
    GPIOPortInit();
    for(i=0;i<2;++i)            //复位闪灯两下
    {
        ANAC_WPT_UNLOCK();
        ANAC->LED_CFG |= ANAC_LED_CFG_INDLED_EN;
        DelayNus(30000);
        ANAC_WPT_UNLOCK();
        ANAC->LED_CFG &= ~ANAC_LED_CFG_INDLED_EN;
    }
    EnableGlobleIRQ();
}



