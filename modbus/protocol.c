#include "protocol.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "mb.h"
#include "mbport.h"



#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4

static unsigned short     usRegInputStart = REG_INPUT_START;
static unsigned short     usRegInputBuf[REG_INPUT_NREGS]={1,2,3,4};
static void Modbus_Task(void *pvParameters);

void Protocol_Init(void)
{
	eMBErrorCode    eStatus;

	eStatus = eMBInit( MB_RTU, 0x0A, 0, 57600, 0 );
	xTaskCreate(Modbus_Task,(signed char*)"Modbus",256,NULL, tskIDLE_PRIORITY + 1, NULL);
}



static void Modbus_Task(void *pvParameters)
{
    portTickType    xLastWakeTime;

    eMBEnable();
    for( ;; )
    {
        eMBPoll();
        vTaskDelay(10);
    }
}
