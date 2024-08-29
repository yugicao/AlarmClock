#ifndef __I2C_LCD_H
#define __I2C_LCD_H
#include "main.h"

#define LCD_ADDR_DEFAULT 0x27 << 1
typedef enum
{
	BACKLIGHT_OFF,
	BACKLIGHT_ON
}backlight_state_t;// 1:on, 0:off
typedef struct
{
	uint8_t adr;
	I2C_HandleTypeDef *pI2C;
	backlight_state_t backlight; // 1:on, 0:off
}I2C_LCD;


void lcd_init (I2C_LCD* lcd, I2C_HandleTypeDef* hi2c1, uint8_t lcd_adr);

void lcd_send_cmd (I2C_LCD *lcd, char cmd);  // send command to the lcd

void lcd_send_data (I2C_LCD *lcd, char data);  // send data to the lcd

void lcd_send_string (I2C_LCD *lcd, const char *str, ...);

void lcd_put_cur(I2C_LCD *lcd, int row, int col);  // put cursor at the entered position row (0 or 1), col (0-15);

void lcd_clear (I2C_LCD *lcd);

void lcd_back_light_on(I2C_LCD *lcd);
	
void lcd_back_light_off(I2C_LCD *lcd);

#endif
