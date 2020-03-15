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
#include <time.h>
#include "smog.h"
#include "reports.h"
#include "logUtils.h"
#include "app.h"

static void waitForMinuteChange(time_t beginTimestamp);

void taskAggregateData(void* p)
{
	LOG_TASK_INFO("aggreg prol");
	smogReset();

	while (1)
	{
		time_t begin = time(NULL);
		waitForMinuteChange(begin);

		smogResult_t smogResult;
		smogSummaryAndReset(&smogResult);

		time_t end = time(NULL);

		report_t report;
		report.pm1 = smogResult.pm1;
		report.pm2_5 = smogResult.pm2_5;
		report.pm10 = smogResult.pm10;
		report.samplesCount = smogResult.samplesCount;
		report.timestampFrom = begin;
		report.timestampTo = end;

		FREERTOS_ERROR_CHECK(xQueueSend(appGetITCStructures()->reportsQueue, &report, pdMS_TO_TICKS(1000)));
		LOG_TASK_INFO("Report queued");
	}
}


static void waitForMinuteChange(time_t beginTimestamp)
{
	struct tm begin = *gmtime(&beginTimestamp);

	time_t currentTimestamp = time(NULL);
	struct tm current = *gmtime(&currentTimestamp);
	while (current.tm_min == begin.tm_min)
	{
		vTaskDelay(pdMS_TO_TICKS(250));
		currentTimestamp = time(NULL);
		current = *gmtime(&currentTimestamp);
	}
}
