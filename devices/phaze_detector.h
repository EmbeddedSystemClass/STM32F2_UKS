#ifndef  PHAZE_DETECTOR_H
#define  PHAZE_DETECTOR_H
#include "stm32f4xx.h"

void Phaze_Detector_Init(void);
void Set_Heater_Power(uint8_t power);
void Heater_Power_Down_Block(void);

#define ZERO_CROSS_PORT    			GPIOB
#define ZERO_CROSS_PORT_RCC 		RCC_AHB1Periph_GPIOB
#define ZERO_CROSS_PORT_EXTI 		EXTI_PortSourceGPIOB

#define ZERO_CROSS 					GPIO_Pin_0
#define CONTR_SP	 				GPIO_Pin_1

#define ZERO_CROSS_PinSource 		GPIO_PinSource0
#define CONTR_SP_PinSource	 		GPIO_PinSource1

#define RELAY_PORT					GPIOB
#define RELAY_PORT_RCC 				RCC_AHB1Periph_GPIOB
#define RELAY_PORT_EXTI 			EXTI_PortSourceGPIOB

#define RELAY_PIN					GPIO_Pin_2

#define MAX_POWER_VALUE		50

struct phaze_detector
{
	uint16_t zero_cross_counter;
	uint16_t input_sp_counter;
	uint16_t contr_sp_counter;
	uint8_t  power_value;//0..50
};

#endif
