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



#ifndef  __BUS_H
#define  __BUS_H



int BUS_WaitSync(int iHigh, int iMaxWait);
int BUS_WaitPattern(int iLow, int iHigh, int iMaxWait);
int BUS_WaitPatternEx(int iLeadingLow, int iLeadingHigh, int iLow, int iHigh, int iMaxWait);
void BUS_PullCode(int iUs);





#endif





