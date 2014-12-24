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

		uks_channels.drying_channel_list[i].number=i+1;

		for(j=0;j<TEMPERATURE_QUEUE_LEN;j++)
		{
			uks_channels.drying_channel_list[i].temperature_queue[j]=200.0;
		}
	}

	uks_channels.screen=SCREEN_CHANNELS_FIRST;
	uks_channels.device_error=ERROR_NONE;

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
				  if (uks_chnl->drying_channel_sort_list[j]->number > uks_chnl->drying_channel_sort_list[j + 1]->number)
				  {
					  SWAP( uks_chnl->drying_channel_sort_list[j], uks_chnl->drying_channel_sort_list[j + 1] );
				  }
			}
		}
	}
}

#define MIN_DELTA_TEMP		4.0
#define MIN_TRESHOLD_TEMP	45.0
#define TEMP_DRYING_END		50.0
#define START_TEMP_UP_TIME	10
#define AVERAGE_TEMP_TIME	10

#define MIN_DELTA_FALLING_TEMP	-2.0

uint8_t UKS_Channel_State_Drying(struct drying_channel *drying_chnl)
{
	switch(drying_chnl->drying_state)
	{
		case DRYING_WAIT_NEW_OPERATION:
		{
			float delta_temp=drying_chnl->temperature_queue[drying_chnl->temperature_queue_counter]-drying_chnl->temperature_queue[(drying_chnl->temperature_queue_counter-START_TEMP_UP_TIME)&(TEMPERATURE_QUEUE_LEN-1)];
			if((delta_temp>MIN_DELTA_TEMP)&&(drying_chnl->temperature>MIN_TRESHOLD_TEMP))
			{
				drying_chnl->drying_state=DRYING_CONTINUE;
			}
		}
		break;

		case DRYING_CONTINUE:
		{
		    float average_temperature=0.0, summ_temperature=0.0;
			uint8_t current_queue_position=drying_chnl->temperature_queue_counter;
			uint8_t i=(current_queue_position-AVERAGE_TEMP_TIME)&(TEMPERATURE_QUEUE_LEN-1);

			while((i&(TEMPERATURE_QUEUE_LEN-1))!=current_queue_position)
			{
				summ_temperature+=(drying_chnl->temperature_queue[i&(TEMPERATURE_QUEUE_LEN-1)]);
				i++;
			}

			average_temperature=summ_temperature/AVERAGE_TEMP_TIME;

			if(average_temperature>TEMP_DRYING_END)
			{
				drying_chnl->drying_state=DRYING_DONE;
				uks_channels.screen=SCREEN_CHANNELS_FIRST;
				Buzzer_Set_Buzz(BUZZER_EFFECT_0,BUZZER_ON);
			}

			float delta_temp=drying_chnl->temperature_queue[drying_chnl->temperature_queue_counter]-drying_chnl->temperature_queue[(drying_chnl->temperature_queue_counter-START_TEMP_UP_TIME)&(TEMPERATURE_QUEUE_LEN-1)];
			if(delta_temp<(MIN_DELTA_FALLING_TEMP))
			{
				drying_chnl->drying_state=DRYING_WAIT_NEW_OPERATION;
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
