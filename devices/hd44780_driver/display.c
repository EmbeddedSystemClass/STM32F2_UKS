#include "display.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "watchdog.h"
#include "uks.h"
#include "hd44780.h"
#include "ADS1120.h"
#include "stdio.h"

extern struct ADS1120_result ADS1120_res;

void Display_Task(void *pvParameters );

void Display_Init(void)
{
	HD44780_Init(20,4);
	xTaskCreate(Display_Task,(signed char*)"DISPLAY_TASK",128,NULL, tskIDLE_PRIORITY + 1, NULL);
   // task_watches[DISPLAY_TASK].task_status=TASK_IDLE;
}

void Display_Task(void *pvParameters )
{
	static volatile uint8_t str_buf[32];
	while(1)
	{
		vTaskDelay(300);

		sprintf(str_buf,"VAL=%09d",ADS1120_res.result);
		HD44780_Puts(0,0,str_buf);
//
		float temp=PT100_Code_To_Temperature(ADS1120_res.result);
		if(temp>=0)
		{
			sprintf(str_buf,"TEMP= %03d.%01d",(uint16_t)temp,(uint16_t)(temp*10)%10);
		}
		else
		{
			sprintf(str_buf,"TEMP=-%03d.%01d",(uint16_t)(-temp),(uint16_t)(((-temp)*10))%10);
		}
		HD44780_Puts(0,1,str_buf);
	}
}
