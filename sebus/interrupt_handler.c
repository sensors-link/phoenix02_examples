/**
 * @file interrupt_handler.c
 * @author bifei.tang
 * @brief  系统中断处理函数
 * @version 0.1
 * @date 2020-07-06
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */
#include "globle.h"
#include "phnx02.h"
#include "lib_include.h"
#include "period_event_handler.h"
#include "sebus.h"
#include "e2prom.h"
#include "smk.h"
#include "globle.h"

//命令
#define CMD_NormalPatrol		  0x01
#define CMD_AlarmPatrol			  0x08
#define CMD_ActionCommand		  0x16
//#define CMD_RemoveCommand		  0x1F
#define CMD_FaultPatrol			  0x06
#define CMD_FaultVerify			  0x18
//#define CMD_KeepLedStatePatrol	  0x09
#define CMD_SingleDotTest		  0x10
#define CMD_ReadAddress			  0x2F
#define CMD_WriteAddress		  0x55
#define CMD_ReadWorkMode		  0x2B
#define CMD_WriteWorkMode0		  0x51
#define CMD_WriteWorkMode1		  0x54
#define CMD_WriteWorkMode2		  0x5A
#define CMD_ReadBackgroundAlarm	  0x2C
#define CMD_WriteBackground		  0x69
#define CMD_WriteAlarm			  0x62
#define CMD_WriteAlarm1			  0x64
#define CMD_WriteAlarm2			  0x66
#define CMD_ReadCompensation	  0x26
//#define CMD_Broadcast			  0x7F
#define CMD_MaskCommand			  0x13
#define CMD_ReadADvalue			  0x38

#define LED_ON       {ANAC_WPT_UNLOCK();ANAC->LED_CFG |= ANAC_LED_CFG_INDLED_EN;}
#define LED_OFF      {ANAC_WPT_UNLOCK();ANAC->LED_CFG &= ~ANAC_LED_CFG_INDLED_EN;}

/**
 * @brief 命令寄存器结构体
 *
 */
struct  CmdAddrBit
{
    unsigned long addr:9;
    unsigned long cmd:7;
    unsigned long rsv:16;
}__attribute__((packed));

/**
 * @brief 命令寄存器组合体
 *
 */
union CmdAddr
{
    unsigned long cmdReg;
    struct CmdAddrBit cmdAddrBits;
}gCmdAddr;

#define CMD_FLAG_ENCODE_EN1                (1<<0)  //编码控制使能1
#define CMD_FLAG_ENCODE_EN2                (1<<1)  //编码控制使能2
#define CMD_FLAG_WR_WK_MODE_EN             (1<<2)  //写工作模式使能
#define CMD_FLAG_WR_BK_EN                  (1<<3)  //写背景值使能
#define CMD_FLAG_WR_SIG_BK_EN              (1<<4)  //写单个背景值使能
#define CMD_FLAG_WR_ALM_EN                 (1<<5)  //写警报阈值使能
#define CMD_FLAG_WR_SIG_ALM_EN             (1<<6)  //写单个警报阈值使能

#define CMD_FLAG_MASK                      (1<<7)

//回码中探测器的状态常量
#define WORK_MODE0_STATE              0x01		//默认灵敏工作模式
#define WORK_MODE1_STATE              0x02
#define WORK_MODE2_STATE              0x03
#define SENSOR_FAULT_STATE            0x04
#define SENSOR_POLLUTE_STATE          0x05
#define SENSOR_ALARM_STATE            0x07
#define SENSOR_FAULT_VERIFY_STATE     0x0C
#define SENSOR_POLLUTE_VERIFY_STATE   0x0D
#define SENSOR_ALARM_VERIFY_STATE     0x0F


static long gCmdFlag;
u16 fbADVal;

/**
 * @brief 偶校验计数
 *
 * @param data
 * @return int
 */
static int EvenCheck(unsigned short data)
{
    int i,icnt=0;
    for(i=0;i<16;++i)
    {
	if( data&0x01)
	{
	    ++icnt;
	}
	data >>= 1;
    }
    if( icnt&0x01 )
    {
	return 1;
    }
    else
    {
	return 0;
    }
}

