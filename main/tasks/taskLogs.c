/*
 * taskLogs.c
 *
 *  Created on: 31 pa? 2020
 *      Author: szymo
 */

#include "taskLogs.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "szpekApi/v1/szpekApiV1.h"
#include "app.h"
#include "settings.h"
#include "logUtils.h"
#include "reports.h"
#include "network.h"

static void trySendUntilSucceeded(const char* logMessage);

void taskLogs(void* p)
{
	while (1)
	{
		char log[APP_REMOTE_LOG_LENGTH];
		if (xQueueReceive(appGetLogsQueue(), log, portMAX_DELAY) != pdTRUE) continue;

		trySendUntilSucceeded(log);
	}
}

static void trySendUntilSucceeded(const char* log)
{
	bool success = false;
	while (!success)
	{
		//ESP_LOGI("TaskLogs", "sending log %s", log);
		success = szpekApiV1Log(log);
		if (!success) {
			vTaskDelay(pdMS_TO_TICKS(10000));
		}
	}
}
