#ifndef ADC_H
#define ADC_H

#include "stm32f4xx.h"
#include "uks.h"

#define ADC_LM35_CHANNELS_NUM 	DRYING_CHANNELS_NUM
#define ADC_FILTER_BUFFER_LEN	16

struct adc_lm35_channels
{
	uint16_t channel[ADC_LM35_CHANNELS_NUM];
	uint16_t filter_buffer[ADC_LM35_CHANNELS_NUM][ADC_FILTER_BUFFER_LEN];

};

void ADC_Channel_Init(void);

#endif
