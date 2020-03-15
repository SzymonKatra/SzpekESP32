#include "app.h"

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_event.h>
#include <esp_sntp.h>
#include "network.h"
#include "logUtils.h"

static const char* LOG_TAG = "App";

static esp_event_loop_handle_t s_appEventLoopHandle;

static void setupAppEventLoop();
static void setupTimeSync();

static void createBasicTasks();
static void createNormalTasks();

static void networkEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void onTimeSynced(struct timeval *tv);

void appInit()
{
	setupAppEventLoop();
	setupTimeSync();

	networkInit();
	networkSTAConnection("UPC6133978", "Z8bbzyb6sawm");

	createBasicTasks();
}

esp_event_loop_handle_t appGetEventLoopHandle()
{
	return s_appEventLoopHandle;
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

static void createBasicTasks()
{
	LOG_INFO("Creating basic tasks...");
}

static void createNormalTasks()
{
	LOG_INFO("Creating normal tasks...");
}

static void networkEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_id == NETWORK_EVENT_CONNECTION_ESTABLISHED)
	{
		LOG_INFO("Network connection established");
	}
	else if (event_id == NETWORK_EVENT_CONNECTION_LOST)
	{
		LOG_INFO("Network connection lost");
	}
}

static void onTimeSynced(struct timeval *tv)
{
	LOG_INFO("Time synchronized: %ld", tv->tv_sec);
	createNormalTasks();
}
