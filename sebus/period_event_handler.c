/**
 * @file event_handler.c
 * @author bifei.tang
 * @brief 周期事件处理子函数
 * @version 0.1
 * @date 2020-07-06
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */
#include "globle.h"
#include "lib_include.h"
#include "period_event_handler.h"
#include "smk.h"
#include "e2prom.h"
//事件变量定义
Event100MS_t gEvent100MS;


/**
 * @brief 烟感采样、归一化数据处理
 *
 * @param pvParam 采样100ms时间基准计数值
 */
static void ADCSampleEventHandler(void *pvParam)
{
	u16 u16ADVal;
    /*暂时省过程*/



	u16ADVal = ADC_GetAverageValue();
	printf("ADV:%x ",u16ADVal);
    extern u16 fbADVal;
    fbADVal = u16ADVal;
    SMK_Input(&gSmk,u16ADVal);
}


/**
 * @brief 补偿处理事件
 *
 * @param pvParam 暂时不用
 */
static void CompensationEventHandler(void *pvParam)
{
    static long gHourCountTmp;
    ++gHourCountTmp;
    printf("1hcmp\r\n");
    SMK_StaticRevise(&gSmk);
    if(gHourCountTmp >= 4)  //4小时记录背景值
    {
    	printf("4hwr_bkv\r\n");
        gHourCountTmp = 0;
        WriteDataToE2PROM(E2PROM_COMPENSATION_BACKGROUND_ADDR,gSmk.bgVal);
    }
}

/**
 * @brief 天事件记录
 *
 * @param pvParam 暂时不用
 */
static void DayEventHandler(void *pvParam)
{
    gDayCount += 0x10;
    gDayCount &= 0xf0;
    u16 RecordPointer;
    if(!ReadDataFromE2PROM(E2PROM_RECORD_POINTER_ADDR,&RecordPointer))
    {
        WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,E2PROM_RECORD_ADDR_START);
        RecordPointer = E2PROM_RECORD_ADDR_START;
    }
    WriteDataToE2PROM(RecordPointer,gDayCount | 0x85);
    RecordPointer += 4;
    WriteDataToE2PROM(RecordPointer,gSmk.bgVal);
    RecordPointer += 4;
    if( RecordPointer >= E2PROM_RECORD_ADDR_END)
    {
        RecordPointer= E2PROM_RECORD_ADDR_START;
    }
    WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,RecordPointer);
    printf("1drd,rcPoint:%d\r\n",RecordPointer);
}



/**
 * @brief 全局参数检测
 *
 */
static void RAMParamCheck(void *pvParam)
{
    int i;
    {
        u16 tmpH = ~(gRamCheck>>16);
        u16 tmpL = gRamCheck & 0xffff;
        printf("1mRamChk\r\n");
        if( tmpH != tmpL)
        {
            while(1);  //wait wdt rst
        }
    }

}




/**
 * @brief 周期事件初始化
 *
 */
void PeriodEventInit(void)
{
    gEvent100MS.pvEventHandler[EVENT_ADC_SMP] = ADCSampleEventHandler;
    gEvent100MS.ulPreiodInterval[EVENT_ADC_SMP] = ADC_SMP_CNT;
    gEvent100MS.ulEventCount[EVENT_ADC_SMP] = ADC_SMP_CNT;

    gEvent100MS.pvEventHandler[EVENT_COMPENSATION] = CompensationEventHandler;
    gEvent100MS.ulPreiodInterval[EVENT_COMPENSATION] = COMPENSATION_CNT;
    gEvent100MS.ulEventCount[EVENT_COMPENSATION] = COMPENSATION_CNT;

    gEvent100MS.pvEventHandler[EVENT_RECORD_DAY] = DayEventHandler;
    gEvent100MS.ulPreiodInterval[EVENT_RECORD_DAY] = RECORD_DAY_CNT;
    gEvent100MS.ulEventCount[EVENT_RECORD_DAY] = RECORD_DAY_CNT;

    gEvent100MS.pvEventHandler[EVENT_RAM_CHK] = RAMParamCheck;
    gEvent100MS.ulPreiodInterval[EVENT_RAM_CHK] = RAM_PARAM_CHK_CNT;
    gEvent100MS.ulEventCount[EVENT_RAM_CHK] = RAM_PARAM_CHK_CNT;

}


/**
 * @brief 周期事件处理函数
 *
 */
void PeriodEventHandler(void)
{
    int i;
    for(i=0;i<EVENT_100MS_NUM;++i)
    {
        DisableGlobleIRQ();
        if(gEvent100MS.ulEventCount[i] == 0)
        {
            gEvent100MS.ulEventCount[i] = gEvent100MS.ulPreiodInterval[i];
            EnableGlobleIRQ();
            gEvent100MS.pvEventHandler[i](gEvent100MS.pvParam);
        }
        EnableGlobleIRQ();
    }
}
