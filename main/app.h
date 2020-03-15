/*
 * app.h
 *
 *  Created on: 14 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_APP_H_
#define MAIN_APP_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/uart.h>
#include <esp_event.h>

typedef struct
{
	TaskHandle_t smogSensor;
	TaskHandle_t aggregateData;
	TaskHandle_t pushReports;
} appTasksList_t;

typedef struct
{
	QueueHandle_t reportsQueue;
} appITCStructures_t;

extern const uart_port_t APP_PMS_UART_PORT;

void appInit();

esp_event_loop_handle_t appGetEventLoopHandle();
const appITCStructures_t* appGetITCStructures();

#endif /* MAIN_APP_H_ */
