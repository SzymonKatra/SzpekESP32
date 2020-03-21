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
#include <driver/gpio.h>
#include <esp_event.h>

typedef enum
{
	APP_MODE_IDLE,
	APP_MODE_RUNNING,
	APP_MODE_CONFIG
} appMode_t;

typedef struct
{
	TaskHandle_t smogSensor;
	TaskHandle_t aggregateData;
	TaskHandle_t pushReports;
	TaskHandle_t checkButton;
} appTasksList_t;

typedef struct
{
	QueueHandle_t reportsQueue;
} appITCStructures_t;

extern const uart_port_t APP_PMS_UART_PORT;
extern const gpio_num_t APP_NETWORK_LED_GPIO;
extern const gpio_num_t APP_CONFIG_LED_GPIO;
extern const gpio_num_t APP_CONFIG_BUTTON_GPIO;

void appInit();

appMode_t appGetCurrentMode();
void appChangeMode(appMode_t appMode);

esp_event_loop_handle_t appGetEventLoopHandle();
const appITCStructures_t* appGetITCStructures();

#endif /* MAIN_APP_H_ */
