


#ifndef __PRD_EVT_H
#define __PRD_EVT_H


#ifdef _DEBUG
//周期时间计数值定义
#define ADC_SMP_CNT             10                 //时间基准100ms,如10*100 = 1s采样一次
#define COMPENSATION_CNT        60*10//60*60*10           //时间基准100ms,如60**60*10*100 = 1h进行补偿处理一次
#define RECORD_DAY_CNT          2*60*10//24*60*60*10        //一天记录一次数据
#define RAM_PARAM_CHK_CNT       30//60*10              //一分钟检测一次
#else
//周期时间计数值定义
#define ADC_SMP_CNT             10                 //时间基准100ms,如10*100 = 1s采样一次
#define COMPENSATION_CNT        60*60*10           //时间基准100ms,如60**60*10*100 = 1h进行补偿处理一次
#define RECORD_DAY_CNT          24*60*60*10        //一天记录一次数据
#define RAM_PARAM_CHK_CNT       60*10              //一分钟检测一次
#endif


#define EVENT_100MS_NUM    4       //定义100毫秒为基数的事件数目

typedef struct Event100MS
{
    unsigned long ulEventCount[EVENT_100MS_NUM];              //事件计数值
    unsigned long ulPreiodInterval[EVENT_100MS_NUM];          //事件处理函数参数
    void (*pvEventHandler[EVENT_100MS_NUM])(void *pvParam);   //事件处理函数
    void *pvParam;
}Event100MS_t;

//周期事件
enum _EVENT_100MS_TYPE{
    EVENT_ADC_SMP = 0,              //AD采样事件
    EVENT_COMPENSATION,             //补偿事件
    EVENT_RECORD_DAY,               //记录天事件
    EVENT_RAM_CHK,               //参数检测事件

};




//事件变量定义
extern Event100MS_t gEvent100MS;


void PeriodEventInit(void);
void PeriodEventHandler(void);




#endif

