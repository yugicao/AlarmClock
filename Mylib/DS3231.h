#ifndef __DS3231_H
#define __DS3231_H
#include "main.h"
typedef struct
{
	int8_t hour;
	int8_t min;
	int8_t sec;
	int8_t day;
	int8_t date;
	int8_t mon;
	int8_t year;
	int8_t alarm_hour;
	int8_t alarm_min;
}DateTime;

void rtc_write_time(DateTime *dt);

void rtc_read_time(DateTime *dt);

float rtc_read_temp(void);

void rtc_init(I2C_HandleTypeDef *hi2c);

uint8_t rtc_read_time_of_week(DateTime *dt);

void rtc_write_alarm(DateTime *dt);

void rtc_read_alarm(DateTime *dt);

#endif
