/**
 * @file e2prom.c
 * @author 唐碧飞
 * @brief E2PROM操作函数
 * 使用系统资源：
 *   无
 * 使用全局变量：
 *   无
 * 依赖应用模块：
 *   efc
 * @version 0.1
 * @date 2020-07-09
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */

#ifndef __EEPROM_H
#define __EEPROM_H
#include "phnx02.h"

#define  FW_VERSION              0x10    //定义软件版本,如0x10为1.0版本

#define WR_E2PROM_FAIL_RETRY_TIMES  3     //写E2PROM失败重试次数

/*@{
*1K E2PROM Region Define Start
*/
#define  E2PROM_SIZE          1024
#define  E2PROM_BASE_ADDR     0x10180000

#define  E2PROM_FW_VERSION_ADDR                                0              //软件版本地址

#define  E2PROM_NODE_ADDR                                      0x08          //节点地址
#define  E2PROM_DEMARCATE_BACKGROUND_ADDR                      0x0c          //标定背景地址
#define  E2PROM_COMPENSATION_BACKGROUND_ADDR                   0x10          //补偿背景地址
#define  E2PROM_WORKMODE_ADDR                                  0x14          //工作模式地址
#define  E2PROM_RECORD_POINTER_ADDR                            0x18          //记录指针地址
#define  E2PROM_DEMARCATE_ALARM_ADDR                           0x1c   //0x20 //0x24       //灵敏 正常警报 迟钝


#define  E2PROM_RECORD_ADDR_START         0x30          //天记录数据开始地址
#define  E2PROM_RECORD_ADDR_END           E2PROM_SIZE   //天记录数据结束地址
/*
*1K E2PROM Region Define End
@}*/




void WriteDataToE2PROM(u16 addr,u16 dat);
BOOL ReadDataFromE2PROM(u16 addr,u16 *dat);
















#endif
