#include "phaze_detector.h"

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include <misc.h>

//»нклуды от FreeRTOS:

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

struct phaze_detector phaze_detect;

xSemaphoreHandle xPhazeSemaphore;

static void Heater_Control_Task(void *pvParameters);

void Phaze_Detector_Init(void)
{
	RCC_AHB1PeriphClockCmd(ZERO_CROSS_PORT_RCC, ENABLE);//тактируем портј
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitTypeDef  GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = ZERO_CROSS|CONTR_SP;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(ZERO_CROSS_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = RELAY_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(RELAY_PORT, &GPIO_InitStructure);

	EXTI_InitTypeDef EXTI_InitStructure;

	SYSCFG_EXTILineConfig(ZERO_CROSS_PORT_EXTI, ZERO_CROSS_PinSource);
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	SYSCFG_EXTILineConfig(ZERO_CROSS_PORT_EXTI, CONTR_SP_PinSource);
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);


	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 14;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 14;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);

	phaze_detect.contr_sp_counter=0;
	phaze_detect.input_sp_counter=0;
	phaze_detect.zero_cross_counter=0;

	vSemaphoreCreateBinary( xPhazeSemaphore );
	Set_Heater_Power(5);

	 xTaskCreate(Heater_Control_Task,(signed char*)"Heater Control",128,NULL, tskIDLE_PRIORITY + 1, NULL);

}

void Set_Heater_Power(uint8_t power)
{
	if(power<=MAX_POWER_VALUE)
	{
		EXTI->IMR &= ~EXTI_Line0;
		phaze_detect.power_value=power;
	}	EXTI->IMR |=  EXTI_Line0;
}

static volatile uint8_t cross_counter=0;

void EXTI0_IRQHandler(void)
{
 	static portBASE_TYPE xHigherPriorityTaskWoken;
 	xHigherPriorityTaskWoken = pdFALSE;

    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line0);
        phaze_detect.zero_cross_counter++;

        if(cross_counter<phaze_detect.power_value)
        {
        	RELAY_PORT->BSRRL|=RELAY_PIN;//pin up
        	phaze_detect.input_sp_counter++;
        }
        else
        {
        	RELAY_PORT->BSRRH|=RELAY_PIN;//pin down
        	if(cross_counter>=MAX_POWER_VALUE)
        	{
        		cross_counter=0;
				xSemaphoreGiveFromISR( xPhazeSemaphore, &xHigherPriorityTaskWoken );

				 if( xHigherPriorityTaskWoken != pdFALSE )
				 {
					portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
				 }
        	}
        }
        cross_counter++;
    }
}

void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line1);
        phaze_detect.contr_sp_counter++;
    }
}

static void Heater_Control_Task(void *pvParameters)
{
	while(1)
	{
		vTaskDelay(300);
	}
}
