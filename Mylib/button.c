#include "button.h"

__weak void button_time_out_handler_cb(Button_Typedef *ButtonX)
{
}
__weak void button_short_press_handler_cb(Button_Typedef *ButtonX)
{
}
__weak void button_press_handler_cb(Button_Typedef *ButtonX)
{
}
__weak void button_release_handler_cb(Button_Typedef *ButtonX)
{
}
void button_handler(Button_Typedef *ButtonX)
{ 
	uint8_t sta_button = HAL_GPIO_ReadPin(ButtonX->GPIOx, ButtonX->GPIO_Pin);
	if( sta_button != ButtonX->filter_sta_button)
	{
		ButtonX->filter_sta_button = sta_button;
		ButtonX->is_deboucing = 1;
		ButtonX->preTick = HAL_GetTick();
	}
	if((HAL_GetTick() - ButtonX->preTick >= 15) && ButtonX->is_deboucing)
	{
		ButtonX->current_sta_button = ButtonX->filter_sta_button;
		ButtonX->is_deboucing = 0;
	}
	
	if(ButtonX->current_sta_button != ButtonX->pre_sta_button)
	{
		if(ButtonX->current_sta_button == 0)
		{
			ButtonX->pressTick = HAL_GetTick();
			button_press_handler_cb(ButtonX);
		}
		else
		{
			if(HAL_GetTick() - ButtonX->pressTick <= 1000)
				button_short_press_handler_cb(ButtonX);
			
			button_release_handler_cb(ButtonX);
			if(HAL_GetTick() - ButtonX->pressTick >= 3000)
				button_time_out_handler_cb(ButtonX);
		}
		ButtonX->pre_sta_button = ButtonX->current_sta_button;
	}
}
void button_init(Button_Typedef *ButtonX, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	ButtonX->current_sta_button = 1;
	ButtonX->filter_sta_button = 1;
	ButtonX->pre_sta_button = 1;
	ButtonX->GPIOx = GPIOx;
	ButtonX->GPIO_Pin = GPIO_Pin;
}
