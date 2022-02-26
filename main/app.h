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
#include <freertos/timers.h>
#include <freertos/queue.h>
#include <driver/uart.h>
#include <driver/i2c.h>
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
	TaskHandle_t firmwareOTA;
	TaskHandle_t weatherSensor;
	TaskHandle_t logs;
} appTasksList_t;

typedef struct
{
	TimerHandle_t timeEvents;
} appTimersList_t;

typedef struct
{
	QueueHandle_t reportMeasurementsQueue;
} appITCStructures_t;

//#define APP_FIRMWARE_NAME "esp32_27-02-2022"
#define APP_FIRMWARE_NAME "esp32_dev"

#define APP_PMS_UART_PORT 	UART_NUM_2
#define APP_I2C_MASTER		I2C_NUM_0
#define APP_LED_GREEN_GPIO 	GPIO_NUM_32
#define APP_LED_ORANGE_GPIO GPIO_NUM_33
#define APP_LED_RED_GPIO 	GPIO_NUM_25
#define APP_BUTTON_GPIO 	GPIO_NUM_26

#define APP_REMOTE_LOG_LENGTH 128

void appInit();

appMode_t appGetCurrentMode();
void appChangeMode(appMode_t appMode);
void appFirmwareTriggerUpdateCheck();
void appFirmwareApply();

esp_event_loop_handle_t appGetEventLoopHandle();
const appITCStructures_t* appGetITCStructures();
const appTasksList_t* appGetTasks();

QueueHandle_t appGetLogsQueue();

void appRegisterEventHandler(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg);

#endif /* MAIN_APP_H_ */
