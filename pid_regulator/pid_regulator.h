#ifndef PID_H
#define PID_H

#include "stdint.h"
#include "phaze_detector.h"

#define SCALING_FACTOR  128//128

/*! \brief PID Status
 *
 * Setpoints and data used by the PID control algorithm
 */
typedef struct PID_DATA{
  //! Last process value, used to find derivative of process value.
  float lastProcessValue;
  //! Summation of errors, used for integrate calculations
  float sumError;
  //! The Proportional tuning constant, multiplied with SCALING_FACTOR
  float P_Factor;
  //! The Integral tuning constant, multiplied with SCALING_FACTOR
  float I_Factor;
  //! The Derivative tuning constant, multiplied with SCALING_FACTOR
  float D_Factor;
  //! Maximum allowed error, avoid overflow
  float maxError;
  //! Maximum allowed sumerror, avoid overflow
  float maxSumError;
} pidData_t;

/*! \brief Maximum values
 *
 * Needed to avoid sign/overflow problems
 */
// Maximum value of variables
#define MAX_INT         HEATER_TEMP_MAX//INT16_MAX
#define MAX_LONG        MAX_INT
#define MAX_I_TERM      15/*(MAX_INT / 2)*/

// Boolean values
#define FALSE           0
#define TRUE            1

#define  TEMP_TUMBLR_RCC				RCC_AHB1Periph_GPIOB
#define  TEMP_TUMBLR_PORT				GPIOB
#define  TEMP_TUMBLR_TEMP_1_PIN			GPIO_Pin_9
#define  TEMP_TUMBLR_TEMP_2_PIN			GPIO_Pin_8

enum
{
	TUMBLR_TEMP_1=0,
	TUMBLR_TEMP_2=1
};

void pid_Init(float p_factor, float i_factor, float d_factor, struct PID_DATA *pid);
int16_t pid_Controller(float setPoint, float processValue, struct PID_DATA *pid_st);
void pid_Reset_Integrator(pidData_t *pid_st);
void PID_Heater_Init(void);

#endif
