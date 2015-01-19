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
extern struct uks uks_channels;
extern struct task_watch task_watches[];

void Display_Task(void *pvParameters );

void Display_Init(void)
{
	HD44780_Init(HD44780_WIDTH,HD44780_HEIGHT);
	xTaskCreate(Display_Task,(signed char*)"DISPLAY_TASK",128,NULL, tskIDLE_PRIORITY + 1, NULL);
   // task_watches[DISPLAY_TASK].task_status=TASK_IDLE;
}

static volatile uint16_t display_init_counter=0;
#define DISPLAY_INIT_PERIOD		3

uint8_t drying_states[3][16]={{/*"������� "*/"\xA8po\xE5\x65\x63\x63  "},{/*"��������     "*/"Oko\xBD\xC0\x65\xBDo "},{/*"��������     "}*/"C\xB3o\xB2o\xE3\xBDo "}};

void Display_Task(void *pvParameters )
{
	static volatile uint8_t str_buf[32];
	task_watches[DISPLAY_TASK].task_status=TASK_ACTIVE;
	while(1)
	{
		vTaskDelay(300);
		if(display_init_counter>=DISPLAY_INIT_PERIOD)
		{
			HD44780_Reinit(HD44780_WIDTH,HD44780_HEIGHT);
			display_init_counter=0;
		}
		else
		{
			display_init_counter++;
		}

		UKS_Sort_Channels(&uks_channels,DRYING_CHANNELS_NUM);

		uint8_t i=0;

		switch(uks_channels.screen)
		{
			case SCREEN_INIT_HEATER:
			{
				float temp=uks_channels.heater_temperature;
				if(temp>=0)
				{
					sprintf(str_buf,"Heater= %3d.%01d         ",(uint16_t)temp,(uint16_t)(temp*10)%10);
				}
				else
				{
					sprintf(str_buf,"Heater=-%3d.%01d         ",(uint16_t)(-temp),(uint16_t)(((-temp)*10))%10);
				}


				HD44780_Puts(0,0,str_buf);
				sprintf(str_buf,"   �po�ecc �a�pe�a  ");/*  ������� �������  */
				HD44780_Puts(0,1,str_buf);
				sprintf(str_buf,"�o�a�y�c�a,�o�o�㸿e");/*����������,���������*/
				HD44780_Puts(0,2,str_buf);
				sprintf(str_buf,"                    ");
				HD44780_Puts(0,3,str_buf);
			}
			break;

			case SCREEN_HEATER_INIT_TIMEOUT:
			{
				sprintf(str_buf,"       O����a       ");/*       ������       */
				HD44780_Puts(0,0,str_buf);
				sprintf(str_buf,"    Bpe�� �a�pe�a   ");//    ����� �������
				HD44780_Puts(0,1,str_buf);
				sprintf(str_buf,"      �pe���e�o     ");//     ���������
				HD44780_Puts(0,2,str_buf);
				sprintf(str_buf,"                    ");
				HD44780_Puts(0,3,str_buf);
			}
			break;

			case SCREEN_CHANNELS_FIRST:
			{
				for(i=0;i<HD44780_HEIGHT;i++)
				{
					float temp=uks_channels.drying_channel_sort_list[i]->temperature;
					if(temp<0.0)
					{
						temp=0.0;
					}
					sprintf(str_buf,"%1d %3d.%01d \xEF\x43 %s",(uks_channels.drying_channel_sort_list[i]->number+1),(uint16_t)temp,(uint16_t)(temp*10)%10,&drying_states[uks_channels.drying_channel_sort_list[i]->drying_state]);
					HD44780_Puts(0,i,str_buf);
				}
			}
			break;

			case SCREEN_CHANNELS_SECOND:
			{
				for(i=HD44780_HEIGHT;i<DRYING_CHANNELS_NUM;i++)
				{
					float temp=uks_channels.drying_channel_sort_list[i]->temperature;
					if(temp<0.0)
					{
						temp=0.0;
					}
					sprintf(str_buf,"%1d %3d.%01d \xEF\x43 %s",(uks_channels.drying_channel_sort_list[i]->number+1),(uint16_t)temp,(uint16_t)(temp*10)%10,&drying_states[uks_channels.drying_channel_sort_list[i]->drying_state]);
					HD44780_Puts(0,(i-HD44780_HEIGHT),str_buf);
				}

				for(i=DRYING_CHANNELS_NUM-HD44780_HEIGHT;i<HD44780_HEIGHT;i++)
				{
					sprintf(str_buf,"                    ");
					HD44780_Puts(0,i,str_buf);
				}

			}
			break;

			case SCREEN_HEATER:
			{
				float temp=uks_channels.heater_temperature;
				if(temp>=0)
				{
					sprintf(str_buf,"Ha�pe�a�e��= %3d.%01d         ",(uint16_t)temp,(uint16_t)(temp*10)%10);
				}
				else
				{
					sprintf(str_buf,"Ha�pe�a�e��=-%3d.%01d         ",(uint16_t)(-temp),(uint16_t)(((-temp)*10))%10);
				}


				HD44780_Puts(0,0,str_buf);
				sprintf(str_buf,"                    ");
				HD44780_Puts(0,1,str_buf);
				HD44780_Puts(0,2,str_buf);
				HD44780_Puts(0,3,str_buf);
			}
			break;

			case SCREEN_ERROR:
			{
				switch(uks_channels.device_error)
				{
					case ERROR_PHAZE:
					{
						sprintf(str_buf,"       O����a       ");/*       ������       */
						HD44780_Puts(0,0,str_buf);
						sprintf(str_buf,"        �a�a        ");//        ����
						HD44780_Puts(0,1,str_buf);
						sprintf(str_buf,"    o�cy�c��ye�     ");//    �����������
						HD44780_Puts(0,2,str_buf);
						sprintf(str_buf,"                    ");
						HD44780_Puts(0,3,str_buf);
					}
					break;

					case ERROR_HEATER_RELAY:
					{
						sprintf(str_buf,"       O����a       ");/*       ������       */
						HD44780_Puts(0,0,str_buf);
						sprintf(str_buf,"Ha�pe�a�e�� ��� pe�e");//����������� ��� ����
						HD44780_Puts(0,1,str_buf);
						sprintf(str_buf,"     �e�c�pa���     ");//     ����������
						HD44780_Puts(0,2,str_buf);
						sprintf(str_buf,"                    ");
						HD44780_Puts(0,3,str_buf);
					}
					break;

					case ERROR_RELAY:
					{
						sprintf(str_buf,"       O����a       ");/*       ������       */
						HD44780_Puts(0,0,str_buf);
						sprintf(str_buf,"        Pe�e        " );//        ����
						HD44780_Puts(0,1,str_buf);
						sprintf(str_buf,"     �e�c�pa��o     ");//     ����������
						HD44780_Puts(0,2,str_buf);
						sprintf(str_buf,"                    ");
						HD44780_Puts(0,3,str_buf);
					}
					break;

					default:
					{
						uks_channels.screen=SCREEN_CHANNELS_FIRST;
					}
				}
			}
			break;

			default:
			{
				uks_channels.screen=SCREEN_CHANNELS_FIRST;
			}
		}
		task_watches[DISPLAY_TASK].counter++;
	}
}
