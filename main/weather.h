/*
 * weather.h
 *
 *  Created on: 26 cze 2020
 *      Author: szymo
 */

#ifndef MAIN_WEATHER_H_
#define MAIN_WEATHER_H_

#include <stdbool.h>

typedef struct
{
	double temperature;
	double pressure;
	double humidity;
} weatherResult_t;

void weatherInit();

bool weatherIsValid();
void weatherInvalidate();
void weatherUpdate(double temperature, double pressure, double humidity);

void weatherGet(weatherResult_t* result);

#endif /* MAIN_WEATHER_H_ */
