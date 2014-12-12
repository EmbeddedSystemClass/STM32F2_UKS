#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_spi.h"

#include <misc.h>

#include "keyboard.h"
//������� �� FreeRTOS:

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "buzzer.h"
#include "uks.h"

//xQueueHandle xKeyQueue;//������� ����������
//xSemaphoreHandle xKeySemaphore;
extern struct uks uks_channels;

static void vKeyboardTask(void *pvParameters);

void Keyboard_Init(void)
{
	RCC_AHB1PeriphClockCmd(INIT_KEYB_PORT, ENABLE);
	GPIO_InitTypeDef init_pin;

	init_pin.GPIO_Pin  = KEY_0 | KEY_1;//����������� �����, ��� ���������� �����
	init_pin.GPIO_Speed = GPIO_Speed_2MHz;
	init_pin.GPIO_Mode  = GPIO_Mode_IN;
	init_pin.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init (KEYB_PORT, &init_pin);

	xTaskCreate(vKeyboardTask,(signed char*)"Keyboard",64,NULL, tskIDLE_PRIORITY + 1, NULL);
}

static void vKeyboardTask(void *pvParameters)
{
    while(1)
    {
    	if(GPIO_ReadInputDataBit(KEYB_PORT,KEY_0)==Bit_RESET)
    	{
    		vTaskDelay(10);
    		if(GPIO_ReadInputDataBit(KEYB_PORT,KEY_0)==Bit_RESET)
    		{
    			uint8_t i=0;

    			Buzzer_Set_Buzz(BUZZER_EFFECT_0,BUZZER_OFF);
    			for(i=0;i<DRYING_CHANNELS_NUM;i++)
    			{
    				if(uks_channels.drying_channel_list[i].drying_state==DRYING_DONE)
    				{
    					uks_channels.drying_channel_list[i].drying_state=DRYING_WAIT_NEW_OPERATION;
    				}
    			}
    		}
    	}

    	if(GPIO_ReadInputDataBit(KEYB_PORT,KEY_1)==Bit_RESET)
    	{
    		vTaskDelay(10);
    		if(GPIO_ReadInputDataBit(KEYB_PORT,KEY_1)==Bit_RESET)
    		{

    		}
    	}
    }
}
