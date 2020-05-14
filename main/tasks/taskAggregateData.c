/*
 * taskAggregateData.c
 *
 *  Created on: 15 mar 2020
 *      Author: szymo
 */

#include "taskAggregateData.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/ringbuf.h>
#include <time.h>
#include "smog.h"
#include "reports.h"
#include "logUtils.h"
#include "app.h"
#include "timeTriggers.h"

const timeTriggersEvent_t TRIGGER_AGGREGATION_EVENT = TIME_TRIGGERS_EVENT_HOUR_PASSED;

static void timePassedHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void taskAggregateData(void* p)
{
	QueueHandle_t reportsQueue = appGetITCStructures()->reportSmogQueue;

	smogReset();
	appRegisterEventHandler(TIME_TRIGGERS_EVENT, TRIGGER_AGGREGATION_EVENT, timePassedHandler, NULL);

	time_t begin = time(NULL);
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		time_t end = time(NULL);

		smogResult_t smogResult;
		smogSummaryAndReset(&smogResult);

		reportSmog_t report;
		report.pm1 = smogResult.pm1;
		report.pm2_5 = smogResult.pm2_5;
		report.pm10 = smogResult.pm10;
		report.samplesCount = smogResult.samplesCount;
		report.timestampFrom = begin;
		report.timestampTo = end;

		// todo: mutex?
		if (uxQueueSpacesAvailable(reportsQueue) == 0)
		{
			reportSmog_t trash;
			xQueueReceive(reportsQueue, &trash, 0);
		}

		LOG_TASK_INFO("Queuing report %ld - %ld", report.timestampFrom, report.timestampTo);
		FREERTOS_ERROR_CHECK(xQueueSend(reportsQueue, &report, 0));

		begin = end;
	}
}

static void timePassedHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	xTaskNotifyGive(appGetTasks()->aggregateData);
}
