#include  "uks.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stdio.h"

struct uks uks_channels;
extern xSemaphoreHandle xMeasure_LM35_Semaphore;

void UKS_Drying_Task(void *pvParameters );

void UKS_Drying_Init(void)
{
	uint8_t i=0;

	for(i=0;i<DRYING_CHANNELS_NUM;i++)
	{
		uks_channels.drying_channel_list[i].drying_state=DRYING_WAIT_NEW_OPERATION;
		uks_channels.drying_channel_list[i].time=0;
		uks_channels.drying_channel_list[i].temperature=0.0;
		uks_channels.drying_channel_list[i].time_forecast=0;
	}

	xTaskCreate(UKS_Drying_Task,(signed char*)"DUKS_DRYING_TASK",128,NULL, tskIDLE_PRIORITY + 1, NULL);
   // task_watches[DRYING_TASK].task_status=TASK_IDLE;
}

void UKS_Drying_Task(void *pvParameters )
{
	uint8_t i=0;
	while(1)
	{
		if( xSemaphoreTake( xMeasure_LM35_Semaphore, ( portTickType ) portMAX_DELAY ) == pdTRUE )
		{
			for(i=0;i<DRYING_CHANNELS_NUM;i++)
			{

			}
		}
	}
}

#define SWAP(A, B) { struct uks *t = A; A = B; B = t; }
void UKS_Sort_Channels(struct uks * uks_chnl,uint8_t num)
{
	  uint16_t i, j;

	  for(i=0;i<num;i++)
	  {
		 // uks_channels.drying_channel_list[i].temperature=uks_channels.drying_channel_list[i].temperature_queue[uks_channels.drying_channel_list[i].temperature_queue_counter];
		  uks_channels.drying_channel_sort_list[i]=&uks_channels.drying_channel_list[i];
	  }

	  for (i = num - 1; i > 0; i--)
	  {
			for (j = 0; j < i; j++)
			{
				  if (uks_channels.drying_channel_sort_list[j]->temperature > uks_channels.drying_channel_sort_list[j + 1]->temperature)
				  {
					  SWAP( uks_channels.drying_channel_sort_list[j], uks_channels.drying_channel_sort_list[j + 1] );
				  }
			}
	  }
}

#define MIN_DELTA_TEMP	5.0

uint8_t UKS_Channel_State_Drying(struct drying_channel *drying_chnl)
{
	switch(drying_chnl->drying_state)
	{
		case DRYING_WAIT_NEW_OPERATION:
		{
			float delta_temp=drying_chnl->temperature_queue[drying_chnl->temperature_queue_counter]-drying_chnl->temperature_queue[(drying_chnl->temperature_queue_counter-10)&(TEMPERATURE_QUEUE_LEN-1)];

		}
		break;

		case DRYING_CONTINUE:
		{

		}
		break;

		case DRYING_DONE:
		{

		}
		break;

		default:
		{

		}
	}
}
