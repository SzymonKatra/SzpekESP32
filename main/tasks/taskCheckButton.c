/*
 * taskCheckButton.c
 *
 *  Created on: 20 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_TASKS_TASKCHECKBUTTON_C_
#define MAIN_TASKS_TASKCHECKBUTTON_C_

#include "taskCheckButton.h"

#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include "app.h"
#include "logUtils.h"

void taskCheckButton(void* p)
{
	// todo: maybe use interrupts?
	while (1)
	{
		// todo: led blink
		if (!gpio_get_level(APP_CONFIG_BUTTON_GPIO))
		{
			vTaskDelay(200 / portTICK_PERIOD_MS);
			// check if it is still high after some time
			if (!gpio_get_level(APP_CONFIG_BUTTON_GPIO))
			{
				appMode_t mode = appGetCurrentMode();
				LOG_TASK_INFO("Button pressed");
				appChangeMode(mode == APP_MODE_RUNNING ? APP_MODE_CONFIG : APP_MODE_RUNNING);
			}

			while (!gpio_get_level(APP_CONFIG_BUTTON_GPIO))
			{
				// wait to release
				vTaskDelay(100 / portTICK_PERIOD_MS);
			}
		}

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}


#endif /* MAIN_TASKS_TASKCHECKBUTTON_C_ */
