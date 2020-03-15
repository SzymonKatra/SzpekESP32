#include "network.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "logUtils.h"
#include "app.h"

ESP_EVENT_DEFINE_BASE(NETWORK_EVENT);

static const char* LOG_TAG = "Network";

static const size_t WIFI_SSID_MAX_LENGTH = 31;
static const size_t WIFI_PASSWORD_MAX_LENGTH = 63;

static const EventBits_t NETWORK_ESTABLISHED_BIT = BIT0;
static EventGroupHandle_t s_eventGroup;

static void eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void setEstablished();
static void clearEstablished();

void networkInit()
{
    s_eventGroup = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &eventHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &eventHandler, NULL));

    LOG_INFO("Initialization finished");
}

networkError_t networkSTAConnection(const char* ssid, const char* password)
{
	if (strlen(ssid) > WIFI_SSID_MAX_LENGTH)
	{
		LOG_ERROR("SSID too long, limit is %d: %s", WIFI_SSID_MAX_LENGTH, ssid);
		return NETWORK_ERR_TOO_LONG_SSID;
	}
	if (strlen(password) > WIFI_PASSWORD_MAX_LENGTH)
	{
		LOG_ERROR("Password too long, limit is %d: %s", WIFI_PASSWORD_MAX_LENGTH, password);
		return NETWORK_ERR_TOO_LONG_SSID;
	}

	wifi_config_t wifiCfg;
	memset(&wifiCfg, 0, sizeof(wifi_config_t));
	strcpy((char*)wifiCfg.sta.ssid, ssid);
	strcpy((char*)wifiCfg.sta.password, password);

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiCfg));
	ESP_ERROR_CHECK(esp_wifi_start());

	return NETWORK_OK;
}

void networkWaitForEstablish()
{
	xEventGroupWaitBits(s_eventGroup, NETWORK_ESTABLISHED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
}

bool networkIsEstablished()
{
	return xEventGroupGetBits(s_eventGroup) & NETWORK_ESTABLISHED_BIT;
}

static void eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT)
	{
		switch (event_id)
		{
		case WIFI_EVENT_STA_START:
			LOG_INFO("Connecting to the AP...");
			esp_wifi_connect();
			break;

		case WIFI_EVENT_STA_CONNECTED:
			LOG_INFO("Connected to the AP successfully! Waiting for an IP...");
			break;

		case WIFI_EVENT_STA_DISCONNECTED:
			clearEstablished();
			LOG_INFO("Disconnected");
			LOG_INFO("Connecting to the AP...");
			esp_wifi_connect();
			break;
		}
	}
	else if (event_base == IP_EVENT)
	{
		if (event_id == IP_EVENT_STA_GOT_IP)
		{
			ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
			LOG_INFO("Got IP: %s", ip4addr_ntoa(&event->ip_info.ip));
			setEstablished();
		}
		else if (event_id == IP_EVENT_STA_LOST_IP)
		{
			LOG_INFO("Lost IP");
			clearEstablished();
		}
	}
}

static void setEstablished()
{
	if (!networkIsEstablished())
	{
		xEventGroupSetBits(s_eventGroup, NETWORK_ESTABLISHED_BIT);
		esp_event_post_to(appGetEventLoopHandle(), NETWORK_EVENT, NETWORK_EVENT_CONNECTION_ESTABLISHED, NULL, 0, pdMS_TO_TICKS(1000));
	}
}

static void clearEstablished()
{
	if (networkIsEstablished())
	{
		xEventGroupClearBits(s_eventGroup, NETWORK_ESTABLISHED_BIT);
		esp_event_post_to(appGetEventLoopHandle(), NETWORK_EVENT, NETWORK_EVENT_CONNECTION_LOST, NULL, 0, pdMS_TO_TICKS(1000));
	}
}
