#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_spi.h"

#include <misc.h>

#include "keyboard.h"
//»нклуды от FreeRTOS:

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "buzzer.h"
#include "uks.h"

#include "watchdog.h"

extern struct uks uks_channels;

static void vKeyboardTask(void *pvParameters);

extern struct task_watch task_watches[];

void Keyboard_Init(void)
{
	RCC_AHB1PeriphClockCmd(INIT_KEYB_PORT, ENABLE);
	GPIO_InitTypeDef init_pin;

	init_pin.GPIO_Pin  = KEY_0 | KEY_1;//подт€гиваем вверх, дл€ уменьшени€ помех
	init_pin.GPIO_Speed = GPIO_Speed_2MHz;
	init_pin.GPIO_Mode  = GPIO_Mode_IN;
	init_pin.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init (KEYB_PORT, &init_pin);

	xTaskCreate(vKeyboardTask,(signed char*)"Keyboard",64,NULL, tskIDLE_PRIORITY + 1, NULL);
}

#define PRESS_SHORT_TIME	50

static void vKeyboardTask(void *pvParameters)
{
	volatile static uint16_t press_time_counter=0;
	task_watches[KEYBOARD_TASK].task_status=TASK_ACTIVE;
    while(1)
    {
    	if(GPIO_ReadInputDataBit(KEYB_PORT,KEY_0)==Bit_RESET)
    	{
    		vTaskDelay(10);
    		if(GPIO_ReadInputDataBit(KEYB_PORT,KEY_0)==Bit_RESET)
    		{
    			uint8_t i=0;

    			Buzzer_Set_Buzz(BUZZER_EFFECT_0,BUZZER_OFF);
    			//uks_channels.screen=SCREEN_CHANNELS_FIRST;
    			for(i=0;i<DRYING_CHANNELS_NUM;i++)
    			{
    				if(uks_channels.drying_channel_list[i].drying_state==DRYING_DONE)
    				{
    					uks_channels.drying_channel_list[i].drying_state=DRYING_WAIT_NEW_OPERATION;
    				}
    			}

    			switch(uks_channels.screen)
				{
					case SCREEN_CHANNELS_FIRST:
					{

					}
					break;

					case SCREEN_INIT_HEATER:
					{

					}
					break;

					case SCREEN_CHANNELS_SECOND:
					{
						//uks_channels.screen=SCREEN_CHANNELS_FIRST;
					}
					break;

					case SCREEN_HEATER:
					{
						//uks_channels.screen=SCREEN_CHANNELS_FIRST;
					}
					break;

					case SCREEN_ERROR:
					{

					}
					break;

					case SCREEN_HEATER_INIT_TIMEOUT:
					{

					}
					break;

					default:
					{
						uks_channels.screen=SCREEN_CHANNELS_FIRST;
					}
				}
    		}
    	}

    	if(GPIO_ReadInputDataBit(KEYB_PORT,KEY_1)==Bit_RESET)
    	{
    		vTaskDelay(10);
    		if(GPIO_ReadInputDataBit(KEYB_PORT,KEY_1)==Bit_RESET)
    		{
    			press_time_counter++;

    			if(press_time_counter>PRESS_SHORT_TIME)
    			{
    				//uks_channels.screen=SCREEN_HEATER;

    				switch(uks_channels.screen)
    				{
    					case SCREEN_CHANNELS_FIRST:
    					{
    						uks_channels.screen=SCREEN_HEATER;
    					}
    					break;

    					case SCREEN_INIT_HEATER:
    					{

    					}
    					break;

    					case SCREEN_CHANNELS_SECOND:
    					{
    						uks_channels.screen=SCREEN_HEATER;
    					}
    					break;

    					case SCREEN_HEATER:
    					{

    					}
    					break;

    					case SCREEN_ERROR:
    					{

    					}
    					break;

    					case SCREEN_HEATER_INIT_TIMEOUT:
    					{

    					}
    					break;

    					default:
    					{
    						uks_channels.screen=SCREEN_CHANNELS_FIRST;
    					}
    				}
    			}
    		}
    	}
    	else
    	{
    		if((press_time_counter>0)&&(press_time_counter<=PRESS_SHORT_TIME))
    		{
				switch(uks_channels.screen)
				{
					case SCREEN_CHANNELS_FIRST:
					{
						uks_channels.screen=SCREEN_CHANNELS_SECOND;
					}
					break;

					case SCREEN_INIT_HEATER:
					{

					}
					break;

					case SCREEN_CHANNELS_SECOND:
					{
						uks_channels.screen=SCREEN_CHANNELS_FIRST;
					}
					break;

					case SCREEN_HEATER:
					{
						uks_channels.screen=SCREEN_CHANNELS_FIRST;
					}
					break;

					case SCREEN_ERROR:
					{

					}
					break;

					case SCREEN_HEATER_INIT_TIMEOUT:
					{

					}
					break;

					default:
					{
						uks_channels.screen=SCREEN_CHANNELS_FIRST;
					}
				}
    		}
    		press_time_counter=0;
    	}
    	task_watches[KEYBOARD_TASK].counter++;
    }
}
