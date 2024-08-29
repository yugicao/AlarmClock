/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c_lcd.h"
#include "DS3231.h"
#include "button.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum	
{
	NORMAL_STATE,
	SETTING_HOUR_STATE,
	SETTING_MIN_STATE,
	SETTING_SEC_STATE,
	SETTING_DATE_STATE,
	SETTING_MON_STATE,
	SETTING_YEAR_STATE,
	ALARM_HOUR_STATE,
	ALARM_MIN_STATE,
	BACKLIGHT_OFF_STATE
}ClockState;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// back light off time (hour: 0-23)
#define BACKLIGHT_OFF_START 20
#define BACKLIGHT_OFF_END 5

#define BLINK_SETTING_DURATION 300
#define BACKLIGHT_ON_DURATION 10000
#define TIME_ADJUST_HOLD_DURATION 1000
#define TIME_ADJUST_STEP_INTERVAL 300
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
I2C_LCD lcd;
DateTime date_time;
Button_Typedef btn_ok, btn_up, btn_down;
//int8_t alarm_hour, alarm_min;
uint32_t time_on_click_up = 0, time_on_click_down = 0, time_on_click_ok = 0;
const char arr_day[][4]= {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
ClockState clock_state = NORMAL_STATE;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void clock_time_update(void);
void up_down_number(int8_t *number, int8_t max, int8_t min, uint8_t isPlus);
uint8_t check_leap_year(uint16_t year);
uint8_t get_max_date(uint8_t mon, uint16_t year);
void check_alarm(void);
void setting_blink(void);
void adjust_time(uint8_t isPlus);
void button_time_out_handler_cb(Button_Typedef *ButtonX);
void button_short_press_handler_cb(Button_Typedef *ButtonX);
void button_press_handler_cb(Button_Typedef *ButtonX);
void button_release_handler_cb(Button_Typedef *ButtonX);
void hold_btn_handle(void);
void backlight_handle(void);
void alarm_screen(void);
void clock_handle(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	HAL_Delay(100);
	lcd_init(&lcd, &hi2c1, LCD_ADDR_DEFAULT);
	rtc_init(&hi2c1);
  button_init(&btn_down, GPIOA, GPIO_PIN_0);
  button_init(&btn_ok, GPIOA, GPIO_PIN_1);
  button_init(&btn_up, GPIOA, GPIO_PIN_2);
	rtc_read_alarm(&date_time);
	lcd_clear(&lcd);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		clock_handle();
		button_handler(&btn_down);
		button_handler(&btn_ok);
		button_handler(&btn_up);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

  /*Configure GPIO pins : PA0 PA1 PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

//-----------------Clock handle---------------------------
void clock_handle(void)
{
	switch(clock_state)
	{
		case NORMAL_STATE:
			clock_time_update();
			check_alarm();
			backlight_handle();
			break;
		case SETTING_HOUR_STATE:
		case SETTING_MIN_STATE:
		case SETTING_SEC_STATE:
		case SETTING_DATE_STATE:
		case SETTING_MON_STATE:
		case SETTING_YEAR_STATE:
			setting_blink();
			hold_btn_handle();
			break;
		case ALARM_HOUR_STATE:
		case ALARM_MIN_STATE:
			alarm_screen();
			hold_btn_handle();
			break;
		case BACKLIGHT_OFF_STATE:
			clock_time_update();
			check_alarm();
			backlight_handle();
			break;
		default: break;
	}
}

void clock_time_update(void)
{
	static uint32_t t_update = 0;
	if(HAL_GetTick() - t_update >= 1000)
	{
		rtc_read_time(&date_time);
		lcd_put_cur(&lcd, 0,4);
		lcd_send_string(&lcd, "%02d:%02d:%02d", date_time.hour, date_time.min, date_time.sec);
		lcd_put_cur(&lcd, 1,1);
		lcd_send_string(&lcd, "%s %02d/%02d/20%02d", arr_day[rtc_read_time_of_week(&date_time)], 
										date_time.date, date_time.mon, date_time.year);
		
		t_update = HAL_GetTick();
	}
}

//-----------------Alarm---------------------------------
void check_alarm(void)
{
	if(date_time.alarm_hour == date_time.hour && date_time.alarm_min == date_time.min)
	{
		static uint32_t t_alarm = 0;
		if(HAL_GetTick() - t_alarm >= 1000)
		{
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);
			t_alarm = HAL_GetTick();
		}
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	}
}

//-----------------Back light check---------------------
void backlight_handle(void)
{
	switch(clock_state)
	{
		case NORMAL_STATE:
		{
			if(date_time.hour >= BACKLIGHT_OFF_START || date_time.hour < BACKLIGHT_OFF_END)
			{
				static uint32_t t_backlight = 0;
				if(t_backlight == 0)
				{
					t_backlight = HAL_GetTick();
				}
				else if(HAL_GetTick() - t_backlight >= BACKLIGHT_ON_DURATION)
				{
					t_backlight = 0;
					lcd_back_light_off(&lcd);
					clock_state = BACKLIGHT_OFF_STATE;
				}
			}
			break;
		}
		case BACKLIGHT_OFF_STATE:
			if(date_time.hour < BACKLIGHT_OFF_START && date_time.hour >= BACKLIGHT_OFF_END)
			{
				lcd_back_light_on(&lcd);
				clock_state = NORMAL_STATE;
			}
			break;		
		default: break;
	}	

}

//-----------------Alarm screen---------------------------
void alarm_screen(void)
{
	static uint32_t t_blink = 0;
	static uint8_t isShow = 1;
	char line2[16];
	
	if(HAL_GetTick() - t_blink >= BLINK_SETTING_DURATION)
	{
		rtc_read_time(&date_time);
		isShow = !isShow;
		sprintf(line2, "%02d:%02d", date_time.alarm_hour, date_time.alarm_min);
		if(isShow)
		{
			lcd_put_cur(&lcd, 1,5);
			lcd_send_string(&lcd, "%s", line2);
		}
		else
		{
			switch(clock_state)
			{
				case ALARM_HOUR_STATE:
					line2[0] = line2[1] = ' ';
					break;
				case ALARM_MIN_STATE:
					line2[3] = line2[4] = ' ';
					break;
				default: break;
			}
			lcd_put_cur(&lcd, 0,5);
			lcd_send_string(&lcd,"ALARM");
			lcd_put_cur(&lcd, 1,5);
			lcd_send_string(&lcd, "%s", line2);
		}
		t_blink = HAL_GetTick();
	}
}

//-----------------Setting blink-------------------------
void setting_blink(void)
{
	static uint32_t t_blink = 0;
	static uint8_t isShow = 1;
	char line1[16];
	char line2[16];
	
	if(HAL_GetTick() - t_blink >= BLINK_SETTING_DURATION)
	{
		rtc_read_time(&date_time);
		isShow = !isShow;
		sprintf(line1, "%02d:%02d:%02d", date_time.hour, date_time.min, date_time.sec);
		sprintf(line2, "%s %02d/%02d/20%02d", arr_day[rtc_read_time_of_week(&date_time)], 
										date_time.date, date_time.mon, date_time.year);
		if(isShow)
		{
			lcd_put_cur(&lcd, 0,4);
			lcd_send_string(&lcd, "%s", line1);
			lcd_put_cur(&lcd, 1,1);
			lcd_send_string(&lcd, "%s", line2);
		}
		else
		{
			switch(clock_state)
			{
				case SETTING_HOUR_STATE:
					line1[0] = line1[1] = ' ';
					break;
				case SETTING_MIN_STATE:
					line1[3] = line1[4] = ' ';
					break;
				case SETTING_SEC_STATE:
					line1[6] = line1[7] = ' ';
					break;
				case SETTING_DATE_STATE:
					line2[4] = line2[5] = ' ';
					break;
				case SETTING_MON_STATE:
					line2[7] = line2[8] = ' ';
					break;
				case SETTING_YEAR_STATE:
					line2[10] = line2[11] = line2[12] = line2[13] = ' ';
					break;
				default: break;
			}
			lcd_put_cur(&lcd, 0,4);
			lcd_send_string(&lcd, "%s", line1);
			lcd_put_cur(&lcd, 1,1);
			lcd_send_string(&lcd, "%s", line2);
		}
		t_blink = HAL_GetTick();
	}
}
//-----------------Adjust time---------------------------
void adjust_time(uint8_t isPlus)
{
	switch(clock_state)
	{
		case SETTING_HOUR_STATE:
			up_down_number(&date_time.hour, 23, 0, isPlus);
			rtc_write_time(&date_time);
			break;
		case SETTING_MIN_STATE:
			up_down_number(&date_time.min, 59, 0, isPlus);
			rtc_write_time(&date_time);
			break;
		case SETTING_SEC_STATE:
			up_down_number(&date_time.sec, 59, 0, isPlus);
			rtc_write_time(&date_time);
			break;
		case SETTING_DATE_STATE:
			up_down_number(&date_time.date, 
										get_max_date(date_time.mon, date_time.year), 1, isPlus);
			rtc_write_time(&date_time);
			rtc_write_time(&date_time);
			break;
		case SETTING_MON_STATE:
		{
			up_down_number(&date_time.mon, 12, 1, isPlus);
			uint8_t max_date = get_max_date(date_time.mon, date_time.year);
			if(date_time.date > max_date)
			{
				date_time.date = max_date;
			}
			rtc_write_time(&date_time);
			break;
		}
		case SETTING_YEAR_STATE:
			up_down_number(&date_time.year, 99, 0, isPlus);
			rtc_write_time(&date_time);
			break;
		case ALARM_HOUR_STATE:
			up_down_number(&date_time.alarm_hour, 23, 0, isPlus);
			rtc_write_alarm(&date_time);
			break;
		case ALARM_MIN_STATE:
			up_down_number(&date_time.alarm_min, 59, 0, isPlus);
			rtc_write_alarm(&date_time);
			break;
		default: break;
	}
}

//-----------------Callback button-----------------------
void button_time_out_handler_cb(Button_Typedef *ButtonX)
{
	if(ButtonX == &btn_ok && clock_state == NORMAL_STATE)
	{
		clock_state = ALARM_HOUR_STATE;
		lcd_clear(&lcd);
		lcd_put_cur(&lcd, 0, 5);
		lcd_send_string(&lcd, "ALARM");
	}
}

void button_short_press_handler_cb(Button_Typedef *ButtonX)
{
	if(ButtonX == &btn_ok)
	{
		switch(clock_state)
		{
			case NORMAL_STATE:
				clock_state = SETTING_HOUR_STATE;
				break;
			case SETTING_HOUR_STATE:
				clock_state = SETTING_MIN_STATE;
				break;
			case SETTING_MIN_STATE:
				clock_state = SETTING_SEC_STATE;
				break;
			case SETTING_SEC_STATE:
				clock_state = SETTING_DATE_STATE;
				break;
			case SETTING_DATE_STATE:
				clock_state = SETTING_MON_STATE;
				break;
			case SETTING_MON_STATE:
				clock_state = SETTING_YEAR_STATE;
				break;
			case SETTING_YEAR_STATE:
				clock_state = NORMAL_STATE;
				break;
			case ALARM_HOUR_STATE:
				clock_state = ALARM_MIN_STATE;
				break;
			case ALARM_MIN_STATE:
				clock_state = NORMAL_STATE;
				break;
			default: break;
		}	
	}
	if(clock_state == BACKLIGHT_OFF_STATE)
	{		
			lcd_back_light_on(&lcd);
			clock_state = NORMAL_STATE;
	}
}

void button_press_handler_cb(Button_Typedef *ButtonX)
{
	if(ButtonX == &btn_up)
	{
		adjust_time(1);
		time_on_click_up = HAL_GetTick();
		time_on_click_ok = HAL_GetTick();
	}
	else if(ButtonX == &btn_down)
	{
		adjust_time(0);
		time_on_click_down = HAL_GetTick();
		time_on_click_ok = HAL_GetTick();
	}
}

void button_release_handler_cb(Button_Typedef *ButtonX)
{
	if(ButtonX == &btn_up)
	{
		time_on_click_up = 0;
		time_on_click_ok = HAL_GetTick();
	}
	else if(ButtonX == &btn_down)
	{
		time_on_click_down = 0;
		time_on_click_ok = HAL_GetTick();
	}
	else if(ButtonX == &btn_ok)
	{
		time_on_click_ok = HAL_GetTick();
	}
}
//-----------------Hold button handle---------------------
void hold_btn_handle(void)
{
	// If the button is held longer than TIME_ADJUST_STEP_INTERVAL, automatically adjust depend on TIME_ADJUST_STEP_INTERVAL
	if(time_on_click_up && HAL_GetTick() - time_on_click_up > TIME_ADJUST_HOLD_DURATION)
	{
		static uint32_t t_adjust_time = 0;
		if(HAL_GetTick() - t_adjust_time > TIME_ADJUST_STEP_INTERVAL)
		{
			adjust_time(1);
			t_adjust_time = HAL_GetTick();
		}
	}
	if(time_on_click_down && HAL_GetTick() - time_on_click_down > TIME_ADJUST_HOLD_DURATION)
	{
		static uint32_t t_adjust_time = 0;
		if(HAL_GetTick() - t_adjust_time > TIME_ADJUST_STEP_INTERVAL)
		{
			adjust_time(0);
			t_adjust_time = HAL_GetTick();
		}
	}
	
	// Resets the screen timeout counter to keep the screen active while the ok button is being held down
	if(time_on_click_ok && HAL_GetTick() - time_on_click_ok > BACKLIGHT_ON_DURATION) 
	{
		time_on_click_ok = 0;
		clock_state = NORMAL_STATE;
	}
}

//-----------------Utils---------------------------------
void up_down_number(int8_t *number, int8_t max, int8_t min, uint8_t isPlus)
{
	if(isPlus)
	{
		(*number)++;
		if(*number > max)
		{
			*number = min;
		}
	}
	else
	{
		(*number)--;
		if(*number < min)
		{
			*number = max;
		}
	}
}
uint8_t check_leap_year(uint16_t year)
{
	if(((year %4 == 0) && (year%100 != 0)) || (year%400 == 0))
	{
		return 1;
	}
	return 0;
}
uint8_t get_max_date(uint8_t mon, uint16_t year)
{
	switch(mon)
	{
		case 1:case 3:case 5:case 7: case 8:case 10: case 12:
			return 31;
		case 4:case 6:case 9:case 11:
			return 30;
		case 2:
			if(check_leap_year(year))
			{
				return 29;
			}
	}
	return 28;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
