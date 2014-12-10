#ifndef 	UKS_H
#define  	UKS_H

#include "stm32f4xx.h"

//struct calibrate
//{
//	uint16_t adc_code;
//	uint32_t value;
//};
//
//struct backup_registers
//{
//	struct calibrate thermo_resistor;
//};
//

#define UKS_DRYING_EMP_END	120.0

#define DISPALY_TEXT_STRING_LEN	20
#define TEMPERATURE_QUEUE_LEN	32
enum
{
	DRYING_CONTINUE=0,
	DRYING_DONE
};

struct drying_channel
{
	float temperature_queue[TEMPERATURE_QUEUE_LEN];
	uint8_t temperature_queue_counter;
	float temperature;//средн€€ температура за 30 сек
	uint16_t time;//minute
	uint16_t time_forecast;
	uint8_t text_string[DISPALY_TEXT_STRING_LEN];
	uint8_t drying_state;
};

#define DRYING_CHANNELS_NUM	7
struct uks
{
	struct drying_channel   drying_channel_list[DRYING_CHANNELS_NUM];
	struct drying_channel * drying_channel_sort_list[DRYING_CHANNELS_NUM];
};

#endif
