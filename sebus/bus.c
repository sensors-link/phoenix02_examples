/**
 * @file bus.c
 * @author 唐碧飞
 * @brief 低级总线收发处理函数
 * 使用系统资源：
 *   1. GPIO
 *   2. Timer1计数器
 *   3. TWC (发码时使用)
 * 使用全局变量：
 *   无
 * 依赖应用模块：
 *   无
 * @version 0.1
 * @date 2020-07-09
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */
#include "phnx02.h"
#include "lib_include.h"
#include "globle.h"


#define PIN_CHECK_STABLE_CNT     10           //引脚检测稳定计数


//SEBUS RX控制
#define PIN_RX               (TWC->STS & TWC_STS_RXDATLEV)      //接收解码输入

//定时器1相关宏定义
#define Tim2SetDelayUs(x)   {TIMERS->CON = 0;\
	TIM2->CTCG1 = x;\
	TIMERS->INTCLR = (TIM_INTCLR_TIM2);\
	TIMERS->CON = (TIM_CON_TE_TIM2 | TIM_CON_TM_TIM2);}

#define Tim2FlagIsSet()     ((TIMERS->INTFLAG&TIM_INTFLAG_TIM2) == TIM_INTFLAG_TIM2)
#define Tim2GetCnt()        (TIM2->CTVAL)


/**
 * @brief 判断总线电平是否为高，多次检测防抖动在此实现
 * @return TRUE=1, FLASE=0
 */
static int BUS_IsHigh()
{
    // TODO
    int iTmp=0;
    int i;
    for(i=0;i<PIN_CHECK_STABLE_CNT;++i)
    {
	if(PIN_RX)
	    ++iTmp;
    }
	if(iTmp==PIN_CHECK_STABLE_CNT)
	return 1;
    else
	return 0;

}

/**
 * @brief 判断总线电平是否为低，多次检测防抖动在此实现
 * @return TRUE=1, FLASE=0
 */
static int BUS_IsLow()
{
    // TODO
    int iTmp=0;
    int i;
    for(i=0;i<PIN_CHECK_STABLE_CNT;++i)
    {
	if(!PIN_RX)
	    ++iTmp;
    }
	if(iTmp==PIN_CHECK_STABLE_CNT)
	return 1;
    else
	return 0;
}

/**
 * @brief 等待总线电平为设定模式
 * SEBUS各种码型的通用等待函数。容差在此实现
 * @param nLow  低电平，微秒
 * @param nHigh  高电平，微秒
 * @param nMaxWait  最长等待时间，微秒
 * @return 0=成功匹配模式 -1=失败(超时)
 */
int BUS_WaitPattern(int iLow, int iHigh, int iMaxWait)
{
    int iTmp0,iTmp1;int bRst=0;
    Tim2SetDelayUs(iMaxWait);
start:
    if( iLow > 0)
    {
    // TODO
        do{
            if( Tim2FlagIsSet()  ){
                return -1;}
        }while(!BUS_IsLow());
        iTmp0 = Tim2GetCnt();
        do{
            iTmp1 = Tim2GetCnt();
            if( Tim2FlagIsSet()  )
                return -1;
            if( !BUS_IsLow() )
                goto start;
        }while(((iTmp0-iTmp1)<iLow - (iLow >> 2)));
    }
    if( iHigh > 0)
    {
        do{
            if( Tim2FlagIsSet()  ){
            	return -1;}
        }while(!BUS_IsHigh());
        iTmp0 = Tim2GetCnt();
        do{
            iTmp1 = Tim2GetCnt();
            if( Tim2FlagIsSet() )
                return -1;
            if( !BUS_IsHigh() )
                goto start;
        }while(((iTmp0-iTmp1)<iHigh - (iHigh >> 2)));
    }
    return 0;

}

/**
 * @brief 等待总线电平为设定高级模式
 * 该高级模式在SWAN_BUS中使用， SEBUS不使用
 * SWAN_BUS各种码型的通用等待函数。容差在此实现
 * @param nLeadingLow  前导低电平，微秒
 * @param nLeadingHigh  前导高电平，微秒
 * @param nLow  低电平，微秒
 * @param nHigh  高电平，微秒
 * @param nMaxWait  最长等待时间，微秒
 * @return 0=成功匹配模式 -1=失败(超时)
 */
int BUS_WaitPatternEx(int iLeadingLow, int iLeadingHigh, int iLow, int iHigh, int iMaxWait)
{
    // SEBUS应用不需实现
	return 0;
}

/**
 * @brief 总线拉码， 使用TWC SEBUS模式直接拉码，无信道编码
 * @param nUs 拉码持续时间，微秒
 */
void BUS_PullCode(int iUs)
{
    TWC_SendEnable();
    TWC_WriteData(1);
    DelayNus(iUs);
    TWC_WriteData(0);
}
