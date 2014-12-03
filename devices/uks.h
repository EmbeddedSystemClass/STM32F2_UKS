#ifndef 	UKS_H
#define  	UKS_H

#include "stm32f4xx.h"

struct calibrate
{
	uint16_t adc_code;
	uint32_t value;
};

struct backup_registers
{
	struct calibrate thermo_resistor;
};

struct uks
{
	struct 		backup_registers *bkp_reg;
	uint8_t 	buzzer_enable;
};

#endif
