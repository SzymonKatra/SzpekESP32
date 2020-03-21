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
#include "szpekApi.h"
#include "app.h"
#include "settings.h"
#include "logUtils.h"
#include "reports.h"
#include "network.h"

static void toContract(const report_t* report, szpekApiSensorsMicroContract_t* contract);

void taskPushReports(void* p)
{
	QueueHandle_t reportsQueue = appGetITCStructures()->reportsQueue;

	while (1)
	{
		report_t report;
		FREERTOS_ERROR_CHECK(xQueuePeek(reportsQueue, &report, portMAX_DELAY));

		szpekApiSensorsMicroContract_t contract;
		toContract(&report, &contract);

		LOG_TASK_INFO("Pushing report... (%ld - %ld)", contract.periodFrom, contract.periodTo);
		if (szpekApiSensorsMicro(&contract))
		{
			FREERTOS_ERROR_CHECK(xQueueReceive(reportsQueue, &report, 0));
			LOG_TASK_INFO("Report pushed successfully (%ld - %ld)!", contract.periodFrom, contract.periodTo);
		}
		else
		{
			LOG_TASK_ERROR("An error occurred while pushing report (%ld - %ld)!", contract.periodFrom, contract.periodTo);
			vTaskDelay(pdMS_TO_TICKS(10000));
		}
	}
}

static void toContract(const report_t* report, szpekApiSensorsMicroContract_t* contract)
{
	contract->pm1Value = report->pm1;
	contract->pm25Value = report->pm2_5;
	contract->pm10Value = report->pm10;
	contract->samplesQuantity = report->samplesCount;
	contract->periodFrom = report->timestampFrom;
	contract->periodTo = report->timestampTo;
	contract->sensorCode = settingsGetSzpekId()->code; // todo: move to szpekApi
}
