/*
 * taskPushReports.c
 *
 *  Created on: 15 mar 2020
 *      Author: szymo
 */

#include "taskPushReports.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "szpekApi/legacy/szpekApiLegacy.h"
#include "szpekApi/v1/szpekApiV1.h"
#include "app.h"
#include "settings.h"
#include "logUtils.h"
#include "reports.h"
#include "network.h"

static void toContract(const report_t* report, szpekApiLegacySensorsMicroContract_t* contract);
static void trySendUntilSucceeded(const szpekApiLegacySensorsMicroContract_t* contract);

void taskPushReports(void* p)
{
	QueueHandle_t reportsQueue = appGetITCStructures()->reportsQueue;

	while (1)
	{
		report_t report;
		if (xQueueReceive(reportsQueue, &report, portMAX_DELAY) != pdTRUE) continue;

		szpekApiLegacySensorsMicroContract_t contract;
		toContract(&report, &contract);

		trySendUntilSucceeded(&contract);
	}
}

static void toContract(const report_t* report, szpekApiLegacySensorsMicroContract_t* contract)
{
	contract->pm1Value = report->pm1;
	contract->pm25Value = report->pm2_5;
	contract->pm10Value = report->pm10;
	contract->samplesQuantity = report->samplesCount;
	contract->periodFrom = report->timestampFrom;
	contract->periodTo = report->timestampTo;
	contract->sensorCode = settingsGetSzpekId()->code; // todo: move to szpekApi
}

static void trySendUntilSucceeded(const szpekApiLegacySensorsMicroContract_t* contract)
{
	bool success = false;
	while (!success)
	{
		LOG_TASK_INFO("Pushing report... (%ld - %ld)", contract->periodFrom, contract->periodTo);
		success = szpekApiLegacySensorsMicro(contract);
		if (success)
		{
			LOG_TASK_INFO("Report pushed successfully (%ld - %ld)!", contract->periodFrom, contract->periodTo);
		}
		else
		{
			LOG_TASK_ERROR("An error occurred while pushing report (%ld - %ld)!", contract->periodFrom, contract->periodTo);
			vTaskDelay(pdMS_TO_TICKS(10000));
		}
	}
}
