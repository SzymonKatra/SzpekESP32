/*
 * weather.c
 *
 *  Created on: 26 cze 2020
 *      Author: szymo
 */


#include "weather.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static SemaphoreHandle_t s_mutex;

static bool s_isValid;
static double s_temperature;
static double s_pressure;
static double s_humidity;

void weatherInit()
{
	s_mutex = xSemaphoreCreateMutex();

	s_isValid = false;
}

bool weatherIsValid()
{
	bool result;

	xSemaphoreTake(s_mutex, portMAX_DELAY);

	result = s_isValid;

	xSemaphoreGive(s_mutex);

	return result;
}

void weatherInvalidate()
{
	xSemaphoreTake(s_mutex, portMAX_DELAY);

	s_isValid = false;

	xSemaphoreGive(s_mutex);
}

void weatherUpdate(double temperature, double pressure, double humidity)
{
	xSemaphoreTake(s_mutex, portMAX_DELAY);

	s_temperature = temperature;
	s_pressure = pressure; // fix by pressure/e^((-0.0289644*9.80665*altitude)/(8.3144598*(273.15+temperature)))
	s_humidity = humidity;
	s_isValid = true;

	xSemaphoreGive(s_mutex);
}

void weatherGet(weatherResult_t* result)
{
	xSemaphoreTake(s_mutex, portMAX_DELAY);

	if (s_isValid)
	{
		result->temperature = s_temperature;
		result->pressure = s_pressure;
		result->humidity = s_humidity;
	}
	else
	{
		result->temperature = 0;
		result->pressure = 0;
		result->humidity = 0;
	}

	xSemaphoreGive(s_mutex);
}