/**
 * @brief 低功耗定时器中断处理
 *
 */
void LPT_IrqHandler(void){
    long i;
    ++g100MSCount;
    for(i=0;i<EVENT_100MS_NUM;++i)
    {
        if( gEvent100MS.ulEventCount[i] > 0 )
            --gEvent100MS.ulEventCount[i];
    }
    LPT_ClrIntFlag();
    gStatus |= WAKE_FROM_LPT;
};

/**
 * @brief Get the Detector State
 *
 */
static int GetDetectorState()
{
    u16 state;
    if( gSmk.extStatus == SMK_EXT_STATUS_POLLUTED)
        return SENSOR_POLLUTE_STATE;
    switch(gSmk.status)
    {
        case SMK_STATUS_NORMAL:
        case SMK_STATUS_POWERON:
        if( ReadDataFromE2PROM(E2PROM_WORKMODE_ADDR,&state) )
        {
            switch (state)
            {
            case 0:
                state = WORK_MODE0_STATE;
                break;
            case 1:
                state = WORK_MODE1_STATE;
                break;
            case 2:
                state = WORK_MODE2_STATE;
                break;
            default:
                state = WORK_MODE0_STATE;
                break;
            }
        }
        else
        {
            return WORK_MODE0_STATE;
        }
        return state;
        break;
        case SMK_STATUS_SENSOR_ERR:
        return SENSOR_FAULT_STATE;
        break;
        case SMK_STATUS_FIRE:
        return SENSOR_ALARM_STATE;
        break;
    }
}

/**
 * @brief 二总线接收命令匹配中断处理
 *
 */
