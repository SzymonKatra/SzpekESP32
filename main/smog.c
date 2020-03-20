/*
 * smog.c
 *
 *  Created on: 15 mar 2020
 *      Author: szymo
 */

#include "smog.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static SemaphoreHandle_t s_mutex;

static uint32_t s_accumulatedPm1;
static uint32_t s_accumulatedPm2_5;
static uint32_t s_accumulatedPm10;
static uint32_t s_samplesCount;

static void smogResetInternal();

void smogInit()
{
	s_mutex = xSemaphoreCreateMutex();

	smogResetInternal();
}

void smogAccumulate(uint16_t pm1, uint16_t pm2_5, uint16_t pm10)
{
	xSemaphoreTake(s_mutex, portMAX_DELAY);

	s_accumulatedPm1 += pm1;
	s_accumulatedPm2_5 += pm2_5;
	s_accumulatedPm10 += pm10;
	s_samplesCount++;

	xSemaphoreGive(s_mutex);
}

void smogReset()
{
	xSemaphoreTake(s_mutex, portMAX_DELAY);

	smogResetInternal();

	xSemaphoreGive(s_mutex);
}

void smogSummaryAndReset(smogResult_t* result)
{
	xSemaphoreTake(s_mutex, portMAX_DELAY);

	result->pm1 = (float)s_accumulatedPm1 / s_samplesCount;
	result->pm2_5 = (float)s_accumulatedPm2_5 / s_samplesCount;
	result->pm10 = (float)s_accumulatedPm10 / s_samplesCount;
	result->samplesCount = s_samplesCount;
	smogResetInternal();

	xSemaphoreGive(s_mutex);
}

static void smogResetInternal()
{
	s_accumulatedPm1 = 0;
	s_accumulatedPm2_5 = 0;
	s_accumulatedPm10 = 0;
	s_samplesCount = 0;
}
