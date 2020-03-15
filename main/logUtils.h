/*
 * utils.h
 *
 *  Created on: 14 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_TASKUTILS_H_
#define MAIN_TASKUTILS_H_


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#define LOG_TASK_ERROR(format, ... ) ESP_LOGE(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_WARNING(format, ... ) ESP_LOGW(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_INFO(format, ... ) ESP_LOGI(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_DEBUG(format, ... ) ESP_LOGD(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)
#define LOG_TASK_VERBOSE(format, ... ) ESP_LOGV(pcTaskGetTaskName(NULL), format, ##__VA_ARGS__)

#define LOG_ERROR(format, ... ) ESP_LOGE(LOG_TAG, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ... ) ESP_LOGW(LOG_TAG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ... ) ESP_LOGI(LOG_TAG, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ... ) ESP_LOGD(LOG_TAG, format, ##__VA_ARGS__)
#define LOG_VERBOSE(format, ... ) ESP_LOGV(LOG_TAG, format, ##__VA_ARGS__)

#endif /* MAIN_TASKUTILS_H_ */
