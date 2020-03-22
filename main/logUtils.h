/*
 * utils.h
 *
 *  Created on: 14 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_TASKUTILS_H_
#define MAIN_TASKUTILS_H_

#include <stdlib.h> /* for abort() */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <esp_log.h>

#define LOG_TASK_ERROR(format, ... ) ESP_LOGE(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_WARNING(format, ... ) ESP_LOGW(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_INFO(format, ... ) ESP_LOGI(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_DEBUG(format, ... ) ESP_LOGD(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_VERBOSE(format, ... ) ESP_LOGV(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)

#define LOG_TIMER_ERROR(timerHandle, format, ... ) ESP_LOGE(pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)
#define LOG_TIMER_WARNING(timerHandle, format, ... ) ESP_LOGW(pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)
#define LOG_TIMER_INFO(timerHandle, format, ... ) ESP_LOGI(pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)
#define LOG_TIMER_DEBUG(timerHandle, format, ... ) ESP_LOGD(pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)
#define LOG_TIMER_VERBOSE(timerHandle, format, ... ) ESP_LOGV(pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)

#define LOG_ERROR(format, ... ) ESP_LOGE(LOG_TAG, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ... ) ESP_LOGW(LOG_TAG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ... ) ESP_LOGI(LOG_TAG, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ... ) ESP_LOGD(LOG_TAG, format, ##__VA_ARGS__)
#define LOG_VERBOSE(format, ... ) ESP_LOGV(LOG_TAG, format, ##__VA_ARGS__)

#ifdef NDEBUG
#define FREERTOS_ERROR_CHECK(x) do {                                    \
        BaseType_t __err_rc = (x);                                      \
        (void) sizeof(__err_rc);                                        \
    } while(0)
#else
#define FREERTOS_ERROR_CHECK(x)  do {                                   \
        BaseType_t __err_rc = (x);                                      \
        if (__err_rc != pdPASS) {                                       \
            ESP_LOGE("FreeRTOS", "Error code %d returned from %s on line %d in %s",		\
									__err_rc,							\
									#x,									\
									__LINE__,							\
									__FILE__);							\
			abort();													\
        }                                                               \
    } while(0)
#endif

#endif /* MAIN_TASKUTILS_H_ */
