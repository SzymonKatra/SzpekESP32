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
#include <driver/touch_pad.h>
#include "app.h"
#include "logUtils.h"

void taskCheckButton(void* p)
{
	//touch_pad_init();
	//touch_pad_set_voltage(TOUCH_HVOLT_2V4, TOUCH_LVOLT_0V8, TOUCH_HVOLT_ATTEN_1V5);
	//for (int i = 0;i< TOUCH_PAD_MAX;i++)
	//{
	//    touch_pad_config(i, 0);
	//}
	//touch_pad_filter_start(10);

	// todo: maybe use interrupts?
	while (1)
	{
		//uint16_t val, rawVal;
		//touch_pad_read_filtered(TOUCH_PAD_NUM7, &val);
		//touch_pad_read_raw_data(TOUCH_PAD_NUM7, &rawVal);
		//LOG_TASK_INFO("Touch 7 = %d, raw = %d", val, rawVal);

		// todo: led blink
		if (!gpio_get_level(APP_BUTTON_GPIO))
		{
			vTaskDelay(200 / portTICK_PERIOD_MS);
			// check if it is still high after some time
			if (!gpio_get_level(APP_BUTTON_GPIO))
			{
				appMode_t mode = appGetCurrentMode();
				LOG_TASK_INFO("Button pressed");
				appChangeMode(mode == APP_MODE_RUNNING ? APP_MODE_CONFIG : APP_MODE_RUNNING);
			}

			while (!gpio_get_level(APP_BUTTON_GPIO))
			{
				// wait to release
				vTaskDelay(100 / portTICK_PERIOD_MS);
			}
		}

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}


#endif /* MAIN_TASKS_TASKCHECKBUTTON_C_ */
