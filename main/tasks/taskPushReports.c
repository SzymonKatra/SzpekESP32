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

static void toContract(const reportMeasurements_t* report, szpekApiV1ReportMeasurements_t* contract);
static void trySendUntilSucceeded(const szpekApiV1ReportMeasurements_t* contract);
static void sendStartupReportUntilSucceded();

void taskPushReports(void* p)
{
	QueueHandle_t reportsQueue = appGetITCStructures()->reportMeasurementsQueue;

	sendStartupReportUntilSucceded();

	while (1)
	{
		reportMeasurements_t report;
		if (xQueueReceive(reportsQueue, &report, portMAX_DELAY) != pdTRUE) continue;

		szpekApiV1ReportMeasurements_t contract;
		toContract(&report, &contract);

		trySendUntilSucceeded(&contract);
	}
}

static void toContract(const reportMeasurements_t* report, szpekApiV1ReportMeasurements_t* contract)
{
	contract->pm1Value = report->pm1;
	contract->pm2_5Value = report->pm2_5;
	contract->pm10Value = report->pm10;
	contract->samplesCount = report->samplesCount;
	contract->timestampFrom = report->timestampFrom;
	contract->timestampTo = report->timestampTo;
	contract->temperatureCelsius = report->temperature;
	contract->pressureHPa = report->pressure;
	contract->humidityPercent = report->humidity;
}

static void trySendUntilSucceeded(const szpekApiV1ReportMeasurements_t* contract)
{
	bool success = false;
	while (!success)
	{
		LOG_TASK_INFO("Pushing report... (%ld - %ld)", contract->timestampFrom, contract->timestampTo);
		success = szpekApiV1ReportMeasurements(contract);
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

static void sendStartupReportUntilSucceded()
{
	bool success = false;
	while (!success)
	{
		LOG_TASK_INFO("Reporting startup...");
		success = szpekApiV1ReportStartup(APP_FIRMWARE_NAME);
		if (success)
		{
			LOG_TASK_INFO("Startup reported successfully!");
			appFirmwareApply();
		}
		else
		{
			LOG_TASK_ERROR("An error occurred while reporting startup!");
			vTaskDelay(pdMS_TO_TICKS(5000));
		}
	}
}
