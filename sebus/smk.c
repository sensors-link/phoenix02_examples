/**
 * @file smk.c
 * @author 唐碧飞
 * @brief 对烟感算法的封装
 * 单纯的算法封装，不处理EEPROM参数加载、更新等。代码可用于PC环境下的算法验证。
 * 使用系统资源：
 *   无
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

#include "smk.h"
#include "globle.h"

// 取较小值
#define MIN(a, b) ((a) < (b) ? (a) : (b))
// 取较大值
#define MAX(a, b) ((a) > (b) ? (a) : (b))
// 取差值绝对值
#define DIFF(a, b) ((a) > (b) ? (a) - (b) : (b) - (a))
// 设定下限
#define LOWLIMIT(a, b) \
    if ((a) < (b))     \
    {                  \
        a = b;         \
    }
// 设定上限
#define HIGHLIMIT(a, b) \
    if ((a) > (b))      \
    {                   \
        a = b;          \
    }


/**
 * @brief 对采样值进行滤波处理
 * 1. 更新滤波值
 * 2. 判断采样值是否陡变，置SMK_EXT_STATUS_FAST_SAMPLE扩展状态
 * @param smk SMK上下文指针
 * @param adVal AD采样值
 */
static void SMK_Filter(SMK *smk, uint16_t adVal)
{
    if (smk->status == SMK_STATUS_POWERON && smk->powerOnCnt == 0)
    {
        // 首次采样滤波值等于采样值
        smk->filterVal16 = adVal;
    }
    else
    {
        // 后续采样，滤波值 = 滤波值 * 15 + 采样值
        smk->filterVal16 = smk->filterVal16 - (smk->filterVal16 >> 3) + (adVal >> 3);
    }
    smk->filterVal = smk->filterVal16 >> (ADC_BIT-8);

    // AD值突变，与滤波值发生较大偏差，进入快速采样模式
    if (DIFF(adVal, smk->filterVal16) > SMK_FAST_SAMPLE_THRESHOLD << 2)
    {
        smk->exitFastSampleCnt = 0;
        smk->extStatus |= SMK_EXT_STATUS_FAST_SAMPLE;
    }
    else
    {
        // 连续多次AD值稳定，才退出快速采样模式
        if (smk->extStatus & SMK_EXT_STATUS_FAST_SAMPLE)
        {
            smk->exitFastSampleCnt++;
            if (smk->exitFastSampleCnt > SMK_EXIT_FASTSAMPLE_CNT)
            {
                smk->extStatus &= ~SMK_EXT_STATUS_FAST_SAMPLE;
                smk->exitFastSampleCnt = 0;
            }
        }
    }
}

/**
 * @brief 对背景值进行跟踪
 * @param smk SMK上下文指针
 */
static void SMK_UpdateBGTracker(SMK *smk)
{
    // “BackGroundTemp1 算法”
    // BackGroundTemp1,跟随每次AD采样结果的变化
    // 用一个uint16临时变量防止溢出
    uint16_t bgTracker = smk->bgTracker + (smk->bgTracker > smk->filterVal ? -1 : 1);
    bgTracker += bgTracker < 1 ? 1 : 0;
    bgTracker -= bgTracker > 255 ? 1 : 0;
    smk->bgTracker = (uint8_t)bgTracker;
}

/**
 * @brief 判断传感器故障
 *
 * @param smk SMK上下文指针
 * @param adVal AD采样值（未使用）
 * @return int 是否传感器故障 1=TRUE 0=FALSE
 */
static int SMK_IsSensorErr(SMK *smk, uint16_t adVal)
{
    (void)adVal; // 未使用参数
//    return smk->filterVal <= SMK_SENSORERR_VALUE;
    // 连续多次滤波值超门限判定为故障
    if (smk->filterVal <= SMK_SENSORERR_VALUE)
    {
        smk->sensorErrCnt++;
        if (smk->sensorErrCnt > SMK_SENSORERR_CNT)
        {
            return 1; // TRUE
        }
    }
    else
    {
        smk->sensorErrCnt = 0;
    }
    return 0; // FALSE
}

/**
 * @brief 判断传感器故障恢复
 *
 * @param smk SMK上下文指针
 * @param adVal AD采样值（未使用）
 * @return int 是否传感器故障恢复 1=TRUE 0=FALSE
 */
static int SMK_IsSensorRecovery(SMK *smk, uint16_t adVal)
{
    (void)adVal; // 未使用参数
    return smk->filterVal > SMK_SENSORERR_RESETVALUE ? 1 : 0;
}

