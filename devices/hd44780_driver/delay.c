#include "delay.h"

__IO uint32_t TimingDelay = 0;
__IO uint32_t Time = 0;
__IO uint32_t Time2 = 0;
uint8_t DELAY_Initialized = 0;
volatile uint32_t mult;

#if defined(DELAY_TIM)
void DELAY_INT_InitTIM(void);
#endif

#if defined(DELAY_TIM)
void DELAY_TIM_IRQ_HANDLER(void) {
	TIM_ClearITPendingBit(DELAY_TIM, TIM_IT_Update);
#elif defined(KEIL_IDE)
void TimingDelay_Decrement(void) {
#else
void SysTick_Handler(void) {
#endif
	Time++;
	if (Time2 != 0x00) {
		Time2--;
	}
}

void DELAY_Init(void) {
	/* If already initialized */
	if (DELAY_Initialized) {
		return;
	}
	
	/* Enable External HSE clock */
//	RCC_HSEConfig(RCC_HSE_ON);
//
//	/* Wait for stable clock */
//	while (!RCC_WaitForHSEStartUp());
	
#if defined(DELAY_TIM)
	DELAY_INT_InitTIM();
#else
	/* Set Systick interrupt every 1ms */
	if (SysTick_Config(SystemCoreClock / 1000)) {
		/* Capture error */
		while (1);
	}
	
	#ifdef __GNUC__
		/* Set multiplier for delay under 1us with pooling mode = not so accurate */
		mult = SystemCoreClock / 7000000;
	#else
		/* Set multiplier for delay under 1us with pooling mode = not so accurate */
		mult = SystemCoreClock / 3000000;
	#endif
#endif
	
	/* Set initialized flag */
	DELAY_Initialized = 1;
}

void DELAY_EnableDelayTimer(void) {
	/* Check if library is even initialized */
	if (!DELAY_Initialized) {
		return;
	}
	
#if defined(DELAY_TIM)
	/* Enable TIMER for delay, useful when you wakeup from sleep mode */
	TIM_Cmd(DELAY_TIM, ENABLE);
#else
	/* Enable systick interrupts, useful when you wakeup from sleep mode */  
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
#endif
}

void DELAY_DisableDelayTimer(void) {
#if defined(DELAY_TIM)
	/* Disable TIMER for delay, useful when you go to sleep mode */
	TIM_Cmd(DELAY_TIM, DISABLE);
#else
	/* Disable systick, useful when you go to sleep mode */
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
#endif
}

/* Internal functions */
#if defined(DELAY_TIM)
void DELAY_INT_InitTIM(void) {
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	TIMER_PROPERTIES_t TIM_Data;
	
	/* Get timer properties */
	TIMER_PROPERTIES_GetTimerProperties(DELAY_TIM, &TIM_Data);
	
	/* Generate timer properties, 1us ticks */
	TIMER_PROPERTIES_GenerateDataForWorkingFrequency(&TIM_Data, 1000000);
	
	/* Enable clock for TIMx */
	TIMER_PROPERTIES_EnableClock(DELAY_TIM);
	
	/* Set timer settings */
	TIM_TimeBaseStruct.TIM_ClockDivision = 0;
	TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStruct.TIM_Period = 999; /* 1 millisecond */
	TIM_TimeBaseStruct.TIM_Prescaler = 0;//SystemCoreClock / (1000000 * (SystemCoreClock / TIM_Data.TimerFrequency)) - 1; /* With prescaler for 1 microsecond tick */
	TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
	
	/* Initialize timer */
	TIM_TimeBaseInit(DELAY_TIM, &TIM_TimeBaseStruct);
	
	/* Enable interrupt */
	TIM_ITConfig(DELAY_TIM, TIM_IT_Update, ENABLE);
	
	/* Set NVIC parameters */
	NVIC_InitStruct.NVIC_IRQChannel = DELAY_TIM_IRQ;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 14;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	/* Add to NVIC */
	NVIC_Init(&NVIC_InitStruct);
	
	/* Start timer */
	TIM_Cmd(DELAY_TIM, ENABLE);
}
#endif

