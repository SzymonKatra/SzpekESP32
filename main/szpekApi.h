/*
 * szpekApi.h
 *
 *  Created on: 18 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_SZPEKAPI_H_
#define MAIN_SZPEKAPI_H_

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	float pm1Value;
	float pm25Value;
	float pm10Value;
	uint32_t samplesQuantity;
	time_t periodFrom;
	time_t periodTo;
	const char* sensorCode;
} szpekApiSensorsMicroData_t;

bool szpekApiSensorsMicro(const szpekApiSensorsMicroData_t* data);

#endif /* MAIN_SZPEKAPI_H_ */
