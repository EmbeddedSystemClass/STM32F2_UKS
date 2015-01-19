#include "time_forecast.h"

#define MAX_FORECAST_TIME			30//MINUTES
#define NUM_OF_CHARACTERISTIC		5

const float dry_characteristic[NUM_OF_CHARACTERISTIC][MAX_FORECAST_TIME]=
{
		{0.0},
		{0.0},
		{0.0},
		{0.0},
		{0.0}
};

uint16_t Time_Forecast_Get_Time(float derivative_temp,uint16_t current_time)
{

	return 0;
}
