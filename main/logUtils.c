/*
 * logUtils.c
 *
 *  Created on: 31 pa? 2020
 *      Author: szymo
 */

#include "logUtils.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdio.h>
#include <esp_log.h>
#include "app.h"
#include "settings.h"

void enqueueRemoteLog(const char* log)
{
	QueueHandle_t logsQueue = appGetLogsQueue();
	if (uxQueueSpacesAvailable(logsQueue) == 0)
	{
		char trash[APP_REMOTE_LOG_LENGTH];
		xQueueReceive(logsQueue, &trash, 0);
	}

	//ESP_LOGI("LogUtils", "report queued %s", log);
	xQueueSend(logsQueue, log, 0);
}
