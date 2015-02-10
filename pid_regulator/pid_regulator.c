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

#include "watchdog.h"

extern xSemaphoreHandle xPhazeSemaphore;
extern struct ADS1120_result ADS1120_res;
extern struct uks uks_channels;

struct PID_DATA pid_heater;

extern struct task_watch task_watches[];

xTaskHandle PID_Regulator_Task_Handle;

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
	if(uks_channels.device_error!=ERROR_NONE)
	{
		return;
	}

//	uks_channels.uks_params.p_factor=120.0;
//	uks_channels.uks_params.i_factor=3.0;
//	uks_channels.uks_params.d_factor=0.0;
//
//	uks_channels.uks_params.heater_temperature_1=50.0;
//	uks_channels.uks_params.heater_temperature_2=70.0;

	pid_Init(uks_channels.uks_params.p_factor,uks_channels.uks_params.i_factor,uks_channels.uks_params.d_factor,&pid_heater);
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

  if(pid->P_Factor!=0.0)
  {
	  pid->maxError = MAX_P_TERM / pid->P_Factor;
  }
  else
  {
	  pid->maxError=0.0;
  }

  if(pid->I_Factor!=0.0)
  {
	  pid->maxSumError = MAX_I_TERM /pid->I_Factor;
  }
  else
  {
	  pid->maxSumError =0.0;
  }

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

  	uks_channels.heater_tempereature_tumblr=TUMBLR_TEMP_1;

  xTaskCreate(PID_Regulator_Task,(signed char*)"PID",128,NULL, tskIDLE_PRIORITY + 1, &PID_Regulator_Task_Handle);
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

  if(pid_st->P_Factor!=0.0)
  {
	  pid_st->maxError = MAX_P_TERM / pid_st->P_Factor;
  }
  else
  {
	  pid_st->maxError=0.0;
  }

  if (error > pid_st->maxError)
  {
    p_term = pid_st->P_Factor*pid_st->maxError;
  }
  else if (error < (-pid_st->maxError))
  {
    p_term = pid_st->P_Factor*(-pid_st->maxError);
  }
  else{
    p_term =pid_st->P_Factor * error;
  }

  // Calculate Iterm and limit integral runaway


  if(pid_st->I_Factor!=0.0)
  {
	  pid_st->maxSumError = MAX_I_TERM /pid_st->I_Factor;
  }
  else
  {
	  pid_st->maxSumError =0.0;
  }

  temp = pid_st->sumError + error;
  if(temp > pid_st->maxSumError)
  {
	  i_term = pid_st->I_Factor *pid_st->maxSumError;
	  pid_st->sumError = pid_st->maxSumError;
  }
  else if(temp < (-pid_st->maxSumError))
  {
	  i_term = pid_st->I_Factor *(-pid_st->maxSumError);
	  pid_st->sumError = -pid_st->maxSumError;
  }
  else
  {
    pid_st->sumError = temp;
    i_term =pid_st->I_Factor * pid_st->sumError;
  }

  // Calculate Dterm
  d_term =pid_st->D_Factor * (pid_st->lastProcessValue - processValue);

  if(d_term>MAX_D_TERM)
  {
	  d_term=MAX_D_TERM;
  }
  else if(d_term<(-MAX_D_TERM))
  {
	  d_term=-MAX_D_TERM;
  }

  pid_st->lastProcessValue = processValue;

  ret = (p_term + i_term + d_term)/* / SCALING_FACTOR*/;

  uks_channels.uks_params.end_drying_temperature[0]=p_term;
  uks_channels.uks_params.end_drying_temperature[1]=i_term;
  uks_channels.uks_params.end_drying_temperature[2]=d_term;


  if(ret > MAX_POWER_VALUE)
  {
	  ret = MAX_POWER_VALUE;
  }
  else if(ret < /*-MAX_INT*/0){
    ret = /*-MAX_INT*/0;
  }

  return ret;
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
	task_watches[PID_TASK].task_status=TASK_ACTIVE;
	while(1)
	{
		if( xSemaphoreTake( xPhazeSemaphore, ( portTickType ) portMAX_DELAY ) == pdTRUE )
		{
			pid_heater.P_Factor=uks_channels.uks_params.p_factor;
			pid_heater.I_Factor=uks_channels.uks_params.i_factor;
			pid_heater.D_Factor=uks_channels.uks_params.d_factor;

			if(GPIO_ReadInputDataBit(TEMP_TUMBLR_PORT,TEMP_TUMBLR_TEMP_1_PIN)==Bit_RESET)
			{
				if(uks_channels.heater_tempereature_tumblr!=TUMBLR_TEMP_1)
				{
					uks_channels.heater_tempereature_tumblr=TUMBLR_TEMP_1;
					pid_heater.sumError = 0;
					pid_heater.lastProcessValue = 0;
					pid_heater.maxError = MAX_INT / (pid_heater.P_Factor + 1);
					pid_heater.maxSumError = MAX_I_TERM / (pid_heater.I_Factor + 1);
				}
				Set_Heater_Power((uint8_t)pid_Controller(uks_channels.uks_params.heater_temperature_1,uks_channels.heater_temperature,&pid_heater));
			}
			else
			{
				if(uks_channels.heater_tempereature_tumblr!=TUMBLR_TEMP_2)
				{
					uks_channels.heater_tempereature_tumblr=TUMBLR_TEMP_2;
					pid_heater.sumError = 0;
					pid_heater.lastProcessValue = 0;
					pid_heater.maxError = MAX_INT / (pid_heater.P_Factor + 1);
					pid_heater.maxSumError = MAX_I_TERM / (pid_heater.I_Factor + 1);
				}
				Set_Heater_Power((uint8_t)pid_Controller(uks_channels.uks_params.heater_temperature_2,uks_channels.heater_temperature,&pid_heater));
			}

			task_watches[PID_TASK].counter++;
		}
	}
}
