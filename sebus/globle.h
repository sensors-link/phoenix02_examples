/**
 * @file globle.h
 * @author bifei.tang
 * @brief 全局变量声明与宏定义
 * @version 0.1
 * @date 2020-07-06
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */

#ifndef __GLOBLE_H

#define __GLOBLE_H
#include "smk.h"


#define FSYS              8000000        //系统运行频率
#define ADC_BIT    10

//@{status start
//定义状态标志
#define STATUS_NO_ADDRESS                    (1<<0)               //没有地址
#define STATUS_POWER_UP                      (1<<1)               //芯片上电启动标志
#define STATUS_ALARM                         (1<<2)               //火警状态
#define STATUS_ALARM_VERIFY                  (1<<3)               //火警确认状态
#define STATUS_POLLUTE_ALARM                 (1<<4)               //污染状态
#define STATUS_FAULT                         (1<<5)               //故障状态
#define STATUS_FAULT_VERIFY                  (1<<6)               //故障状态确认
#define STATUS_FAST_SMP                      (1<<7)               //快速采样
#define WAKE_FROM_LPT                        (1<<8)              //唤醒来自LPT

#define STATUS_DISABLE_ALARM_INT             (1<<9)              //禁止拉火警
//用户添加

//@}status end


/*@{
*记录标志位定义
*/
#define RECORD_FLAG_WR_FW_VERSION     (1<<0)                      //芯片上电写软件版本到E2PROM里标志
#define RECORD_UPDATE_BK_VALUE        (1<<1)                      //记录下背景值
#define RECORD_ALARM_VERIFY           (1<<2)                      //火警确认
#define RECORD_ALARM_REQUEST          (1<<3)                      //火警请求


/*
*记录标志位结束
@}*/

//全局变量声明
extern unsigned int g100MSCount;            //低功耗定时器中断计数值(100ms中断一次)

extern long gStatus;                //运行状态标志
extern long gRecordFlag;            //记录事件标志

extern long gGroupNumber;           //组号
extern long gAddrInGroup;           //组内地址

extern long gAlarmVerifyCount;     //火警确认计数
extern long bProtocol;              //协议 1=EPC  0=9K
extern unsigned char gDayCount;     //天计数

extern unsigned long gRamCheck;
extern SMK gSmk;

//外部函数声明
void DelayNus(unsigned int del);
void SoftReset(void);


#ifdef _DEBUG
extern int __wrap_printf(const char* fmt, ...);
#define printf(...)   __wrap_printf(__VA_ARGS__)
#else
#define printf(...)
#endif



#endif


