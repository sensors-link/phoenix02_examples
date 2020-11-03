/**
 * @file exception_handler.c
 * @author bifei.tang (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2020-04-23
 *
 * @copyright Fanhai Data Tech. (c) 2020
 * @note: user add code
 */

#include "lib_include.h"

void PMU_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("pmu int");
#endif

};

void LPT_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("lpt int");
#endif
    LPT_ClrIntFlag();
};

void TIMER1_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("timer1 int");
#endif


    TIM_ClrIntFlag(TIM1);
};

void TIMER2_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("timer2 int");
#endif


    TIM_ClrIntFlag(TIM2);
};

void TIMER3_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("timer3 int");
#endif

    TIM_ClrIntFlag(TIM3);
};

void TIMER4_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("timer4 int");
#endif

    TIM_ClrIntFlag(TIM4);
};

void UART1_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("uart1 int");
#endif

};

void UART2_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("uart2 int");
#endif

};

void SPI_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("spi int");
#endif

};

void ANAC_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("anac int");
#endif

};

void EFC_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("efc int");
#endif

};

void IOM_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("iom int");
#endif

};

void I2C_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("i2c int");
#endif

};

void RTC_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("rtc int");
#endif

};

void TWC_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("twc int");
#endif

};

void LPU_IrqHandler(void){
#ifdef _FW_DEBUG
	_DBG_("lpu int");
#endif

};


void MSOFT_IntHandler(void)  {

#ifdef _FW_DEBUG
	_DBG_("soft int");
#endif
    SoftClrIRQ();
}

// void MTIM_IntHandler(void) {
// #ifdef _FW_DEBUG
// 	_DBG_("tim int");
// #endif
// }

void MEXP_Handler(void) {
#ifdef _FW_DEBUG
	_DBG_("exp handler");
#endif
}

void NMI_Handler(void) {
#ifdef _FW_DEBUG
	_DBG_("wdt int");
#endif
    WDT_ClrIntFlag();
}

