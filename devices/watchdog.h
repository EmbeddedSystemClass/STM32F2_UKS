#ifndef WATCHDOG_H
#define WATCHDOG_H
#include "stm32f4xx.h"

#define TASK_NUM	9//количество задач, кроме задачи ватчдога , в системе

enum
{
	PROTO_TASK=0,//ok
	BUZZER_TASK,//ok
	DISPLAY_TASK,//ok
	DRYING_TASK,//ok
	HEATER_INIT_TASK,//not required
	ADS1120_TASK,//ok
	ADC_TASK,//ok
	KEYBOARD_TASK,//ok
	HEATER_CONTROL_TASK,//ok
};

enum
{
	TASK_ACTIVE=0,
	TASK_IDLE=1
};

struct task_watch
{
	uint32_t counter;
	uint8_t  task_status;
};

void Watchdog_Init(void);

#endif
