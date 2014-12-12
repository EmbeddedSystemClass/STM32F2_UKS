#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

#include <misc.h>
#include "system_stm32f4xx.h"

//FreeRTOS:

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


//#include "menu.h"
#include "ADS1120.h"

#include "keyboard.h"
#include "buzzer.h"
#include "watchdog.h"
#include "power_detector.h"
#include "phaze_detector.h"
#include "backup_sram.h"
//#include "hd44780.h"
#include "display.h"
#include "rtc.h"
#include "pid_regulator.h"
#include "uks.h"

#include "usbd_cdc_vcp.h" // подключаем USB CDC
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_dcd_int.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;
int main(void)
{
	SystemInit();

	Buzzer_Init();
	Power_Detector_Init();
////	Backup_SRAM_Init();
	Phaze_Detector_Init();
	//HD44780_Init(20,4);
	Display_Init();

	ADS1120_init();
	ADC_Channel_Init();
	PID_Heater_Init();
	UKS_Drying_Init();

	Buzzer_Init();
	Keyboard_Init();
	USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_CDC_cb,&USR_cb);
//	RTC_Clock_Init();
////	Watchdog_Init();


    vTaskStartScheduler();

    while(1);
}
//---------------------------------------------------------------------------------------

void vApplicationTickHook( void )
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
	}
}
