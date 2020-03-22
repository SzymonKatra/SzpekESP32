/*
 * timeEvents.h
 *
 *  Created on: 22 mar 2020
 *      Author: szymo
 */

#include <esp_event.h>

ESP_EVENT_DECLARE_BASE(TIME_TRIGGERS_EVENT);

typedef enum
{
	TIME_TRIGGERS_EVENT_MINUTE_PASSED,
	TIME_TRIGGERS_EVENT_HOUR_PASSED
} timeTriggersEvent_t;

