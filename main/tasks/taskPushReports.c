/*
 * taskPushReports.c
 *
 *  Created on: 15 mar 2020
 *      Author: szymo
 */

#include "taskPushReports.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void taskPushReports(void* p)
{
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
