#include  "uks.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stdio.h"
#include "string.h"
#include "buzzer.h"
#include "pid_regulator.h"

struct uks uks_channels;
extern xSemaphoreHandle xMeasure_LM35_Semaphore;

void UKS_Drying_Task(void *pvParameters );
void UKS_Heater_Init_Task(void *pvParameters );
uint8_t UKS_Channel_State_Drying(struct drying_channel *drying_chnl);
uint8_t CRC_Check( uint8_t  *Spool_pr,uint8_t Count_pr );

//#define MIN_DELTA_TEMP		4.0
//#define MIN_TRESHOLD_TEMP	45.0
//#define TEMP_DRYING_END		50.0
#define START_TEMP_UP_TIME	10
#define AVERAGE_TEMP_TIME	10
//
//#define MIN_DELTA_FALLING_TEMP	-2.0

void UKS_Drying_Init(void)
{
	uint8_t i=0,j=0;

	if(uks_channels.device_error!=ERROR_NONE)
	{
		return;
	}

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
	uks_channels.screen=SCREEN_INIT_HEATER;

  //uks_channels.device_error=ERROR_NONE;
	xTaskCreate(UKS_Heater_Init_Task,(signed char*)"UKS_HEATER_INIT_TASK",128,NULL, tskIDLE_PRIORITY + 1, NULL);
   // task_watches[DRYING_TASK].task_status=TASK_IDLE;
}

