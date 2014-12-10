#include  "uks.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stdio.h"

struct uks uks_channels;

void UKS_Drying_Task(void *pvParameters );

void UKS_Drying_Init(void)
{
	uint8_t i=0;

	for(i=0;i<DRYING_CHANNELS_NUM;i++)
	{
		uks_channels.drying_channel_list[i].drying_state=DRYING_DONE;
		uks_channels.drying_channel_list[i].time=0;
		uks_channels.drying_channel_list[i].temperature=0.0;
		uks_channels.drying_channel_list[i].time_forecast=0;
	}

	xTaskCreate(UKS_Drying_Task,(signed char*)"DUKS_DRYING_TASK",128,NULL, tskIDLE_PRIORITY + 1, NULL);
   // task_watches[DRYING_TASK].task_status=TASK_IDLE;
}

void UKS_Drying_Task(void *pvParameters )
{
	while(1)
	{

	}
}

#define SWAP(A, B) { struct uks *t = A; A = B; B = t; }
void UKS_Sort_Channels(struct uks * uks_chnl,uint8_t num)
{
	  uint16_t i, j;

	  for(i=0;i<num;i++)
	  {
		  uks_channels.drying_channel_list[i].temperature=uks_channels.drying_channel_list[i].temperature_queue[uks_channels.drying_channel_list[i].temperature_queue_counter];
		  uks_channels.drying_channel_sort_list[i]=&uks_channels.drying_channel_list[i];
	  }

	  for (i = num - 1; i > 0; i--)
	  {
	    for (j = 0; j < i; j++)
	    {
	      if (uks_channels.drying_channel_sort_list[j]->temperature > uks_channels.drying_channel_sort_list[j + 1]->temperature)
	        SWAP( uks_channels.drying_channel_sort_list[j], uks_channels.drying_channel_sort_list[j + 1] );
	    }
	  }
}
