/**
 * @file globle.c
 * @author bifei.tang
 * @brief 全局变量定义
 * @version 0.1
 * @date 2020-07-06
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */

#include "globle.h"
#include "phnx02.h"
#include "lib_include.h"
#include "smk.h"

//全局计数以100ms为基准
unsigned int g100MSCount;            //低功耗定时器中断计数值(100ms中断一次)
long gStatus;                //运行状态标志
long gRecordFlag;            //记录事件标志

long gGroupNumber;           //组号
long gAddrInGroup;           //组内地址

long gAlarmVerifyCount;    //火警确认计数
long bProtocol;

unsigned char gDayCount;     //天计数

unsigned long gRamCheck;
SMK gSmk;

/**
 * @brief 微秒延时函数
 *
 * @param del 单位微秒
 */
void DelayNus(u32 del)
{
	TIMERS->CON = 0;
	TIM1->CTCG1 = del;
	TIMERS->INTCLR = TIM_INTCLR_TIM1;
	TIMERS->CON = (TIM_CON_TE_TIM1 | TIM_CON_TM_TIM1);
	while( (TIMERS->INTFLAG & TIM_INTFLAG_TIM1) == 0);
	TIMERS->CON = 0;
}


/**
 * @brief 软件复位
 *
 */
void SoftReset(void)
{
    DisableGlobleIRQ();
    PMU_SoftCoreReset();//PMU_SoftChipReset();
    while(1){};
}


