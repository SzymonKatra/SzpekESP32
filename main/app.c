#include "app.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include <esp_event.h>
#include <esp_sntp.h>
#include "network.h"
#include "logUtils.h"
#include "tasks/tasks.h"
#include "reports.h"
#include "smog.h"
#include "settings.h"
#include "crypto.h"

static const char* LOG_TAG = "App";

const uart_port_t APP_PMS_UART_PORT = UART_NUM_2;

static esp_event_loop_handle_t s_appEventLoopHandle;

static appITCStructures_t s_itcStructures;
static appTasksList_t s_tasks;

static void setupAppEventLoop();
static void setupTimeSync();

static void createITCStructures();
static void createTasks();

static void networkEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void onTimeSynced(struct timeval *tv);

void appInit()
{
	vTaskPrioritySet(NULL, 10);
	assert(uxTaskPriorityGet(NULL) == 10);

	settingsInit();
	cryptoInit(settingsGetSensorBase64Secret());

	setupAppEventLoop();
	setupTimeSync();

	networkInit();
	networkSTAConnection("UPC6133978", "Z8bbzyb6sawm");

	smogInit();

	createITCStructures();
	createTasks();
}

esp_event_loop_handle_t appGetEventLoopHandle()
{
	return s_appEventLoopHandle;
}

const appITCStructures_t* appGetITCStructures()
{
	return &s_itcStructures;
}

static void setupAppEventLoop()
{
	esp_event_loop_args_t appEventLoopArgs = {
		.queue_size = 10,
		.task_name = "AppEventLoop",
		.task_priority = 2,
		.task_stack_size = 2048,
		.task_core_id = tskNO_AFFINITY
	};

	ESP_ERROR_CHECK(esp_event_loop_create(&appEventLoopArgs, &s_appEventLoopHandle));
	ESP_ERROR_CHECK(esp_event_handler_register_with(s_appEventLoopHandle, NETWORK_EVENT, ESP_EVENT_ANY_ID, networkEventHandler, NULL));
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

	s_itcStructures.reportsQueue = xQueueCreate(24, sizeof(report_t));
	configASSERT(s_itcStructures.reportsQueue);

	LOG_INFO("ITC structures created successfully!");
}

static void createTasks()
{
	LOG_INFO("Creating tasks...");

	FREERTOS_ERROR_CHECK(xTaskCreate(taskSmogSensor, "SmogSensor", 2048, NULL, 7, &s_tasks.smogSensor));
	FREERTOS_ERROR_CHECK(xTaskCreate(taskAggregateData, "AggregateData", 2048, NULL, 5, &s_tasks.aggregateData));
	vTaskSuspend(s_tasks.aggregateData);
	FREERTOS_ERROR_CHECK(xTaskCreate(taskPushReports, "PushReports", 8192, NULL, 1, &s_tasks.pushReports));
	vTaskSuspend(s_tasks.pushReports);

	LOG_INFO("Tasks created successfully!");
}

static void networkEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_id == NETWORK_EVENT_CONNECTION_ESTABLISHED)
	{
		LOG_INFO("Network connection established");
		vTaskResume(s_tasks.pushReports);
	}
	else if (event_id == NETWORK_EVENT_CONNECTION_LOST)
	{
		LOG_INFO("Network connection lost");
		vTaskSuspend(s_tasks.pushReports);
	}
}

static void onTimeSynced(struct timeval *tv)
{
	LOG_INFO("Time synchronized: %ld", tv->tv_sec);
	vTaskResume(s_tasks.aggregateData);
}
