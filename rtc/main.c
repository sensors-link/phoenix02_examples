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

void RTC_IrqHandler(void) {
    printf("rtc alarm int\r\n");

    RTC->STS = RTC_STS_ALMF;
};

int main(void) {
    printf("rtc example\r\n");
    RTC_Init(RTC_XTL, RTC_FMT_24H);
    RTC_SetDate(20, 6, 1, 1);
    RTC_SetTime(15, 9, 0);
    RTC_SetAlarm(RTC_ALM_EN_WEEK1 | RTC_ALM_EN_WEEK2, 15, 10);
    RTC_AlarmCofig(ENABLE, ENABLE);
    RTC->STS = RTC_STS_ALMF;
    PLIC_EnableIRQ(RTC_IRQn);
    PLIC_SetPriority(RTC_IRQn, 1);
    int hour = RTC_GetHour();
    int minute = RTC_GetMinute();
    int sec = RTC_GetSecond();
    while (1) {
        int secTmp = RTC_GetSecond();
        if (sec != secTmp) {
            sec = secTmp;
            int year = RTC_GetYear();
            int month = RTC_GetMonth();
            int day = RTC_GetDay();
            int week = RTC_GetWeek();
            int hour = RTC_GetHour();
            int minute = RTC_GetMinute();
            int sec = RTC_GetSecond();
            printf("year:%d\r\n", year);
            printf("month:%d\r\n", month);
            printf("day:%d\r\n", day);
            printf("week:%d\r\n", week);
            printf("hour:%d\r\n", hour);
            printf("minute:%d\r\n", minute);
            printf("sec:%d\r\n", sec);
        }
    }
}
