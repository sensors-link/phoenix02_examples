/**
 * @file main.c
 * @author bifei.tang
 * @brief 烟感产品主程序
 * @version 0.1
 * @date 2020-07-03
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */
#include "phnx02.h"
#include "lib_include.h"
#include "globle.h"
#include "period_event_handler.h"
#include "sebus.h"
#include "smk.h"
#include "e2prom.h"


#define WDT_RST_N_MS      2000           //看门狗复位时间单位ms：如2000 = 2s

//参数相关宏定义
#define BG_VAL_NORMAL_MAX      0x50      //正常下背景值最大值
#define SMOKE_SMP_MAX_VAL      0xe0     //烟雾采样极限值

#define BG_VAL_DEF             0x28      //默认背景值
#define ALARM_VAL_DEF          0x68      //默认警报阈值


void BoardInit(void);

/**
 * @brief 从E2PROM里装载参数,并判断参数置相应标志位或配置默认值
 *
 */
static void LoadParamFromEEPROM(void)
{
    int i;u16 tmp;
    u8 WorkMode,BackGroupValue,AlarmMark,BackGroupValueTmp;
    //设备地址判断
    if( !ReadDataFromE2PROM(E2PROM_NODE_ADDR,&tmp) )   //没有编写地址
    {
    	printf("rd nodeAddrFail:%d\r\n",tmp);
        gStatus |= STATUS_NO_ADDRESS;
    }
    else                                                            //有地址进行地址范围检查与获得组号，组内地址
    {
        printf("nodeaddr:%d\r\n",tmp);
        if( (tmp > 0) && (tmp < 325) )                        //地址在范围内
        {
            gGroupNumber = (tmp%18 != 0) ? (tmp/18 + 1) : (tmp/18);
            gAddrInGroup = (tmp%18 != 0) ? (tmp % 18) : 18;
            TWC_SetCMDAndMask(TWC_CMD_3,gGroupNumber,0b0111111000000000);
            TWC_SetCMDAndMask(TWC_CMD_4,tmp,0b0111111000000000);
        }
        else
        {
            gStatus |= STATUS_NO_ADDRESS;
        }
    }
    //工作模式判断
    if( !ReadDataFromE2PROM(E2PROM_WORKMODE_ADDR,&tmp) )   //没有编写地址
    {
        WorkMode = 0;        //第一次E2PROM没有写工作模式，设置工作模式为0
    }
    else
    {
        if( WorkMode > 2 )      //判断工作模式是否正确，不正确设为0
        {
            WorkMode = 0;
        }
    }
    //背景值判断
    if( !ReadDataFromE2PROM(E2PROM_DEMARCATE_BACKGROUND_ADDR,&tmp) )
    {
        BackGroupValue = BG_VAL_DEF;
    }
    else
    {
        if(tmp < BG_VAL_NORMAL_MAX)
        {
            BackGroupValue = tmp;
        }
        else
        {
            BackGroupValue = BG_VAL_DEF;
        }
    }
    //警报阈值判断
    if( !ReadDataFromE2PROM(E2PROM_DEMARCATE_ALARM_ADDR + (WorkMode<<2),&tmp) )
    {
        AlarmMark = ALARM_VAL_DEF;
    }
    else
    {
        AlarmMark = tmp;
    }
    //补偿背景值判断
    if( !ReadDataFromE2PROM(E2PROM_COMPENSATION_BACKGROUND_ADDR,&tmp) )
    {
        BackGroupValueTmp = BackGroupValue;
    }
    else
    {
        BackGroupValueTmp  = tmp;
    }
    printf("bk:%x,alm:%d,bkTmp:%d\r\n",BackGroupValue,AlarmMark,BackGroupValueTmp);
    SMK_Init(&gSmk,BackGroupValue,AlarmMark,BackGroupValueTmp);
    //记录数据指针判断
    if( !ReadDataFromE2PROM(E2PROM_RECORD_POINTER_ADDR,&tmp) )
    {
        //指针无效写入记录地址
    	WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,E2PROM_RECORD_ADDR_START);
    }
    else
    {
        if( (tmp < E2PROM_RECORD_ADDR_START) || (tmp >= E2PROM_RECORD_ADDR_END) )
        {
            //指针无效写入记录地址
            WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,E2PROM_RECORD_ADDR_START);
        }
    }
}

/**
 * @brief 特定事件发生记录数据到E2PROM
 *
 */
