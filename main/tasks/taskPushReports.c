/*
 * taskPushReports.c
 *
 *  Created on: 15 mar 2020
 *      Author: szymo
 */

#include "taskPushReports.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <szpekApi.h>
#include "settings.h"
#include "logUtils.h"

void taskPushReports(void* p)
{
	szpekApiSensorsMicroData_t data;
	data.pm1Value = 4;
	data.pm25Value = 44;
	data.pm10Value = 444;
	data.samplesQuantity = 997;
	data.periodFrom = 1584574846;
	data.periodTo = 1584575000;
	data.sensorCode = settingsGetSensorCode();
	bool result = szpekApiSensorsMicro(&data);
	LOG_TASK_INFO("api call result: %d", result);

	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