#define HEATER_INIT_TIMEOUT	40
void UKS_Heater_Init_Task(void *pvParameters )
{
	uint16_t heater_init_timeout_counter=0;
	float temp_tmp, delta_temp;

	if(uks_channels.heater_tempereature_tumblr==TUMBLR_TEMP_1)
	{
		temp_tmp=uks_channels.uks_params.heater_temperature_1;
	}
	else
	{
		temp_tmp=uks_channels.uks_params.heater_temperature_2;
	}

	delta_temp=uks_channels.heater_temperature-temp_tmp;

	if(delta_temp<0.0)
	{
		delta_temp=-delta_temp;
	}

	while(delta_temp>1.0)
	{

		delta_temp=uks_channels.heater_temperature-temp_tmp;

		if(delta_temp<0.0)
		{
			delta_temp=-delta_temp;
		}

		vTaskDelay(1000);
		//uks_channels.screen=SCREEN_INIT_HEATER;
		heater_init_timeout_counter++;

		if(heater_init_timeout_counter>=HEATER_INIT_TIMEOUT)
		{
			uks_channels.screen=SCREEN_HEATER_INIT_TIMEOUT;
			uks_channels.device_error=ERROR_HEATER_TIMEOUT;
			Buzzer_Set_Buzz(BUZZER_EFFECT_0,BUZZER_ON);
			Heater_Power_Down_Block();
			vTaskDelete( NULL );
		}
	}

	if(/*uks_channels.screen!=SCREEN_HEATER_INIT_TIMEOUT*/uks_channels.device_error==ERROR_NONE)
	{
		uks_channels.screen=SCREEN_CHANNELS_FIRST;
		xTaskCreate(UKS_Drying_Task,(signed char*)"UKS_DRYING_TASK",128,NULL, tskIDLE_PRIORITY + 1, NULL);
	}
	 vTaskDelete( NULL );
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



uint8_t UKS_Channel_State_Drying(struct drying_channel *drying_chnl)
{
	switch(drying_chnl->drying_state)
	{
		case DRYING_WAIT_NEW_OPERATION:
		{
			float delta_temp=drying_chnl->temperature_queue[drying_chnl->temperature_queue_counter]-drying_chnl->temperature_queue[(drying_chnl->temperature_queue_counter-START_TEMP_UP_TIME)&(TEMPERATURE_QUEUE_LEN-1)];
			if((delta_temp>uks_channels.uks_params.delta_temp_start_drying)&&(drying_chnl->temperature>uks_channels.uks_params.treshold_temp_start_drying))
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

			if(average_temperature>uks_channels.uks_params.end_drying_temperature[drying_chnl->number])
			{
				drying_chnl->drying_state=DRYING_DONE;
				uks_channels.screen=SCREEN_CHANNELS_FIRST;
				Buzzer_Set_Buzz(BUZZER_EFFECT_0,BUZZER_ON);
			}

			float delta_temp=drying_chnl->temperature_queue[drying_chnl->temperature_queue_counter]-drying_chnl->temperature_queue[(drying_chnl->temperature_queue_counter-START_TEMP_UP_TIME)&(TEMPERATURE_QUEUE_LEN-1)];
			if(delta_temp<(uks_channels.uks_params.delta_temp_cancel_drying))
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

void UKS_Restore_Settings(void)
{
	uint8_t i=0;

	uks_channels.backup_uks_params=(struct uks_parameters *) BKPSRAM_BASE;

	uint8_t *back_crc=(uint8_t*)(BKPSRAM_BASE+sizeof(struct uks_parameters));
	uint8_t true_crc=CRC_Check((uint8_t*)(uks_channels.backup_uks_params),sizeof(struct uks_parameters));

	if(*back_crc==true_crc)
	{
		for(i=0;i<DRYING_CHANNELS_NUM;i++)
		{
			if((uks_channels.backup_uks_params->end_drying_temperature[i]>=END_DRYING_TEMP_MIN)&&(uks_channels.backup_uks_params->end_drying_temperature[i]<=END_DRYING_TEMP_MAX))
			{
				uks_channels.uks_params.end_drying_temperature[i]=uks_channels.backup_uks_params->end_drying_temperature[i];
			}
			else
			{
				uks_channels.uks_params.end_drying_temperature[i]=END_DRYING_TEMP_DEFAULT;
				Backup_SRAM_Write_Reg(&(uks_channels.backup_uks_params->end_drying_temperature[i]),&(uks_channels.uks_params.end_drying_temperature[i]),sizeof(float));
			}
		}

		if((uks_channels.backup_uks_params->heater_temperature_1>=HEATER_TEMP_MIN)&&(uks_channels.backup_uks_params->heater_temperature_1<=HEATER_TEMP_MAX))
		{
			uks_channels.uks_params.heater_temperature_1=uks_channels.backup_uks_params->heater_temperature_1;
		}
		else
		{
			uks_channels.uks_params.heater_temperature_1=HEATER_TEMP_DEFAULT;
			Backup_SRAM_Write_Reg(&uks_channels.backup_uks_params->heater_temperature_1,&uks_channels.uks_params.heater_temperature_1,sizeof(float));
		}

		if((uks_channels.backup_uks_params->heater_temperature_2>=HEATER_TEMP_MIN)&&(uks_channels.backup_uks_params->heater_temperature_2<=HEATER_TEMP_MAX))
		{
			uks_channels.uks_params.heater_temperature_2=uks_channels.backup_uks_params->heater_temperature_2;
		}
		else
		{
			uks_channels.uks_params.heater_temperature_2=HEATER_TEMP_DEFAULT;
			Backup_SRAM_Write_Reg(&uks_channels.backup_uks_params->heater_temperature_2,&uks_channels.uks_params.heater_temperature_2,sizeof(float));
		}


		if((uks_channels.backup_uks_params->p_factor>=P_FACTOR_MIN)&&(uks_channels.backup_uks_params->p_factor<=P_FACTOR_MAX))
		{
			uks_channels.uks_params.p_factor=uks_channels.backup_uks_params->p_factor;
		}
		else
		{
			uks_channels.uks_params.p_factor=P_FACTOR_DEFAULT;
			Backup_SRAM_Write_Reg(&uks_channels.backup_uks_params->p_factor,&uks_channels.uks_params.p_factor,sizeof(float));
		}


		if((uks_channels.backup_uks_params->i_factor>=I_FACTOR_MIN)&&(uks_channels.backup_uks_params->i_factor<=I_FACTOR_MAX))
		{
			uks_channels.uks_params.i_factor=uks_channels.backup_uks_params->i_factor;
		}
		else
		{
			uks_channels.uks_params.i_factor=I_FACTOR_DEFAULT;
			Backup_SRAM_Write_Reg(&uks_channels.backup_uks_params->i_factor,&uks_channels.uks_params.i_factor,sizeof(float));
		}


		if((uks_channels.backup_uks_params->d_factor>=D_FACTOR_MIN)&&(uks_channels.backup_uks_params->d_factor<=D_FACTOR_MAX))
		{
			uks_channels.uks_params.d_factor=uks_channels.backup_uks_params->d_factor;
		}
		else
		{
			uks_channels.uks_params.d_factor=D_FACTOR_DEFAULT;
			Backup_SRAM_Write_Reg(&uks_channels.backup_uks_params->d_factor,&uks_channels.uks_params.d_factor,sizeof(float));
		}


		if((uks_channels.backup_uks_params->delta_temp_start_drying>=DELTA_TEMP_START_DRYING_MIN)&&(uks_channels.backup_uks_params->delta_temp_start_drying<=DELTA_TEMP_START_DRYING_MAX))
		{
			uks_channels.uks_params.delta_temp_start_drying=uks_channels.backup_uks_params->delta_temp_start_drying;
		}
		else
		{
			uks_channels.uks_params.delta_temp_start_drying=DELTA_TEMP_START_DRYING_DEFAULT;
			Backup_SRAM_Write_Reg(&uks_channels.backup_uks_params->delta_temp_start_drying,&uks_channels.uks_params.delta_temp_start_drying,sizeof(float));
		}


		if((uks_channels.backup_uks_params->delta_temp_cancel_drying>=DELTA_TEMP_CANCEL_DRYING_MIN)&&(uks_channels.backup_uks_params->delta_temp_cancel_drying<=DELTA_TEMP_CANCEL_DRYING_MAX))
		{
			uks_channels.uks_params.delta_temp_cancel_drying=uks_channels.backup_uks_params->delta_temp_cancel_drying;
		}
		else
		{
			uks_channels.uks_params.delta_temp_cancel_drying=DELTA_TEMP_CANCEL_DRYING_DEFAULT;
			Backup_SRAM_Write_Reg(&uks_channels.backup_uks_params->delta_temp_cancel_drying,&uks_channels.uks_params.delta_temp_cancel_drying,sizeof(float));
		}


		if((uks_channels.backup_uks_params->treshold_temp_start_drying>=TRESHOLD_TEMP_START_DRYING_MIN)&&(uks_channels.backup_uks_params->treshold_temp_start_drying<=TRESHOLD_TEMP_START_DRYING_MAX))
		{
			uks_channels.uks_params.treshold_temp_start_drying=uks_channels.backup_uks_params->treshold_temp_start_drying;
		}
		else
		{
			uks_channels.uks_params.treshold_temp_start_drying=TRESHOLD_TEMP_START_DRYING_DEFAULT;
			Backup_SRAM_Write_Reg(&uks_channels.backup_uks_params->treshold_temp_start_drying,&uks_channels.uks_params.treshold_temp_start_drying,sizeof(float));
		}
	}
	else
	{
		for(i=0;i<DRYING_CHANNELS_NUM;i++)
		{
			uks_channels.uks_params.end_drying_temperature[i]=END_DRYING_TEMP_DEFAULT;
		}

		uks_channels.uks_params.p_factor=P_FACTOR_DEFAULT;
		uks_channels.uks_params.i_factor=I_FACTOR_DEFAULT;
		uks_channels.uks_params.d_factor=D_FACTOR_DEFAULT;

		uks_channels.uks_params.heater_temperature_1=HEATER_TEMP_DEFAULT;
		uks_channels.uks_params.heater_temperature_2=HEATER_TEMP_DEFAULT;

		uks_channels.uks_params.delta_temp_start_drying=DELTA_TEMP_START_DRYING_DEFAULT;
		uks_channels.uks_params.delta_temp_cancel_drying=DELTA_TEMP_CANCEL_DRYING_DEFAULT;
		uks_channels.uks_params.treshold_temp_start_drying=TRESHOLD_TEMP_START_DRYING_DEFAULT;

	}
}


uint8_t  CRC_Check( uint8_t  *Spool_pr,uint8_t Count_pr )
{
	uint8_t crc_n = 0;
	uint8_t  *Spool;
	uint8_t  Count ;

	Spool=Spool_pr;
	Count=Count_pr;

  		while(Count!=0x0)
        {
	        crc_n = crc_n ^ (*Spool++);

	        crc_n = ((crc_n & 0x01) ? (uint8_t)0x80: (uint8_t)0x00) | (uint8_t)(crc_n >> 1);

	        if (crc_n & (uint8_t)0x80) crc_n = crc_n ^ (uint8_t)0x3C;
			Count--;
        }
    return crc_n;
}
