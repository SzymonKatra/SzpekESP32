/*
 * timerTimeEvents.c
 *
 *  Created on: 22 mar 2020
 *      Author: szymo
 */

#include "timerTimeTriggers.h"

#include <time.h>
#include "logUtils.h"
#include "app.h"
#include "timeTriggers.h"

static struct tm s_prevTm;

void timerTimeTriggersInit()
{
	time_t timestamp = time(NULL);
	s_prevTm = *gmtime(&timestamp);
}

void timerTimeTriggersCallback(TimerHandle_t handle)
{
	time_t timestamp = time(NULL);
	struct tm currentTm = *gmtime(&timestamp);

	if (currentTm.tm_min != s_prevTm.tm_min)
	{
		LOG_TIMER_INFO(handle, "Minute elapsed");
		timeTriggersEventMinuteData_t data = currentTm.tm_min;
		esp_event_post_to(appGetEventLoopHandle(), TIME_TRIGGERS_EVENT, TIME_TRIGGERS_EVENT_MINUTE_PASSED, &data, sizeof(timeTriggersEventMinuteData_t), 0);
	}

	if (currentTm.tm_hour != s_prevTm.tm_hour)
	{
		LOG_TIMER_INFO(handle, "Hour elapsed");
		timeTriggersEventHourData_t data = currentTm.tm_hour;
		esp_event_post_to(appGetEventLoopHandle(), TIME_TRIGGERS_EVENT, TIME_TRIGGERS_EVENT_HOUR_PASSED, &data, sizeof(timeTriggersEventHourData_t), 0);
	}

	s_prevTm = currentTm;
}
