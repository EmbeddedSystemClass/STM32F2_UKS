#include  "uks.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stdio.h"
#include "buzzer.h"

struct uks uks_channels;
extern xSemaphoreHandle xMeasure_LM35_Semaphore;

void UKS_Drying_Task(void *pvParameters );
uint8_t UKS_Channel_State_Drying(struct drying_channel *drying_chnl);

void UKS_Drying_Init(void)
{
	uint8_t i=0,j=0;

	for(i=0;i<DRYING_CHANNELS_NUM;i++)
	{
		uks_channels.drying_channel_list[i].drying_state=DRYING_WAIT_NEW_OPERATION;
		uks_channels.drying_channel_list[i].time=0;
		uks_channels.drying_channel_list[i].temperature=0.0;
		uks_channels.drying_channel_list[i].time_forecast=0;

		uks_channels.drying_channel_list[i].number=i;

		for(j=0;j<TEMPERATURE_QUEUE_LEN;j++)
		{
			uks_channels.drying_channel_list[i].temperature_queue[j]=200.0;
		}
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
				UKS_Channel_State_Drying(&uks_channels.drying_channel_list[i]);
			}
		}
	}
}

#define SWAP(A, B) { struct uks *t = A; A = B; B = t; }
//void UKS_Sort_Channels(struct uks * uks_chnl,uint8_t num)
//{
//	  uint16_t i, j;
//
//	  for(i=0;i<num;i++)
//	  {
//		 // uks_channels.drying_channel_list[i].temperature=uks_channels.drying_channel_list[i].temperature_queue[uks_channels.drying_channel_list[i].temperature_queue_counter];
//		  uks_channels.drying_channel_sort_list[i]=&uks_channels.drying_channel_list[i];
//	  }
//
//	  for (i = num - 1; i > 0; i--)
//	  {
//			for (j = 0; j < i; j++)
//			{
//				  if (uks_channels.drying_channel_sort_list[j]->temperature > uks_channels.drying_channel_sort_list[j + 1]->temperature)
//				  {
//					  SWAP( uks_channels.drying_channel_sort_list[j], uks_channels.drying_channel_sort_list[j + 1] );
//				  }
//			}
//	  }
//}

void UKS_Sort_Channels(struct uks * uks_chnl,uint8_t num)
{
	uint8_t chnl_count=0, dry_done_count=0,dry_continue_count=0,dry_wait_count=0;
	uint8_t i,j;

	for(i=0;i<num;i++)
	{
		if(uks_chnl->drying_channel_list[i].drying_state==DRYING_DONE)
		{
			uks_chnl->drying_channel_sort_list[chnl_count]=&uks_chnl->drying_channel_list[i];
			chnl_count++;
			dry_done_count++;
		}
	}
	if(dry_done_count)
	{
		for (i = chnl_count - 1; i > 0; i--)
		{
			for (j = 0; j < i; j++)
			{
				  if (uks_chnl->drying_channel_sort_list[j]->number > uks_chnl->drying_channel_sort_list[j + 1]->number)
				  {
					  SWAP( uks_chnl->drying_channel_sort_list[j], uks_chnl->drying_channel_sort_list[j + 1] );
				  }
			}
		}
	}
//------------------------------------
	for(i=0;i<num;i++)
	{
		if(uks_chnl->drying_channel_list[i].drying_state==DRYING_CONTINUE)
		{
			uks_chnl->drying_channel_sort_list[chnl_count]=&uks_chnl->drying_channel_list[i];
			chnl_count++;
			dry_continue_count++;
		}
	}

	if(dry_continue_count)
	{
		for (i = (chnl_count - 1); i > (chnl_count-dry_continue_count); i--)
		{
			for (j = (chnl_count-dry_continue_count); j < i; j++)
			{
				  if (uks_chnl->drying_channel_sort_list[j]->temperature > uks_chnl->drying_channel_sort_list[j + 1]->temperature)
				  {
					  SWAP( uks_chnl->drying_channel_sort_list[j], uks_chnl->drying_channel_sort_list[j + 1] );
				  }
			}
		}
	}
//------------------------------------
	for(i=0;i<num;i++)
	{
		if(uks_chnl->drying_channel_list[i].drying_state==DRYING_WAIT_NEW_OPERATION)
		{
			uks_chnl->drying_channel_sort_list[chnl_count]=&uks_chnl->drying_channel_list[i];
			chnl_count++;
			dry_wait_count++;
		}
	}

	if(dry_wait_count)
	{
		for (i = (chnl_count - 1); i > (chnl_count-dry_wait_count); i--)
		{
			for (j = (chnl_count-dry_wait_count); j < i; j++)
			{
				  if (uks_chnl->drying_channel_sort_list[j]->temperature > uks_chnl->drying_channel_sort_list[j + 1]->temperature)
				  {
					  SWAP( uks_chnl->drying_channel_sort_list[j], uks_chnl->drying_channel_sort_list[j + 1] );
				  }
			}
		}
	}
}

#define MIN_DELTA_TEMP		10.0
#define MIN_TRESHOLD_TEMP	50.0
#define TEMP_DRYING_END		72.0

uint8_t UKS_Channel_State_Drying(struct drying_channel *drying_chnl)
{
	switch(drying_chnl->drying_state)
	{
		case DRYING_WAIT_NEW_OPERATION:
		{
			float delta_temp=drying_chnl->temperature_queue[drying_chnl->temperature_queue_counter]-drying_chnl->temperature_queue[(drying_chnl->temperature_queue_counter-10)&(TEMPERATURE_QUEUE_LEN-1)];
			if((delta_temp>MIN_DELTA_TEMP)&&(drying_chnl->temperature>MIN_TRESHOLD_TEMP))
			{
				drying_chnl->drying_state=DRYING_CONTINUE;
			}
		}
		break;

		case DRYING_CONTINUE:
		{
			if(drying_chnl->temperature>TEMP_DRYING_END)
			{
				drying_chnl->drying_state=DRYING_DONE;
				Buzzer_Set_Buzz(BUZZER_EFFECT_0,BUZZER_ON);
			}
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
