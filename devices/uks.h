#ifndef 	UKS_H
#define  	UKS_H

#include "stm32f4xx.h"


#define UKS_DRYING_EMP_END	120.0

#define DISPALY_TEXT_STRING_LEN	20
#define TEMPERATURE_QUEUE_LEN	32
enum
{
	DRYING_CONTINUE=0,
	DRYING_DONE,
	DRYING_WAIT_NEW_OPERATION
};

enum
{
	SCREEN_INIT_HEATER=0,
	SCREEN_CHANNELS_FIRST,
	SCREEN_CHANNELS_SECOND,
	SCREEN_HEATER,
	SCREEN_ERROR,
	SCREEN_HEATER_INIT_TIMEOUT,
};

enum
{
	ERROR_NONE=0,
	ERROR_HEATER_RELAY,
	ERROR_RELAY,
	ERROR_PHAZE,
	ERROR_HEATER_TIMEOUT
};

struct drying_channel
{
	uint8_t 	number;
	float 		temperature_queue[TEMPERATURE_QUEUE_LEN];
	uint8_t 	temperature_queue_counter;
	uint16_t 	code;
	float 	 	temperature;//current temperature
	uint16_t 	time;//minute
	uint16_t 	time_forecast;
	uint8_t 	drying_state;
	float 		drying_end_temperature;
};

#define DRYING_CHANNELS_NUM	7

struct uks_parameters
{
	float end_drying_temperature[DRYING_CHANNELS_NUM];

	float heater_temperature_1;
	float heater_temperature_2;

	float p_factor;//PID regulator
	float i_factor;
	float d_factor;
};

struct uks
{
	struct drying_channel   drying_channel_list[DRYING_CHANNELS_NUM];
	struct drying_channel * drying_channel_sort_list[DRYING_CHANNELS_NUM];
	float 		heater_temperature;
	uint32_t 	heater_code;
	uint8_t 	screen;
	uint8_t 	device_error;
	uint8_t		heater_tempereature_tumblr;

	struct uks_parameters uks_params;
	struct uks_parameters *backup_uks_params;
};

void UKS_Drying_Init(void);
void UKS_Restore_Settings(void);

#endif
