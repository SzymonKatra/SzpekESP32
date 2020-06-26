#include "app.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include <esp_event.h>
#include <esp_sntp.h>
#include <driver/gpio.h>
#include <esp_ota_ops.h>
#include "network.h"
#include "logUtils.h"
#include "tasks/tasks.h"
#include "timers/timers.h"
#include "reports.h"
#include "smog.h"
#include "weather.h"
#include "settings.h"
#include "crypto.h"
#include "configServer.h"
#include <string.h>

#define APP_INIT_PRIORITY (configTIMER_TASK_PRIORITY + 1)
#define APP_EVENT_LOOP_PRIORITY (configTIMER_TASK_PRIORITY - 1)

static const char* LOG_TAG = "App";

static esp_event_loop_handle_t s_appEventLoopHandle;

static appMode_t s_appMode;
static appITCStructures_t s_itcStructures;
static appTasksList_t s_tasks;
static appTimersList_t s_timers;

#define ledNetwork(value) gpio_set_level(APP_LED_GREEN_GPIO, value)
#define ledConfig(value) gpio_set_level(APP_LED_RED_GPIO, value)
#define ledOrange(value) gpio_set_level(APP_LED_ORANGE_GPIO, value)

static void createAppEventLoop();
static void setupTimeSync();

static void createITCStructures();
static void createTimers();
static void createTasks();

static void networkEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void onTimeSynced(struct timeval *tv);

void appInit()
{
	vTaskPrioritySet(NULL, APP_INIT_PRIORITY);
	configASSERT(uxTaskPriorityGet(NULL) == APP_INIT_PRIORITY);

	LOG_INFO("Firmware name: %s", APP_FIRMWARE_NAME);

	ledNetwork(0);
	ledConfig(0);
	ledOrange(1);

	createAppEventLoop();

	appRegisterEventHandler(NETWORK_EVENT, ESP_EVENT_ANY_ID, networkEventHandler, NULL);

	settingsInit();
	cryptoInit(settingsGetSzpekId()->secretBase64);
	setupTimeSync();
	networkInit();
	smogInit();
	weatherInit();

	createITCStructures();
	createTimers();
	createTasks();

	appChangeMode(APP_MODE_RUNNING);
}

appMode_t appGetCurrentMode()
{
	return s_appMode;
}

void appChangeMode(appMode_t appMode)
{
	if (s_appMode == appMode) return;

	const char* modeStr = "";
	switch (appMode)
	{
	case APP_MODE_IDLE: modeStr = "idle"; break;
	case APP_MODE_RUNNING: modeStr = "running"; break;
	case APP_MODE_CONFIG: modeStr = "config"; break;
	}
	LOG_INFO("Changing app mode to %s ...", modeStr);

	s_appMode = appMode;

	if (appMode == APP_MODE_IDLE)
	{
		networkStop();
	}
	else if (appMode == APP_MODE_RUNNING)
	{
		const settingsWifi_t* wifi = settingsGetWifi();
		networkSTAConnection(wifi->ssid, wifi->password);
	}
	else if (appMode == APP_MODE_CONFIG)
	{
		const settingsSzpekId_t* szpekId = settingsGetSzpekId();
		char ssidBuffer[64];
		sprintf(ssidBuffer, "szpek-%s", szpekId->code);
		networkHotspot(ssidBuffer, "szpekadmin");
	}
}

void appFirmwareTriggerUpdateCheck()
{
	xTaskNotifyGive(s_tasks.firmwareOTA);
}

void appFirmwareApply()
{
	LOG_INFO("Applying current firmware (%s) as boot firmware...", APP_FIRMWARE_NAME);
	esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
	if (err == ESP_OK)
	{
		LOG_INFO("Current firmware (%s) applied as boot firmware!", APP_FIRMWARE_NAME);
	}
	else
	{
		LOG_ERROR("An error occurred while applying current firmware (%s) as boot! Error code = %d", APP_FIRMWARE_NAME, err);
	}
}

esp_event_loop_handle_t appGetEventLoopHandle()
{
	return s_appEventLoopHandle;
}

const appITCStructures_t* appGetITCStructures()
{
	return &s_itcStructures;
}

const appTasksList_t* appGetTasks()
{
	return &s_tasks;
}

void appRegisterEventHandler(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void* event_handler_arg)
{
	ESP_ERROR_CHECK(esp_event_handler_register_with(s_appEventLoopHandle, event_base, event_id, event_handler, event_handler_arg));
}

