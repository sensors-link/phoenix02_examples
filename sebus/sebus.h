/**
 * @file sebus.c
 * @author 唐碧飞
 * @brief SEBUS帧处理帮助函数
 * 使用系统资源：
 *   无
 * 使用全局变量：
 *   无
 * 依赖应用模块：
 *   bus
 * @version 0.1
 * @date 2020-07-09
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */

#ifndef __SEBUS_H
#define __SEBUS_H




int SEBUS_WaitStartBit(void);
int SEBUS_SendAlarmInterrupt(int bEPCMode, int nPos);
int SEBUS_Feedback(int bEPCMode, unsigned int uCode);








#endif

