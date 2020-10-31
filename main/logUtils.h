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
#include "settings.h"
#include "reports.h"
#include "app.h"

void enqueueRemoteLog(const char* log);

#define _LOGUTILS_LOG_FORMAT(letter, format) #letter " (%d) %s: " format

#define _LOGUTILS_LOG(level, tag, format, ...) do {               \
        if ( LOG_LOCAL_LEVEL >= level ) { \
        	ESP_LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
        	if ( settingsGetRemoteLogLevel() >= level) { \
        		char log[APP_REMOTE_LOG_LENGTH]; \
        		if (level==ESP_LOG_ERROR )          { snprintf(log, APP_REMOTE_LOG_LENGTH, _LOGUTILS_LOG_FORMAT(E, format), esp_log_timestamp(), tag, ##__VA_ARGS__); } \
				else if (level==ESP_LOG_WARN )      { snprintf(log, APP_REMOTE_LOG_LENGTH, _LOGUTILS_LOG_FORMAT(W, format), esp_log_timestamp(), tag, ##__VA_ARGS__); } \
				else if (level==ESP_LOG_DEBUG )     { snprintf(log, APP_REMOTE_LOG_LENGTH, _LOGUTILS_LOG_FORMAT(D, format), esp_log_timestamp(), tag, ##__VA_ARGS__); } \
				else if (level==ESP_LOG_VERBOSE )   { snprintf(log, APP_REMOTE_LOG_LENGTH, _LOGUTILS_LOG_FORMAT(V, format), esp_log_timestamp(), tag, ##__VA_ARGS__); } \
				else if (level==ESP_LOG_INFO )      { snprintf(log, APP_REMOTE_LOG_LENGTH, _LOGUTILS_LOG_FORMAT(I, format), esp_log_timestamp(), tag, ##__VA_ARGS__); } \
				enqueueRemoteLog(log); \
			} \
        } \
    } while(0)

#define LOG_TASK_ERROR(format, ... ) _LOGUTILS_LOG(ESP_LOG_ERROR, pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_WARNING(format, ... ) _LOGUTILS_LOG(ESP_LOG_WARN, pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_INFO(format, ... ) _LOGUTILS_LOG(ESP_LOG_INFO, pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_DEBUG(format, ... ) _LOGUTILS_LOG(ESP_LOG_DEBUG, pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_VERBOSE(format, ... ) _LOGUTILS_LOG(ESP_LOG_VERBOSE, pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)

#define LOG_TIMER_ERROR(timerHandle, format, ... ) _LOGUTILS_LOG(ESP_LOG_ERROR, pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)
#define LOG_TIMER_WARNING(timerHandle, format, ... ) _LOGUTILS_LOG(ESP_LOG_WARN, pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)
#define LOG_TIMER_INFO(timerHandle, format, ... ) _LOGUTILS_LOG(ESP_LOG_INFO, pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)
#define LOG_TIMER_DEBUG(timerHandle, format, ... ) _LOGUTILS_LOG(ESP_LOG_DEBUG, pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)
#define LOG_TIMER_VERBOSE(timerHandle, format, ... ) _LOGUTILS_LOG(ESP_LOG_VERBOSE, pcTimerGetTimerName(timerHandle), format, ##__VA_ARGS__)

#define LOG_ERROR(format, ... ) _LOGUTILS_LOG(ESP_LOG_ERROR, LOG_TAG, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ... ) _LOGUTILS_LOG(ESP_LOG_WARN, LOG_TAG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ... ) _LOGUTILS_LOG(ESP_LOG_INFO, LOG_TAG, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ... ) _LOGUTILS_LOG(ESP_LOG_DEBUG, LOG_TAG, format, ##__VA_ARGS__)
#define LOG_VERBOSE(format, ... ) _LOGUTILS_LOG(ESP_LOG_VERBOSE, LOG_TAG, format, ##__VA_ARGS__)

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
