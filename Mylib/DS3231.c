
#include "DS3231.h"
#define RTC_ADDR (0x68<<1)
//method 1
//extern I2C_HandleTypeDef hi2c1;
//method 2
static I2C_HandleTypeDef *ds_i2c;

uint8_t Decimal2BCD(uint8_t num)
{
	return (num/10)<<4|(num%10);
}

uint8_t BCD2Decimal(uint8_t num)
{
	return (num>>4)*10+(num&0x0F);
}


void rtc_write_time(DateTime *dt)
{
	uint8_t data[8];
	data[0] = 0x00;
	data[1] = Decimal2BCD(dt->sec);
	data[2] = Decimal2BCD(dt->min);
	data[3] = Decimal2BCD(dt->hour);
	data[4] = Decimal2BCD(dt->day);
	data[5] = Decimal2BCD(dt->date);
	data[6] = Decimal2BCD(dt->mon);
	data[7] = Decimal2BCD(dt->year);

	HAL_I2C_Master_Transmit(ds_i2c, RTC_ADDR, data, 8, 100);
	
}

void rtc_read_time(DateTime *dt)
{
	uint8_t data[7];
	uint8_t add_reg = 0x00;
	HAL_I2C_Master_Transmit(ds_i2c, RTC_ADDR, &add_reg, 1, 100);
	HAL_I2C_Master_Receive(ds_i2c, RTC_ADDR, data, 7, 100);
	dt->sec = BCD2Decimal(data[0]);
	dt->min = BCD2Decimal(data[1]);
	dt->hour = BCD2Decimal(data[2]);
	dt->day = BCD2Decimal(data[3]);
	dt->date = BCD2Decimal(data[4]);
	dt->mon = BCD2Decimal(data[5]);
	dt->year = BCD2Decimal(data[6]);

}

void rtc_write_alarm(DateTime *dt) //min and hour
{
	uint8_t data[3];
	data[0] = 0x08;
	data[1] = Decimal2BCD(dt->alarm_min);
	data[2] = Decimal2BCD(dt->alarm_hour);

	HAL_I2C_Master_Transmit(ds_i2c, RTC_ADDR, data, 3, 100);
}

void rtc_read_alarm(DateTime *dt)
{
	uint8_t data[2];
	uint8_t add_reg = 0x08;
	HAL_I2C_Master_Transmit(ds_i2c, RTC_ADDR, &add_reg, 1, 100);
	HAL_I2C_Master_Receive(ds_i2c, RTC_ADDR, data, 2, 100);
	dt->alarm_min = BCD2Decimal(data[0]);
	dt->alarm_hour = BCD2Decimal(data[1]);
}

float rtc_read_temp()
{
	uint8_t data_temp[2];
	uint8_t add_reg = 0x11;
	HAL_I2C_Master_Transmit(ds_i2c, RTC_ADDR, &add_reg, 1, 100);
	HAL_I2C_Master_Receive(ds_i2c, RTC_ADDR, data_temp, 2, 100);
	return ( data_temp[0] + ((data_temp[1] >> 6 ) * 0.25) );
}

uint8_t rtc_read_time_of_week(DateTime *dt)
{
	int d    = dt->date 		  ;
	int m    = dt->mon    		;
	int y    = dt->year + 2000;

	return (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7;
}

void rtc_init(I2C_HandleTypeDef *hi2c)
{
	ds_i2c = hi2c;
}