static void createAppEventLoop()
{
	esp_event_loop_args_t appEventLoopArgs = {
		.queue_size = 10,
		.task_name = "AppEventLoop",
		.task_priority = APP_EVENT_LOOP_PRIORITY,
		.task_stack_size = 2048,
		.task_core_id = tskNO_AFFINITY
	};

	ESP_ERROR_CHECK(esp_event_loop_create(&appEventLoopArgs, &s_appEventLoopHandle));
}

static void setupTimeSync()
{
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_set_time_sync_notification_cb(onTimeSynced);
	sntp_init();
}

static void createITCStructures()
{
	LOG_INFO("Creating ITC structures...");

	s_itcStructures.reportMeasurementsQueue = xQueueCreate(24, sizeof(reportMeasurements_t));
	configASSERT(s_itcStructures.reportMeasurementsQueue);

	LOG_INFO("ITC structures created successfully!");
}

static void createTimers()
{
	LOG_INFO("Creating software timers...");

	s_timers.timeEvents = xTimerCreate("TimeEvents", pdMS_TO_TICKS(100), pdTRUE, 0, timerTimeTriggersCallback);
	configASSERT(s_timers.timeEvents);

	LOG_INFO("Software timers created successfully!");
}

static void createTasks()
{
	configASSERT(uxTaskPriorityGet(NULL) == APP_INIT_PRIORITY);
	LOG_INFO("Creating tasks...");

	FREERTOS_ERROR_CHECK(xTaskCreate(taskCheckButton, "CheckButton", 2048, NULL, 6, &s_tasks.checkButton));
	configASSERT(s_tasks.checkButton);

	FREERTOS_ERROR_CHECK(xTaskCreate(taskSmogSensor, "SmogSensor", 2048, NULL, 7, &s_tasks.smogSensor));
	configASSERT(s_tasks.smogSensor);

	FREERTOS_ERROR_CHECK(xTaskCreate(taskAggregateData, "AggregateData", 2048, NULL, 5, &s_tasks.aggregateData));
	configASSERT(s_tasks.aggregateData);
	vTaskSuspend(s_tasks.aggregateData);
	configASSERT(eTaskGetState(s_tasks.aggregateData) == eSuspended);

	FREERTOS_ERROR_CHECK(xTaskCreate(taskPushReports, "PushReports", 8192, NULL, 2, &s_tasks.pushReports));
	configASSERT(s_tasks.pushReports);
	vTaskSuspend(s_tasks.pushReports);
	//configASSERT(eTaskGetState(s_tasks.pushReports) == eSuspended); // INVESTIGATE: suspended task pre-empted and not suspended??? maybe other tasks are preempting?

	FREERTOS_ERROR_CHECK(xTaskCreate(taskFirmwareOTA, "FirmwareOTA", 8192, NULL, 1, &s_tasks.firmwareOTA));
	configASSERT(s_tasks.firmwareOTA);

	FREERTOS_ERROR_CHECK(xTaskCreate(taskWeatherSensor, "WeatherSensor", 2048, NULL, 7, &s_tasks.weatherSensor));
	configASSERT(s_tasks.weatherSensor);

	LOG_INFO("Tasks created successfully!");
}

static void networkEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_id == NETWORK_EVENT_CONNECTION_ESTABLISHED)
	{
		LOG_INFO("Network connection established");
		appFirmwareTriggerUpdateCheck();
		vTaskResume(s_tasks.pushReports);
		ledNetwork(1);
	}
	else if (event_id == NETWORK_EVENT_CONNECTION_LOST)
	{
		LOG_INFO("Network connection lost");
		vTaskSuspend(s_tasks.pushReports);
		ledNetwork(0);
	}
	else if (event_id == NETWORK_EVENT_HOTSPOT_STARTED)
	{
		LOG_INFO("Hotspot started");
		configServerStart();
		ledConfig(1);
	}
	else if (event_id == NETWORK_EVENT_HOTSPOT_STOPPED)
	{
		LOG_INFO("Hotspot stopped");
		configServerStop();
		ledConfig(0);
	}
}

static void onTimeSynced(struct timeval *tv)
{
	LOG_INFO("Time synchronized: %ld", tv->tv_sec);
	if (xTimerIsTimerActive(s_timers.timeEvents) == pdFALSE)
	{
		timerTimeTriggersInit();
		xTimerStart(s_timers.timeEvents, 0);

		vTaskResume(s_tasks.aggregateData); // resume only once, todo: maybe create task here instead of resuming?
	}
}
