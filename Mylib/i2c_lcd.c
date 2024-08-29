
/** Put this in the src folder **/

#include "i2c_lcd.h"
#include <stdarg.h>
#include <stdio.h>

void lcd_send_cmd (I2C_LCD *lcd, char cmd)
{
	char data_u, data_l;
	uint8_t data_t[4];
	uint8_t backlight = lcd->backlight ? 0x08 : 0x00;

	data_u = (cmd & 0xf0);
	data_l = ((cmd << 4) & 0xf0);

	data_t[0] = data_u | 0x04 | backlight; // en=1, rs=0
	data_t[1] = data_u | backlight;        // en=0, rs=0
	data_t[2] = data_l | 0x04 | backlight; // en=1, rs=0
	data_t[3] = data_l | backlight;        // en=0, rs=0

	HAL_I2C_Master_Transmit(lcd->pI2C, lcd->adr, (uint8_t *) data_t, 4, 100);
}
void lcd_send_data (I2C_LCD *lcd, char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	uint8_t backlight = lcd->backlight ? 0x08 : 0x00;

	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);

	data_t[0] = data_u | 0x05 | backlight; // en=1, rs=1
	data_t[1] = data_u | 0x01 | backlight; // en=0, rs=1
	data_t[2] = data_l | 0x05 | backlight; // en=1, rs=1
	data_t[3] = data_l | 0x01 | backlight; // en=0, rs=1

	HAL_I2C_Master_Transmit (lcd->pI2C, lcd->adr, (uint8_t *) data_t, 4, 100);
}

void lcd_clear (I2C_LCD *lcd)
{
	lcd_send_cmd (lcd, 0x80);
	for (int i=0; i<70; i++)
	{
		lcd_send_data (lcd, ' ');
	}
}

void lcd_put_cur(I2C_LCD *lcd, int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }
    lcd_send_cmd (lcd, col);
}


void lcd_init (I2C_LCD *lcd, I2C_HandleTypeDef* hi2c1, uint8_t lcd_adr)
{
	lcd->pI2C = hi2c1;
	lcd->adr = lcd_adr;
	lcd->backlight = BACKLIGHT_ON;
	// 4 bit initialisation
	HAL_Delay(50);  // wait for >40ms
	lcd_send_cmd (lcd, 0x30);
	HAL_Delay(5);  // wait for >4.1ms
	lcd_send_cmd (lcd, 0x30);
	HAL_Delay(1);  // wait for >100us
	lcd_send_cmd (lcd, 0x30);
	HAL_Delay(10);
	lcd_send_cmd (lcd, 0x20);  // 4bit mode
	HAL_Delay(10);

  // dislay initialisation
	lcd_send_cmd (lcd, 0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	lcd_send_cmd (lcd, 0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	HAL_Delay(1);
	lcd_send_cmd (lcd, 0x01);  // clear display
	HAL_Delay(1);
	HAL_Delay(1);
	lcd_send_cmd (lcd, 0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	lcd_send_cmd (lcd, 0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}

void lcd_send_string (I2C_LCD *lcd, const char *str, ...)
{
	va_list args;
	char str_send[17];
	va_start(args, str);
	vsnprintf(str_send, 17, str, args);
	va_end(args);
  for(uint8_t i=0; str_send[i]; ++i)
	{
		lcd_send_data (lcd, str_send[i]);
	}
}

//back light

void lcd_back_light_on(I2C_LCD *lcd)
{
	uint8_t data = 0x08;
	HAL_I2C_Master_Transmit(lcd->pI2C, lcd->adr, (uint8_t*)&data, 1, 100);
	lcd->backlight = BACKLIGHT_ON;
}
void lcd_back_light_off(I2C_LCD *lcd)
{
	uint8_t data = 0x00;
	HAL_I2C_Master_Transmit(lcd->pI2C, lcd->adr, (uint8_t*)&data, 1, 100);
	lcd->backlight = BACKLIGHT_OFF;
}