void TWC_IrqHandler(void){
    int i;int iTmp;
    static int iTmpBak,iAlarmRequestCount=20,iNormalPatrolCount=3;
    gCmdAddr.cmdReg = TWC->RXD;     //取得命令地址寄存器数据
    TWC_ClrIRQFlag(TWC_RX_FRAME_END);        //清中断标志
    TWC->CR &= ~TWC_CR_RXRECEN;              //禁止硬件解码
    switch (gCmdAddr.cmdAddrBits.cmd)
    {
        case CMD_NormalPatrol		 :        //正常巡检
        {
        unsigned short tmp = (gGroupNumber-1)*18 + gAddrInGroup;
        if( gCmdAddr.cmdAddrBits.addr == tmp)
        {
            int iDetectorState,iDetectorValue;
            iDetectorState = GetDetectorState();
            iDetectorValue = SMK_GetDetectorValue(&gSmk);
            SEBUS_Feedback(bProtocol, (( (0x02<<2) | (iDetectorState>>2) ) << 8) | ( (iDetectorState << 6) | iDetectorValue) );  //6bit：类型， 02:光电探测器  4bit：状态，6bit：探测值
            gEvent100MS.ulEventCount[EVENT_ADC_SMP] = 0;      //没巡检到自己就立即采样一次 中断中不会产生中断不需要保护
            if( (gSmk.status == SMK_STATUS_FIRE) && ((gStatus & STATUS_DISABLE_ALARM_INT)==0) )
            {
                --iAlarmRequestCount;
                if( iAlarmRequestCount == 0)
                {
                    gStatus |=~STATUS_DISABLE_ALARM_INT;
                }
                gRecordFlag |= RECORD_ALARM_REQUEST;
            }
            else
            {
                --iNormalPatrolCount;
                if(iNormalPatrolCount == 0)
                {
                    iNormalPatrolCount = 3;
                    iAlarmRequestCount = 20;
                }
            }
        }
        printf("NormalPatrol:%x\r\n",iNormalPatrolCount);
        }
        break;
        case CMD_AlarmPatrol		 :        //火警巡检
        if( gCmdAddr.cmdAddrBits.addr == gGroupNumber)
        {
            if( gSmk.status == SMK_STATUS_FIRE)
            {
                SEBUS_Feedback(bProtocol,(1<<(18-gAddrInGroup)));
            }
            printf("AlarmPatrol:%x\r\n",(1<<(18-gAddrInGroup)));
        }
        gAlarmVerifyCount = 0;
        break;
        case CMD_ActionCommand		 :        //火警确认
        {
        unsigned short tmp = (gGroupNumber-1)*18 + gAddrInGroup;
        if( gCmdAddr.cmdAddrBits.addr == tmp )
        {
            int iDetectorState,iDetectorValue;
            LED_ON;
            ++gAlarmVerifyCount;
            gStatus |= STATUS_DISABLE_ALARM_INT;             //禁止拉码
            gRecordFlag |= RECORD_ALARM_VERIFY;             //记录
            iDetectorState = SENSOR_ALARM_VERIFY_STATE;
            iDetectorValue = SMK_GetDetectorValue(&gSmk);
            SEBUS_Feedback(bProtocol, (( (0x02<<2) | (iDetectorState>>2) ) << 8) | ( (iDetectorState << 6) | iDetectorValue) );  //6bit：类型， 02:光电探测器  4bit：状态，6bit：探测值
            printf("alm act:%x\r\n",(( (0x02<<2) | (iDetectorState>>2) ) << 8) | ( (iDetectorState << 6) | iDetectorValue));
        }
        }
        break;
        // case CMD_RemoveCommand		 :
        // break;
        case CMD_FaultPatrol		 :        //故障巡检
        if( gCmdAddr.cmdAddrBits.addr == gGroupNumber)
        {
            if( (gSmk.status != SMK_STATUS_SENSOR_ERR) && (gSmk.extStatus != SMK_EXT_STATUS_POLLUTED) )
            {
                SEBUS_Feedback(bProtocol,(1<<(18-gAddrInGroup)));
            }
            printf("FaultPatrol:%x\r\n",(1<<(18-gAddrInGroup)));
        }
        break;
        case CMD_FaultVerify		 :        //故障确认
        {
        unsigned short tmp = (gGroupNumber-1)*18 + gAddrInGroup;
        if( gCmdAddr.cmdAddrBits.addr == tmp)
        {
            int iDetectorState,iDetectorValue;
            if( gSmk.status == SMK_STATUS_SENSOR_ERR)
                iDetectorState = SENSOR_FAULT_VERIFY_STATE;
            else if( gSmk.extStatus == SMK_EXT_STATUS_POLLUTED)
            {
                iDetectorState = SENSOR_POLLUTE_VERIFY_STATE;
            }
            else
            {
                return;
            }
            iDetectorValue = SMK_GetDetectorValue(&gSmk);
            SEBUS_Feedback(bProtocol, (( (0x02<<2) | (iDetectorState>>2) ) << 8) | ( (iDetectorState << 6) | iDetectorValue) );  //6bit：类型， 02:光电探测器  4bit：状态，6bit：探测值
            printf("FaultVerify:%x\r\n",(( (0x02<<2) | (iDetectorState>>2) ) << 8) | ( (iDetectorState << 6) | iDetectorValue));
        }
        }
        break;
        // case CMD_KeepLedStatePatrol	 :
        // break;
        case CMD_SingleDotTest		 :       //单点测试
        {
			unsigned short tmp = (gGroupNumber-1)*18 + gAddrInGroup;
			if( gCmdAddr.cmdAddrBits.addr == tmp)
			{
				int iDetectorState,iDetectorValue;
				gEvent100MS.ulEventCount[EVENT_ADC_SMP] = 0;      //立即采样 中断中不会产生中断不需要保护
				gStatus |= STATUS_DISABLE_ALARM_INT;
				LED_ON;
				iDetectorState = GetDetectorState();
				iDetectorValue = SMK_GetDetectorValue(&gSmk);
				SEBUS_Feedback(bProtocol,(( (0x02<<2) | (iDetectorState>>2) ) << 8) | ( (iDetectorState << 6) | iDetectorValue) );  //6bit：类型， 02:光电探测器  4bit：状态，6bit：探测值
				printf("sigtest:%x\r\n",(( (0x02<<2) | (iDetectorState>>2) ) << 8) | ( (iDetectorState << 6) | iDetectorValue));
			}
        }
        break;
        case CMD_ReadAddress		 :       //读地址
        gCmdFlag = 0;
        if( gCmdAddr.cmdAddrBits.addr == 0x55)  //读地址
        {
            LED_ON;      //读地址亮灯
            if( gStatus & STATUS_NO_ADDRESS)
            {
                SEBUS_Feedback(bProtocol,0x01ff);        //没地址回0x01ff
            	printf("no addr\r\n");
            }
            else
            {
                unsigned short tmp = (gGroupNumber-1)*18 + gAddrInGroup;
                int  even = EvenCheck(tmp);
                SEBUS_Feedback(bProtocol, (tmp<<2) | 0x02 | even);
            	printf("ack addr\r\n");
            }
        }
        else if( gCmdAddr.cmdAddrBits.addr == 0xaa)  //使能编码1
        {
        	printf("en encode\r\n");
            gCmdFlag |= CMD_FLAG_ENCODE_EN1;
        }
        break;
        case CMD_WriteAddress		 :       //写地址
        {
            static u16 addrTmp;
            if( !(gCmdFlag & CMD_FLAG_ENCODE_EN2))
            {
                if(gCmdFlag & CMD_FLAG_ENCODE_EN1)
                {
                    gCmdFlag &= ~CMD_FLAG_ENCODE_EN1;
                    gCmdFlag |= CMD_FLAG_ENCODE_EN2;
                    if( (gCmdAddr.cmdAddrBits.addr >=1) && (gCmdAddr.cmdAddrBits.addr < 325) )
                    {
                        addrTmp = gCmdAddr.cmdAddrBits.addr;      //备份地址
                    }
                    else
                    {
                    	printf("addr err%x\r\n",gCmdAddr.cmdAddrBits.addr);
                        SoftReset();            //写地址错误直接复位闪两下
                    }
                }
            }
            else
            {
                gCmdFlag &= ~CMD_FLAG_ENCODE_EN2;
                if( gCmdAddr.cmdAddrBits.addr != addrTmp )     //地址不同直接复位闪两下
                {
                	printf("addr%x!=addrtmp%x\r\n",gCmdAddr.cmdAddrBits.addr,addrTmp);
                    SoftReset();
                }
                else                    //两次相同写地址就编码地址成功后复位重新装载地址有关节点数据
                {
                    WriteDataToE2PROM(E2PROM_NODE_ADDR,addrTmp);
                    printf("addr%x,nodeAddr:%d,%x,wr ok\r\n",E2PROM_NODE_ADDR,addrTmp);
                    SoftReset();
                }
            }
        }
        break;
        case CMD_ReadWorkMode		 :       //读工作模式
            if( ReadDataFromE2PROM(E2PROM_WORKMODE_ADDR,&iTmp) )
            {
                SEBUS_Feedback(bProtocol,(iTmp<<8) | ((u8)~iTmp));
            }
            printf("readwmok:%x\r\n",iTmp);
        break;
        case CMD_WriteWorkMode0		 :       //写工作模式0
            iTmp = 0;
            goto write_work_mode;
        case CMD_WriteWorkMode1		 :       //写工作模式1
            iTmp = 1;
            goto write_work_mode;
        case CMD_WriteWorkMode2		 :       //写工作模式2
            iTmp = 2;
write_work_mode:
            if( gCmdAddr.cmdAddrBits.addr == 0x1f5)
            {
                if( (gCmdFlag & CMD_FLAG_WR_WK_MODE_EN) == 0)
                {
                    gCmdFlag |= CMD_FLAG_WR_WK_MODE_EN;
                    iTmpBak = iTmp;
                    printf("g wrwm:1:%x\r\n",iTmp);
                }
                else
                {
                    gCmdFlag &= ~CMD_FLAG_WR_WK_MODE_EN;
                    if( iTmpBak == iTmp)
                    {
                        WriteDataToE2PROM(E2PROM_WORKMODE_ADDR,iTmp);
                        printf("g wrwm:2:%x\r\n",iTmp);
                        SoftReset();
                    }
                }
            }
            else
            {
                unsigned short tmp = (gGroupNumber-1)*18 + gAddrInGroup;
                if( (gCmdFlag & CMD_FLAG_WR_WK_MODE_EN) == 0)
                {
					if( gCmdAddr.cmdAddrBits.addr == tmp)
					{
                        gCmdFlag |= CMD_FLAG_WR_WK_MODE_EN;
                        iTmpBak = iTmp;
                        SEBUS_Feedback(bProtocol,(iTmp<<8) | ((u8)~iTmp));
                        printf("wrwm:1:%x\r\n",iTmp);
                    }
                }
                else
                {
                    gCmdFlag &= ~CMD_FLAG_WR_WK_MODE_EN;
                    if( iTmpBak == iTmp)
                    {
                        WriteDataToE2PROM(E2PROM_WORKMODE_ADDR,iTmp);
                        printf("wrwm:2:%x\r\n",iTmp);
                        SoftReset();
                    }
                }
            }
        break;
        case CMD_ReadBackgroundAlarm	 :   //读背景值警报阈值
            {
                u16 u16DemarcateBk,u16DemarcateAlm;
                if( ReadDataFromE2PROM(E2PROM_DEMARCATE_BACKGROUND_ADDR,&u16DemarcateBk) )
                {
                    if( ReadDataFromE2PROM(E2PROM_DEMARCATE_ALARM_ADDR,&u16DemarcateAlm) )
                    {
                        SEBUS_Feedback(bProtocol,(u16DemarcateBk<<8) | u16DemarcateAlm);
                        printf("rdbk:%x,alm:%x\r\n",u16DemarcateBk,u16DemarcateAlm);
                    }
                }
            }
        break;
        case CMD_WriteBackground	 :       //写背景值
            if( gCmdAddr.cmdAddrBits.addr == 0x1f5)
            {
                if( (gCmdFlag & CMD_FLAG_WR_BK_EN) == 0 )
                {
                    gCmdFlag |= CMD_FLAG_WR_BK_EN;
                    printf("g wrbk:1\r\n");
                }
                else
                {
                    gCmdFlag &= ~CMD_FLAG_WR_BK_EN;
                    if( (gSmk.filterVal >= 0x10) && (gSmk.filterVal <= 0x50) )
                    {
                        WriteDataToE2PROM(E2PROM_DEMARCATE_BACKGROUND_ADDR,gSmk.filterVal);
                        printf("g wrbk:2\r\n");
                        SoftReset();
                    }
                    else
                    {
                        SEBUS_Feedback(bProtocol,0xff00);
                        printf("g wrbk err:2:%x\r\n");
                    }
                }
            }
            else
            {
                unsigned short tmp = (gGroupNumber-1)*18 + gAddrInGroup;
                if( (gCmdFlag & CMD_FLAG_WR_SIG_BK_EN) == 0)
                {
                    if( gCmdAddr.cmdAddrBits.addr == tmp)
                    {
                        gCmdFlag |= CMD_FLAG_WR_SIG_BK_EN;
                        SEBUS_Feedback(bProtocol, ((u16)gSmk.filterVal<<8) | ((u8)~gSmk.filterVal));
                    }
                    printf("wrbk:1:%x,tmp:%x\r\n",gCmdAddr.cmdAddrBits.addr,tmp);
                }
                else
                {
                    gCmdFlag &= ~CMD_FLAG_WR_SIG_BK_EN;
                    if( (gCmdAddr.cmdAddrBits.addr >= 0x10) && (gCmdAddr.cmdAddrBits.addr <= 0x50) )
                    {
                        WriteDataToE2PROM(E2PROM_DEMARCATE_BACKGROUND_ADDR,gCmdAddr.cmdAddrBits.addr);
                        printf("wrbk:2:%x\r\n",gCmdAddr.cmdAddrBits.addr);
                        SoftReset();
                    }
                    else
                    {
                        SEBUS_Feedback(bProtocol,0xff00);
                    }
                }

            }
        break;
        case CMD_WriteAlarm		 :           //写警报值
            iTmp = 0;
            goto write_alarm;
        case CMD_WriteAlarm1		 :       //写警报值1
            iTmp = 1;
            goto write_alarm;
        case CMD_WriteAlarm2		 :       //写警报值2
            iTmp = 2;
write_alarm:
            if( gCmdAddr.cmdAddrBits.addr == 0x01f5)
            {
                if( (gCmdFlag & CMD_FLAG_WR_ALM_EN) == 0 )
                {
                    gCmdFlag |= CMD_FLAG_WR_ALM_EN;
                    iTmpBak = iTmp;
                    printf("g wralm:1:%x\r\n",iTmp);
                }
                else
                {
                    gCmdFlag &= ~CMD_FLAG_WR_ALM_EN;
                    if( (gSmk.filterVal > gSmk.bgVal) && (iTmpBak == iTmp) )
                    {
                        WriteDataToE2PROM(E2PROM_DEMARCATE_ALARM_ADDR + (iTmp<<2),gSmk.filterVal);
                        printf("g wralm:2:%x\r\n",iTmp);
                        SoftReset();
                    }
                    else
                    {
                        SEBUS_Feedback(bProtocol,0x00ff);
                    }
                }
            }
            else
            {
                unsigned short tmp = (gGroupNumber-1)*18 + gAddrInGroup;
                if( (gCmdFlag & CMD_FLAG_WR_SIG_ALM_EN) == 0 )
                {
                    if( gCmdAddr.cmdAddrBits.addr == tmp)
                    {
                    gCmdFlag |= CMD_FLAG_WR_SIG_ALM_EN;
                    iTmpBak = iTmp;
                     SEBUS_Feedback(bProtocol, ((u16)gSmk.filterVal<<8) | ((u8)~gSmk.filterVal));
                     printf("wralm:1:%x\r\n",gSmk.filterVal);
                    }
                }
                else
                {
                    gCmdFlag &= ~CMD_FLAG_WR_SIG_ALM_EN;
                    if( (gCmdAddr.cmdAddrBits.addr > gSmk.bgVal) && (iTmpBak == iTmp))
                    {
                        WriteDataToE2PROM(E2PROM_DEMARCATE_ALARM_ADDR + (iTmp<<2),gCmdAddr.cmdAddrBits.addr);
                        printf("wralm:2:%x\r\n",gCmdAddr.cmdAddrBits.addr);
                        SoftReset();
                    }
                    else
                    {
                        SEBUS_Feedback(bProtocol,0x00ff);
                    }
                }
            }
        break;
        case CMD_ReadCompensation	 :       //读补偿值
        {
            u16 u16DemarcateBk;
            if( ReadDataFromE2PROM(E2PROM_DEMARCATE_BACKGROUND_ADDR,&u16DemarcateBk) )
            {
                if( gSmk.bgVal > u16DemarcateBk )
                {
                    SEBUS_Feedback(bProtocol,( (gSmk.bgVal - u16DemarcateBk)<<8) | ((u8)~(gSmk.bgVal - u16DemarcateBk)));
                    printf("rd comp:%x\r\n",(gSmk.bgVal - u16DemarcateBk));
                }
            }
        }
        break;
        // case CMD_Broadcast		 :
        // break;
        case CMD_MaskCommand		 :       //屏蔽命令
            iTmp = 0x5a;
            if( gCmdFlag & CMD_FLAG_MASK)
            {
                iTmp = 0;
            }
            gCmdFlag |= CMD_FLAG_MASK;
            gStatus |= STATUS_DISABLE_ALARM_INT;
            SEBUS_Feedback(bProtocol,(iTmp<<8));
            printf("msk:%x\r\n",iTmp);
        break;
        case CMD_ReadADvalue		 :       //读AD值
            gEvent100MS.ulEventCount[EVENT_ADC_SMP] = 0;      //立即采样 中断中不会产生中断不需要保护
            SEBUS_Feedback(bProtocol,( (fbADVal>>(ADC_BIT-8))<<8) | ((u8)~(fbADVal>>(ADC_BIT-8))));
            printf("adv:%x\r\n",fbADVal);
        break;
    default:
        break;
    }
    TWC->CR |= TWC_CR_RXRECEN; //使能硬件解码
};