static void RecordHandler(void)
{
    u16 tmp;
    if(!gRecordFlag)        //没有需要记录的数据返回
        return;
    if( gRecordFlag & RECORD_FLAG_WR_FW_VERSION)    //是否刚上电记录软件版本
    {
        WriteDataToE2PROM(E2PROM_FW_VERSION_ADDR,FW_VERSION);
        gRecordFlag &= ~RECORD_FLAG_WR_FW_VERSION;        //写成功清记录标志
    }
    else if( gRecordFlag & RECORD_UPDATE_BK_VALUE)           //记录背景值到E2PROM
    {
    	WriteDataToE2PROM(E2PROM_COMPENSATION_BACKGROUND_ADDR,gSmk.bgVal);
        gRecordFlag &= ~RECORD_UPDATE_BK_VALUE;        //写成功清记录标志
    }
    else if( gRecordFlag & RECORD_ALARM_REQUEST)
    {
        u16 RecordPointer;
        if(!ReadDataFromE2PROM(E2PROM_RECORD_POINTER_ADDR,&RecordPointer))
        {
            WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,E2PROM_RECORD_ADDR_START);
            RecordPointer = E2PROM_RECORD_ADDR_START;
        }
        WriteDataToE2PROM(RecordPointer,0x54);  //事件代码：火警请求背景值
        RecordPointer += 4;
        WriteDataToE2PROM(RecordPointer,gSmk.bgVal);
        RecordPointer += 4;
        if( RecordPointer >= E2PROM_RECORD_ADDR_END)
        {
            RecordPointer= E2PROM_RECORD_ADDR_START;
        }
        WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,RecordPointer);
        if( ReadDataFromE2PROM(E2PROM_WORKMODE_ADDR,&tmp))
        {
            if(!ReadDataFromE2PROM(E2PROM_RECORD_POINTER_ADDR,&RecordPointer))
            {
                WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,E2PROM_RECORD_ADDR_START);
                RecordPointer = E2PROM_RECORD_ADDR_START;
            }
            WriteDataToE2PROM(RecordPointer,0x53 + (1<<tmp));  //事件代码：火警请求是AD采样滤波值
            RecordPointer += 4;
            WriteDataToE2PROM(RecordPointer,gSmk.filterVal);
            RecordPointer += 4;
            if( RecordPointer >= E2PROM_RECORD_ADDR_END)
            {
                RecordPointer= E2PROM_RECORD_ADDR_START;
            }
            WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,RecordPointer);
        }
        gRecordFlag &= ~RECORD_ALARM_REQUEST;
    }
    else if(gRecordFlag & RECORD_ALARM_VERIFY)
    {
        u16 RecordPointer;
        if(!ReadDataFromE2PROM(E2PROM_RECORD_POINTER_ADDR,&RecordPointer))
        {
            WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,E2PROM_RECORD_ADDR_START);
            RecordPointer = E2PROM_RECORD_ADDR_START;
        }
        WriteDataToE2PROM(RecordPointer,0x54);  //事件代码：火警确认
        RecordPointer += 4;
        WriteDataToE2PROM(RecordPointer,gAlarmVerifyCount);
        RecordPointer += 4;
        if( RecordPointer >= E2PROM_RECORD_ADDR_END)
        {
            RecordPointer= E2PROM_RECORD_ADDR_START;
        }
        WriteDataToE2PROM(E2PROM_RECORD_POINTER_ADDR,RecordPointer);

        gRecordFlag &= ~RECORD_ALARM_VERIFY;
    }

}

/**
 * @brief 火警时快速拉码处理
 *
 */
static void AlarmAndProtocolHandler(void)
{
    int iTmp;int i;
    int mode,txLelCfg,rxDecCfg,rxGltchFiltCfg;
    if( (bProtocol == -1) || (gStatus & STATUS_ALARM) )
    {
        iTmp = SEBUS_WaitStartBit();
        if( iTmp == -1)
        {
            return;
        }
        if( bProtocol == -1 )
        {
            bProtocol = iTmp;
            if( bProtocol == 1)
            {
                TWC_SetGapAndGapComp(150,60);
            }
            else
            {
                TWC_SetGapAndGapComp(120,60);
            }
            TWC_SEBUSConfig(TWC_MODE_EPC,TWC_TX_LEVEL_HIGH_EN,TWC_RX_DEC_MATCH_CMD_INT,TWC_RX_FILT_2N_CYCLE(2));
            printf("bP%d\r\n",bProtocol);
        }
        else
        {
            if( (gStatus & STATUS_DISABLE_ALARM_INT ) == 0)
                SEBUS_SendAlarmInterrupt(iTmp, gGroupNumber);
        }

    }

}


/**
 * @brief 进入深睡眠
 *
 */
static void DeepSleep(void)
{
    do{
    DisableGlobleIRQ();
    PMU_EnterDeepSleep();
    EnableGlobleIRQ();
    for(int i=0;i<100;++i) asm("nop");
    DisableGlobleIRQ();
    }while( (gStatus & WAKE_FROM_LPT) == 0);
    gStatus &= ~WAKE_FROM_LPT;
    EnableGlobleIRQ();
}




int main(void)
{
	static unsigned int uiCnt;
#ifdef _WDT_EN
    WDT_Init(WDT_RST_N_MS,PMU_CR_LPTCLKSEL_LRC,WDT_OV_RST);
    WDT_StartCount();
#endif
    BoardInit();
    PeriodEventInit();
    LoadParamFromEEPROM();
    printf("test\r\n");
    uiCnt = g100MSCount;
    while(1)
    {
    #ifdef _WDT_EN
        WDT_ClrCount();
    #endif
        if(gStatus & STATUS_NO_ADDRESS)
        {
            DisableGlobleIRQ();
            if(g100MSCount - uiCnt > 20)            //2s后重新运行
            {
            	uiCnt = g100MSCount;
            	printf("2s");
                EnableGlobleIRQ();
#ifndef _DEBUG
                SoftReset();
#endif
            } else {
            	EnableGlobleIRQ();
            }
        }
        else
        {
            PeriodEventHandler();
            RecordHandler();
        }
        AlarmAndProtocolHandler();
        DeepSleep();
    }
    return 0;
}




