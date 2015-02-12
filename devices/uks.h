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

#define END_DRYING_TEMP_MIN		  0.0
#define END_DRYING_TEMP_MAX		200.0

#define HEATER_TEMP_MIN			  0.0
#define HEATER_TEMP_MAX			200.0

#define P_FACTOR_MIN			  0.0
#define P_FACTOR_MAX		  10000.0

#define I_FACTOR_MIN		   -100.0
#define I_FACTOR_MAX		    100.0

#define D_FACTOR_MIN		   -10000.0
#define D_FACTOR_MAX		    10000.0

#define DELTA_TEMP_START_DRYING_MIN		 0.0
#define DELTA_TEMP_START_DRYING_MAX 	10.0

#define DELTA_TEMP_CANCEL_DRYING_MIN   -10.0
#define DELTA_TEMP_CANCEL_DRYING_MAX    -1.0

#define TRESHOLD_TEMP_START_DRYING_MIN	40.0
#define TRESHOLD_TEMP_START_DRYING_MAX  100.0

#define HEATER_INIT_TIMEOUT_MIN			10
#define HEATER_INIT_TIMEOUT_MAX			10000

#define MEASURING_FRAME_TIME_MIN		5
#define MEASURING_FRAME_TIME_MAX		30


#define END_DRYING_TEMP_DEFAULT	 		    105.0
#define HEATER_TEMP_DEFAULT		 		    105.0
#define P_FACTOR_DEFAULT		 		    9.0
#define I_FACTOR_DEFAULT		   			0.009
#define D_FACTOR_DEFAULT		   			1500.0
#define DELTA_TEMP_START_DRYING_DEFAULT		4.0
#define DELTA_TEMP_CANCEL_DRYING_DEFAULT   -2.0
#define TRESHOLD_TEMP_START_DRYING_DEFAULT	45.0
#define HEATER_INIT_TIMEOUT_DEFAULT			1200
#define MEASURING_FRAME_TIME_DEFAULT		10



struct uks_parameters
{
	float end_drying_temperature[DRYING_CHANNELS_NUM];

	float heater_temperature_1;
	float heater_temperature_2;

	float p_factor;//PID regulator
	float i_factor;
	float d_factor;

	float delta_temp_start_drying;
	float treshold_temp_start_drying;
	float delta_temp_cancel_drying;

	uint16_t heater_init_timeout;
	uint16_t measuring_frame_time;
};

struct uks
{
	struct drying_channel   drying_channel_list[DRYING_CHANNELS_NUM];
	struct drying_channel * drying_channel_sort_list[DRYING_CHANNELS_NUM];
	float 		heater_temperature;
	float		heater_temperature_current_setting;
	uint32_t 	heater_code;
	uint16_t    power_value;
	uint8_t 	screen;
	uint8_t 	device_error;
	uint8_t		heater_tempereature_tumblr;

	struct uks_parameters uks_params;
	struct uks_parameters *backup_uks_params;
};

void UKS_Drying_Init(void);
void UKS_Restore_Settings(void);

#endif