/**
 * @brief 判断火警状态
 *
 * @param smk SMK上下文指针
 * @param adVal AD采样值
 * @return int 是否火警 1=TRUE 0=FALSE
 */
static int SMK_IsFireAlarm(SMK *smk, uint16_t adVal)
{
    // 连续多次当前采样值和滤波值双超门限判定为火警
    if (adVal > smk->fireThrd && smk->filterVal > smk->fireThrd)
    {
        smk->fireAlarmCnt++;
        if (smk->fireAlarmCnt > SMK_FIREALARM_CNT)
        {
            return 1; // TRUE
        }
    }
    else
    {
        smk->fireAlarmCnt = 0;
    }
    return 0; // FALSE
}

/**
 * @brief 判断火警恢复
 *
 * @param smk SMK上下文指针
 * @param adVal AD采样值
 * @return int 是否从火警中恢复 1=TRUE 0=FALSE
 */
static int SMK_IsFireRecovery(SMK *smk, uint16_t adVal)
{
    // 连续多次滤波值低于门限判定为火警恢复
    if (smk->filterVal < smk->fireThrd && smk->filterVal > SMK_SENSORERR_VALUE)
    {
        smk->fireRecoveryCnt++;
        if (smk->fireRecoveryCnt > SMK_FIRERECOVERY_CNT)
        {
            return 1; // TRUE
        }
    }
    else
    {
        smk->fireRecoveryCnt = 0;
    }
    return 0; // FALSE
}


/**
 * @brief 获得是否污染
 *
 * @param smk SMK上下文指针
 * @return int 1=TRUE 0=FALSE
 */
static int SMK_IsPolluted(SMK *smk)
{
    // 当前报警增量小于标定增量，则判断为污染
    return smk->fireThrd - smk->bgVal < smk->calibratedFireThrd - smk->calibratedBGVal ? 1 : 0;
}

/**
 * @brief 给定初值，初始化算法
 * 与产品相关参数，如灵敏度级别等由外部处理
 *
 * @param smk  SMK上下文指针
 * @param calibratedBGVal  标定背景值
 * @param calibratedFireThrd  标定火警门限值
 * @param revisedBGVal  补偿后背景值
 */
void SMK_Init(SMK *smk, uint8_t calibratedBGVal, uint8_t calibratedFireThrd, uint8_t revisedBGVal)
{
    // smk结构清零
    uint8_t *p = (uint8_t *)smk;
    for (int i = 0; i < sizeof(SMK); i++)
        *p++ = 0;

    smk->calibratedBGVal = calibratedBGVal;
    smk->calibratedFireThrd = calibratedFireThrd;

    uint8_t calibratedIncrement = calibratedFireThrd - calibratedBGVal; // 标定报警增量

    // 初始背景值不能超过补偿极限
    smk->bgVal = revisedBGVal;
    HIGHLIMIT(smk->bgVal, SMK_BACKGROUND_REVISED_HIGHLIMIT);
    LOWLIMIT(smk->bgVal, calibratedBGVal);
    LOWLIMIT(smk->bgVal, SMK_BACKGROUND_REVISED_LOWLIMIT);

    // 初始报警门限为 初始背景值+标定增量
    // 例外情况: 初始报警门限不超过0xE0, 超过时触发 TooMuchDusty机制
    uint16_t fireThrd = smk->bgVal + calibratedIncrement; // 使用uint16_t避免溢出
    smk->fireThrd = MIN(fireThrd, SMK_FIRE_REVISED_LIMIT);
    smk->bgTracker = smk->bgVal;

    // 初始化为上电状态
    smk->status = SMK_STATUS_POWERON;

    // 初始污染状态判断
    if (SMK_IsPolluted(smk))
        smk->extStatus |= SMK_EXT_STATUS_POLLUTED;
}

/**
 * @brief SMK_Input
 *
 * @param smk SMK上下文指针, 需先用SMK_Init初始化
 * @param adVal  AD采样值，输入激励
 * @return SMK_STATUS  当前SMK状态机状态
 */
