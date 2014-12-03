#ifndef TIMER_PROPERTIES_H
#define TIMER_PROPERTIES_H	100

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

/**
 * Result enumeration
 *
 * Parameters:
 * 	- TIMER_PROPERTIES_Result_Ok:
 * 		Everything OK
 * 	- TIMER_PROPERTIES_Result_Error:
 * 		An error occured
 * 	- TIMER_PROPERTIES_Result_TimerNotValid:
 * 		Timer is not valid
 * 	- TIMER_PROPERTIES_Result_FrequencyTooHigh:
 * 		Frequency for timer is too high
 * 	- TIMER_PROPERTIES_Result_FrequencyTooLow:
 * 		Frequency for timer is too low
 */
typedef enum {
	TIMER_PROPERTIES_Result_Ok,
	TIMER_PROPERTIES_Result_Error,
	TIMER_PROPERTIES_Result_TimerNotValid,
	TIMER_PROPERTIES_Result_FrequencyTooHigh,
	TIMER_PROPERTIES_Result_FrequencyTooLow
} TIMER_PROPERTIES_Result_t;

/**
 * Struct for timer data
 * 
 * Parameters:
 * 	- uint32_t TimerFrequency:
 * 		timer's working frequency
 * 	- uint32_t MaxPeriod:
 * 		Max timer period
 * 	- uint32_t MaxPrescaler:
 * 		Max timer prescaler
 * 	- uint32_t Period:
 * 		Timer's working period
 * 	- uint32_t Prescaler:
 * 		Timer's working prescaler
 * 	- uint32_t Frequency:
 * 		Timer's reload frequency
 */
typedef struct {
	uint32_t TimerFrequency;
	uint32_t MaxPeriod;
	uint32_t MaxPrescaler;
	uint32_t Period;
	uint32_t Prescaler;
	uint32_t Frequency;
} TIMER_PROPERTIES_t;

/**
 * Returns you a timer properties
 * 
 * Parameters:
 *	- TIM_TypeDef* TIMx:
 *		Timer used to get settings for
 *	- TIMER_PROPERTIES_t* Timer_Data:
 *		Pointer to TIMER_PROPERTIES_t struct to store data to
 *		
 * Member of TIMER_PROPERTIES_Result_t is returned
 */
extern TIMER_PROPERTIES_Result_t TIMER_PROPERTIES_GetTimerProperties(TIM_TypeDef* TIMx, TIMER_PROPERTIES_t* Timer_Data);

/**
 * Generate period and prescaller for given timer frequency
 * 
 * Parameters:
 *	- TIMER_PROPERTIES_t* Timer_Data:
 *		Pointer for timer data
 *	- uint32_t frequency:
 *		Frequency used
 *		
 * Member of TIMER_PROPERTIES_Result_t is returned
 */
extern TIMER_PROPERTIES_Result_t TIMER_PROPERTIES_GenerateDataForWorkingFrequency(TIMER_PROPERTIES_t* Timer_Data, double frequency);

/**
 * Enable timer's clock
 * 
 * Parameters:
 * 	- TIM_TypeDef* TIMx:
 * 		Timer to enable clock for
 * 		
 * Member of TIMER_PROPERTIES_Result_t is returned
 */
extern TIMER_PROPERTIES_Result_t TIMER_PROPERTIES_EnableClock(TIM_TypeDef* TIMx);

/**
 * Disable timer's clock
 * 
 * Parameters:
 * 	- TIM_TypeDef* TIMx:
 * 		Timer to disable clock for
 * 		
 * Member of TIMER_PROPERTIES_Result_t is returned
 */
extern TIMER_PROPERTIES_Result_t TIMER_PROPERTIES_DisableClock(TIM_TypeDef* TIMx);
 
#endif

