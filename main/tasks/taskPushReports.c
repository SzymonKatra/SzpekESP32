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
#include "szpekApi/v1/szpekApiV1.h"
#include "app.h"
#include "settings.h"
#include "logUtils.h"
#include "reports.h"
#include "network.h"

static void toContract(const report_t* report, szpekApiV1ReportSmog_t* contract);
static void trySendUntilSucceeded(const szpekApiV1ReportSmog_t* contract);

void taskPushReports(void* p)
{
	QueueHandle_t reportsQueue = appGetITCStructures()->reportsQueue;

	while (1)
	{
		report_t report;
		if (xQueueReceive(reportsQueue, &report, portMAX_DELAY) != pdTRUE) continue;

		szpekApiV1ReportSmog_t contract;
		toContract(&report, &contract);

		trySendUntilSucceeded(&contract);
	}
}

static void toContract(const report_t* report, szpekApiV1ReportSmog_t* contract)
{
	contract->pm1Value = report->pm1;
	contract->pm2_5Value = report->pm2_5;
	contract->pm10Value = report->pm10;
	contract->samplesCount = report->samplesCount;
	contract->timestampFrom = report->timestampFrom;
	contract->timestampTo = report->timestampTo;
}

static void trySendUntilSucceeded(const szpekApiV1ReportSmog_t* contract)
{
	bool success = false;
	while (!success)
	{
		LOG_TASK_INFO("Pushing report... (%ld - %ld)", contract->timestampFrom, contract->timestampTo);
		success = szpekApiV1ReportSmog(contract);
		if (success)
		{
			LOG_TASK_INFO("Report pushed successfully (%ld - %ld)!", contract->timestampFrom, contract->timestampTo);
		}
		else
		{
			LOG_TASK_ERROR("An error occurred while pushing report (%ld - %ld)!", contract->timestampFrom, contract->timestampTo);
			vTaskDelay(pdMS_TO_TICKS(10000));
		}
	}
}