SMK_STATUS SMK_Input(SMK *smk, uint16_t adVal)
{
    // 先使用原始AD采样值滤波，以保持精度
    SMK_Filter(smk, adVal);
    // TODO: 是否需要判断状态？POWERON和SMK_STATUS_SENSOR_ERR下是否也追踪？原A30一直追踪
    SMK_UpdateBGTracker(smk);
    // 再将AD采样值降精度为8位，便于比较、计算
    adVal = adVal >> (ADC_BIT-8);

    // 有限状态机
    uint8_t nextState = smk->status;
    switch (smk->status)
    {
    case SMK_STATUS_POWERON:
        smk->powerOnCnt++;
        if (smk->powerOnCnt > SMK_POWERON_SAMPLE_CNT)
        {
            if (smk->bgVal > smk->filterVal)
                smk->bgVal = MAX(smk->calibratedBGVal, smk->filterVal);

            // 上电更新背景值前不能判断污染报火警，否则背景值大于B0H清洗迷宫后上电就报火警
            // 简化算法起见，即使传感器故障也先进入SMK_STATUS_NORMAL状态再判断
            nextState = SMK_STATUS_NORMAL;
        }
        break;
    case SMK_STATUS_NORMAL:
        if (SMK_IsSensorErr(smk, adVal))
        {
            nextState = SMK_STATUS_SENSOR_ERR;
        }
        else if (SMK_IsFireAlarm(smk, adVal))
        {
            smk->fireRecoveryCnt = 0;
            nextState = SMK_STATUS_FIRE;
        }
        break;
    case SMK_STATUS_FIRE:
        if (SMK_IsSensorErr(smk, adVal))
        {
            nextState = SMK_STATUS_SENSOR_ERR;
        }
        else if (SMK_IsFireRecovery(smk, adVal))
        {
            smk->fireAlarmCnt = 0;
            nextState = SMK_STATUS_NORMAL;
        }
        break;
    case SMK_STATUS_SENSOR_ERR:
        if (SMK_IsSensorRecovery(smk, adVal))
        {
            smk->sensorErrCnt = 0;
            nextState = SMK_STATUS_NORMAL;
        }
        break;
    default:
        // 不应执行到此处
        break;
    }

    smk->status = nextState;
    return smk->status;
}

/**
 * @brief 背景补偿
 * 由外部根据补偿时间定时调用
 *
 * @param smk SMK上下文指针
 * @return uint8_t 补偿后的背景值
 */
uint8_t SMK_StaticRevise(SMK *smk)
{
    // 上电态、火警态、故障态，不补偿
    if (smk->status != SMK_STATUS_NORMAL)
        return smk->bgVal;

    if (smk->bgVal < smk->bgTracker) //正补偿
    {
        //超上限不补偿
        if (smk->bgVal < SMK_BACKGROUND_REVISED_HIGHLIMIT)
        {
            smk->bgVal++;
            smk->fireThrd++;
            // 污染处理
            HIGHLIMIT(smk->fireThrd, SMK_FIRE_REVISED_LIMIT)
        }
    }
    else if (smk->bgVal > smk->bgTracker) //负补偿
    {
        // 超下限不补偿
        // 下补偿不能小于calibratedBGVal
        if (smk->bgVal > SMK_BACKGROUND_REVISED_LOWLIMIT + 1 && smk->bgVal > smk->calibratedBGVal)
        {
            smk->bgVal--;
            // 污染处理处理
            if (smk->fireThrd - smk->bgVal > smk->calibratedFireThrd - smk->calibratedBGVal)
                smk->fireThrd--;
        }
    }
    // else 不补偿 (smk->bgVal == smk->bgTracker)

    if (SMK_IsPolluted(smk))
        smk->extStatus |= SMK_EXT_STATUS_POLLUTED;
    else
        smk->extStatus &= ~SMK_EXT_STATUS_POLLUTED;

    return smk->bgVal;
}

/***********************************************************************************************************/
// Helper Functions

/**
 * @brief 获得归一化的探测值
 *
 * @param smk SMK上下文指针
 * @return uint8_t 归一化值
 */
uint8_t SMK_GetDetectorValue(SMK *smk)
{
    // 将滤波值按bgVal=0x10, fireThrd=0x30作线性归一化
    // 公式: y = (y1-y0)/(x1-x0) * (x-x0) + y0
    // x0 = bgVal, x1=fireThrd, y0=0x10, y1=0x30, x=filterVal
    // 最小值修正为0x10, 最大值修正为0x3f
    int y = (smk->filterVal - smk->bgVal) * (0x30 - 0x10) / (smk->fireThrd - smk->bgVal) + 0x10;
    y = y < 0x10 ? 0x10 : y;
    y = y > 0x3f ? 0x3f : y;
    return (uint8_t)y;
}
