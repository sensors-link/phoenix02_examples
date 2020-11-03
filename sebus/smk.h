#ifndef _SMK_H_
#define _SMK_H_

#include <stdint.h>

typedef enum
{
	SMK_STATUS_POWERON = 0,
	SMK_STATUS_NORMAL,
	SMK_STATUS_FIRE,
	SMK_STATUS_SENSOR_ERR
} SMK_STATUS;

typedef enum
{
	SMK_EXT_STATUS_FAST_SAMPLE = 1,
	SMK_EXT_STATUS_POLLUTED = 2
} SMK_EXT_STATUS;

typedef struct
{
	// 配置参数
	uint8_t calibratedBGVal;	//标定背景值backGroundValue
	uint8_t calibratedFireThrd; //标定火警值FireAlarmValue

	// 运行参数
	uint16_t filterVal16; // 当前滤波值(扩展精度)
	uint8_t filterVal;	  // 当前滤波值
	uint8_t fireThrd;	  // 当前火警值
	uint8_t bgVal;		  // 当前背景值
	uint8_t bgTracker;	  // BackGroundTemp1算法参数,跟随每次AD采样结果的变化，BackGroundTemp1 += BackGroundTemp1>filterVal?-1:1

	uint8_t status;	   // 当前基本状态
	uint8_t extStatus; // 快速采样、污染等扩展状态

	uint8_t exitFastSampleCnt; // 快速采样次数
	uint8_t reserved1;		   // 保留，对齐

	uint8_t powerOnCnt;		 // 上电采样计数次数
	uint8_t sensorErrCnt;	 // 连续传感器故障计数
	uint8_t fireAlarmCnt;	 // 连续满足火警条件次数
	uint8_t fireRecoveryCnt; // 连续满足火警恢复条件次数

} SMK;

#define SMK_FIRE_REVISED_LIMIT 0xE0			  //火警补偿极限值（保险绳）0xE0  TooMuchDusty
#define SMK_BACKGROUND_REVISED_HIGHLIMIT 0xB0 // 静态补偿上限值0xB0, 对应_Max_BackGround_Compensation
#define SMK_BACKGROUND_REVISED_LOWLIMIT 0x10  // 静态补偿下限值0x10
#define SMK_SENSORERR_RESETVALUE 0x0F		  // 传感器故障恢复门限
#define SMK_SENSORERR_VALUE 0x0A			  // 传感器故障门限

#define SMK_FAST_SAMPLE_THRESHOLD 0x10 // 进入和退出快速采样模式的采样值与滤波值差值门限

#define SMK_EXIT_FASTSAMPLE_CNT 10 // 退出快速采样模式的连续满足条件采样计数
#define SMK_POWERON_SAMPLE_CNT 20  // 上电状态采样次数
#define SMK_FIREALARM_CNT 6		   // 连续滤波值大于门限火警判断次数
#define SMK_FIRERECOVERY_CNT 10	   // 连续滤波值小于门限火警恢复判断次数
#define SMK_SENSORERR_CNT 10	   // 连续滤波值小于门限传感器故障判断次数

// 模块导出的接口函数
extern void SMK_Init(SMK *smk, uint8_t calibratedBGVal, uint8_t calibratedFireThrd, uint8_t revisedBGVal);
extern SMK_STATUS SMK_Input(SMK *smk, uint16_t adVal);
extern uint8_t SMK_StaticRevise(SMK *smk);
extern uint8_t SMK_GetDetectorValue(SMK *smk);

#endif /* SMK_H_ */