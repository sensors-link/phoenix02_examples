/**
 * @file main.c
 * @author bifei.tang (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2020-06-01
 *
 * @copyright Fanhai Data Tech. (c) 2020
 *
 */

#include "lib_include.h"


void RTC_IrqHandler(void){
	_DBG_("rtc alarm int");

    RTC->STS = RTC_STS_ALMF;
};


int main(void)
{
	debug_frmwrk_init();
	_DBG_("rtc example");
    RTC_Init(RTC_XTL,RTC_FMT_24H);
    RTC_SetDate(20,6,1,1);
    RTC_SetTime(15,9,0);
    RTC_SetAlarm(RTC_ALM_EN_WEEK1 | RTC_ALM_EN_WEEK2,15,10);
    RTC_AlarmCofig(ENABLE,ENABLE);
    RTC->STS = RTC_STS_ALMF;
    PLIC_EnableIRQ(RTC_IRQn);
    PLIC_SetPriority(RTC_IRQn,1);
    int hour = RTC_GetHour();
    int minute = RTC_GetMinute();
    int sec = RTC_GetSecond();
    while(1)
    {
        int secTmp = RTC_GetSecond();
        if(sec != secTmp)
        {
        	sec = secTmp;
        	int year = RTC_GetYear();
            int month = RTC_GetMonth();
            int day = RTC_GetDay();
            int week = RTC_GetWeek();
            int hour = RTC_GetHour();
            int minute = RTC_GetMinute();
            int sec = RTC_GetSecond();
            _DBD16(year);_DBG("/");
            _DBD(month);_DBG("/");
            _DBD(day);_DBG("/");
            _DBD(week);_DBG("\r\n");

            _DBD(hour);_DBG("/");
            _DBD(minute);_DBG("/");
            _DBD(sec);_DBG("\r\n");
        }
    }


}
