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
#include "bus.h"

#define SEBUS_MAX_FRAME_DURATION 64000     // 最大帧时长100ms
#define SEBUS_MAX_FRAME_SYNC_DURATION 20000 // 最大帧同步时长64ms
#define SEBUS_EPCMODE_MAX_DOWNLINK_BIT 2500  // EPC模式最大下行BIT时长2ms
#define SEBUS_9KMODE_MAX_DOWNLINK_BIT 1400   // 9K模式最大下行BIT时长1.4ms

#define SEBUS_EPCMODE_CODE_WIDTH 150  // EPC模式回码宽度150us
#define SEBUS_9KMODE_CODE_WIDTH 250   // 9K模式回码宽度250us
#define SEBUS_EPCMODE_ALARM_WIDTH 250 // EPC模式告警中断宽度300us
#define SEBUS_9KMODE_ALARM_WIDTH 250  // 9K模式告警中断宽度250us

/**
 * @brief 等待直到一个可用的SEBUS帧开始位
 * 通过开始位特征，可判断总线协议是EPC模式还是9K模式。
 * 该函数可在下面情况使用
 * 1. 初始化时判断总线类型
 * 2. 拉报警中断时等待开始位
 * @return 1=等到EPC模式开始位 0=等到9000模式开始位 -1=失败(SEBUS_MAX_FRAME_DURATION时间内未能等到帧开始位)
 */
int SEBUS_WaitStartBit(void)
{
    int ret;
    // 等待20ms高电平，帧同步
    ret = BUS_WaitPattern(0, 20000, SEBUS_MAX_FRAME_DURATION);
    if (ret != 0)
        return -1;
    // 等待400us低电平400us高电平，9000模式起始帧
    ret = BUS_WaitPattern(400, 400, SEBUS_MAX_FRAME_SYNC_DURATION);
    if (ret != 0)
        return -1;
    // 高电平继续持续2100us，为EPC模式起始帧
    ret = BUS_WaitPattern(0, 2100, 2200);
    return ret ? 0 : 1;
}


/**
 * @brief SEBUS在发码区发送报警中断脉冲
 * 需在SEBUS_WaitStartBit检测到开始位后立即调用
 * @param bEPCMode 是否EPC模式
 * @param nPos ：中断脉冲位置，等于组号
 * @return 0=成功发送 -1=失败
 */
int SEBUS_SendAlarmInterrupt(int bEPCMode, int nPos)
{
	int ret;
    for (int i = 0; i < nPos - 1; i++)
    {
        if (bEPCMode)
        {
            ret = BUS_WaitPattern(250, 1250, SEBUS_EPCMODE_MAX_DOWNLINK_BIT);
        }
        else
        {
            ret = BUS_WaitPattern(200, 800, SEBUS_9KMODE_MAX_DOWNLINK_BIT);
        }
        if (ret != 0)
            return -1;
    }
    if (bEPCMode)
    {
        ret = BUS_WaitPattern(50, 0, SEBUS_EPCMODE_MAX_DOWNLINK_BIT); // FIXME
        if (ret != 0)
            return -1;
        BUS_PullCode(SEBUS_EPCMODE_ALARM_WIDTH);
    }
    else
    {
        ret = BUS_WaitPattern(200, 200, SEBUS_9KMODE_MAX_DOWNLINK_BIT); // FIXME
        if (ret != 0)
            return -1;
        BUS_PullCode(SEBUS_9KMODE_ALARM_WIDTH);
    }
    return 0;
}


/**
 * @brief SEBUS回码
 * 要求在总线空格码期间调用
 * @param bEPCMode 是否EPC模式
 * @param uCode 低18bit为回码内容
 * @return 0=成功发送 -1=失败
 */
int SEBUS_Feedback(int bEPCMode, unsigned int uCode)
{
    int ret;
    // 跳过空格码低电平期间
    BUS_WaitPattern(0, 500, 1000);
    // 开始回码
    for (int i = 0; i < 18; ++i)
    {
        if (bEPCMode)
        {
            // 在低电平第50us拉码
            ret = BUS_WaitPattern(50, 0, SEBUS_EPCMODE_MAX_DOWNLINK_BIT);
            if (ret != 0)
                return -1;
            if (uCode & 0x020000) // 从第18bit开始检查发送，0拉电流，1不拉电流
            {
                BUS_PullCode(SEBUS_EPCMODE_CODE_WIDTH);
            }
            ret = BUS_WaitPattern(0, 100, SEBUS_EPCMODE_MAX_DOWNLINK_BIT); // 跳过剩余低电平期间
            if (ret != 0)
                return -1;
        }
        else
        {
            // 在高电平第200us拉码
            ret = BUS_WaitPattern(200, 200, SEBUS_9KMODE_MAX_DOWNLINK_BIT);
            if (ret != 0)
                return -1;
            if (uCode & 0x020000) // 从第18bit开始检查发送，0拉电流，1不拉电流
            {
                BUS_PullCode(SEBUS_9KMODE_CODE_WIDTH);
            }
        }
        uCode <<= 1;
    }
    return 0;
}
