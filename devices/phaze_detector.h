#ifndef  PHAZE_DETECTOR_H
#define  PHAZE_DETECTOR_H
#include "stm32f4xx.h"

void Phaze_Detector_Init(void);

#define ZERO_CROSS_PORT    			GPIOB
#define ZERO_CROSS_PORT_RCC 		RCC_AHB1Periph_GPIOB
#define ZERO_CROSS_PORT_EXTI 		EXTI_PortSourceGPIOB

#define ZERO_CROSS 					GPIO_Pin_0
#define CONTR_SP	 				GPIO_Pin_1

#define ZERO_CROSS_PinSource 		GPIO_PinSource0
#define CONTR_SP_PinSource	 		GPIO_PinSource1


#endif
