#include "pid_regulator.h"
#include "stdint.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "phaze_detector.h"
#include "ADS1120.h"
#include "uks.h"

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

extern xSemaphoreHandle xPhazeSemaphore;
extern struct ADS1120_result ADS1120_res;
extern struct uks uks_channels;

struct PID_DATA pid_heater;

enum
{
	TUMBLR_TEMP_1=0,
	TUMBLR_TEMP_2=1
};

static volatile tumblr_state=TUMBLR_TEMP_1;

static void PID_Regulator_Task(void *pvParameters);

/*! \brief Initialisation of PID controller parameters.
 *
 *  Initialise the variables used by the PID algorithm.
 *
 *  \param p_factor  Proportional term.
 *  \param i_factor  Integral term.
 *  \param d_factor  Derivate term.
 *  \param pid  Struct with PID status.
 */
void PID_Heater_Init(void)
{
	pid_Init(120.0,3.0,0.0,&pid_heater);
}
void pid_Init(float p_factor, float i_factor, float d_factor, struct PID_DATA *pid)
// Set up PID controller parameters
{
  // Start values for PID controller
  pid->sumError = 0;
  pid->lastProcessValue = 0;
  // Tuning constants for PID loop
  pid->P_Factor = p_factor;
  pid->I_Factor = i_factor;
  pid->D_Factor = d_factor;
  // Limits to avoid overflow
  pid->maxError = MAX_INT / (pid->P_Factor + 1);
  pid->maxSumError = MAX_I_TERM / (pid->I_Factor + 1);

    GPIO_InitTypeDef GPIO_InitStruct;
  	RCC_AHB1PeriphClockCmd(TEMP_TUMBLR_RCC, ENABLE);

  	//Common settings
  	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;

  	//RS
  	GPIO_InitStruct.GPIO_Pin = TEMP_TUMBLR_TEMP_1_PIN|TEMP_TUMBLR_TEMP_2_PIN;
  	GPIO_Init(TEMP_TUMBLR_PORT, &GPIO_InitStruct);
  	//GPIO_WriteBit(TEMP_TUMBLR_PORT, HD44780_RS_PIN, Bit_RESET);

  xTaskCreate(PID_Regulator_Task,(signed char*)"PID",128,NULL, tskIDLE_PRIORITY + 1, NULL);
}


/*! \brief PID control algorithm.
 *
 *  Calculates output from setpoint, process value and PID status.
 *
 *  \param setPoint  Desired value.
 *  \param processValue  Measured value.
 *  \param pid_st  PID status struct.
 */
int16_t pid_Controller(float setPoint, float processValue, struct PID_DATA *pid_st)
{
  float error, p_term, d_term;
  float i_term, ret, temp;

  error = setPoint - processValue;

  // Calculate Pterm and limit error overflow
  if (error > pid_st->maxError){
    p_term = MAX_INT;
  }
  else if (error < -pid_st->maxError){
    p_term = -MAX_INT;
  }
  else{
    p_term =(int16_t)(pid_st->P_Factor * error);
  }

  // Calculate Iterm and limit integral runaway
  temp = pid_st->sumError + error;
  if(temp > pid_st->maxSumError)
  {
	  i_term = MAX_I_TERM;
	  pid_st->sumError = pid_st->maxSumError;
  }
  else if(temp < -pid_st->maxSumError)
  {
	  i_term = -MAX_I_TERM;
	  pid_st->sumError = -pid_st->maxSumError;
  }
  else
  {
    pid_st->sumError = temp;
    i_term =(float)(pid_st->I_Factor * pid_st->sumError);
  }

  // Calculate Dterm
  d_term =(float)(pid_st->D_Factor * (pid_st->lastProcessValue - processValue));

  pid_st->lastProcessValue = processValue;

  ret = (p_term + i_term + d_term)/* / SCALING_FACTOR*/;
  if(ret > MAX_INT)
  {
	  ret = MAX_INT;
  }
  else if(ret < /*-MAX_INT*/0){
    ret = /*-MAX_INT*/0;
  }

  return((float)ret);
}

/*! \brief Resets the integrator.
 *
 *  Calling this function will reset the integrator in the PID regulator.
 */
void pid_Reset_Integrator(pidData_t *pid_st)
{
  pid_st->sumError = 0;
}

static void PID_Regulator_Task(void *pvParameters)
{
	while(1)
	{
		if( xSemaphoreTake( xPhazeSemaphore, ( portTickType ) portMAX_DELAY ) == pdTRUE )
		{
			if(GPIO_ReadInputDataBit(TEMP_TUMBLR_PORT,TEMP_TUMBLR_TEMP_1_PIN)==Bit_RESET)
			{
				if(tumblr_state!=TUMBLR_TEMP_1)
				{
					tumblr_state=TUMBLR_TEMP_1;
					pid_heater.sumError = 0;
					pid_heater.lastProcessValue = 0;
					pid_heater.maxError = MAX_INT / (pid_heater.P_Factor + 1);
					pid_heater.maxSumError = MAX_I_TERM / (pid_heater.I_Factor + 1);
				}
				Set_Heater_Power((uint8_t)pid_Controller(70.0,uks_channels.heater_temperature,&pid_heater));
			}
			else
			{
				if(tumblr_state!=TUMBLR_TEMP_2)
				{
					tumblr_state=TUMBLR_TEMP_2;
					pid_heater.sumError = 0;
					pid_heater.lastProcessValue = 0;
					pid_heater.maxError = MAX_INT / (pid_heater.P_Factor + 1);
					pid_heater.maxSumError = MAX_I_TERM / (pid_heater.I_Factor + 1);
				}
				Set_Heater_Power((uint8_t)pid_Controller(50.0,uks_channels.heater_temperature,&pid_heater));
			}
		}
	}
}
