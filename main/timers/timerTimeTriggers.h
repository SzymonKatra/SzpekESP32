/*
 * timerTimeEvents.h
 *
 *  Created on: 22 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_TIMERS_TIMERTIMETRIGGERS_H_
#define MAIN_TIMERS_TIMERTIMETRIGGERS_H_

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

void timerTimeTriggersInit();
void timerTimeTriggersCallback(TimerHandle_t handle);

#endif /* MAIN_TIMERS_TIMERTIMETRIGGERS_H_ */
