/*
 * settings.c
 *
 *  Created on: 18 mar 2020
 *      Author: szymo
 */

#include "settings.h"

#include <string.h>
#include <esp_partition.h>
#include <cJSON.h>
#include <nvs.h>
#include "logUtils.h"

static const char* LOG_TAG = "Settings";

static settingsSzpekId_t s_szpekId;
static bool s_wifiValid;
static settingsWifi_t s_wifi;

static void loadSzpekId();
static void loadWifi(nvs_handle_t nvsHandle);

void settingsInit()
{
	s_wifiValid = false;

	loadSzpekId();

	nvs_handle_t nvsHandle;
	esp_err_t err = nvs_open("settings", NVS_READONLY, &nvsHandle);
	if (err == ESP_OK)
	{
		loadWifi(nvsHandle);
		nvs_close(nvsHandle);
	}
	else
	{
		LOG_ERROR("Error (%s) opening NVS handle!", esp_err_to_name(err));
	}
}

const settingsSzpekId_t* settingsGetSzpekId()
{
	return &s_szpekId;
}

bool settingsIsWifiValid()
{
	return s_wifiValid;
}

const settingsWifi_t* settingsGetWifi()
{
	return &s_wifi;
}

void settingsSetWifi(const settingsWifi_t* wifiSettings)
{
	s_wifi = *wifiSettings;
	nvs_handle_t nvsHandle;
	esp_err_t err = nvs_open("settings", NVS_READWRITE, &nvsHandle);
	if (err == ESP_OK)
	{
		nvs_set_str(nvsHandle, "wifi_ssid", s_wifi.ssid);
		nvs_set_str(nvsHandle, "wifi_password", s_wifi.password);
		err = nvs_commit(nvsHandle);
		if (err != ESP_OK)
		{
			LOG_ERROR("An error occured while commiting NVS changes (%s)!", esp_err_to_name(err));
		}
		nvs_close(nvsHandle);
	}
	else
	{
		LOG_ERROR("An error occurred while opening NVS handle (%s)!", esp_err_to_name(err));
	}
}

static void loadSzpekId()
{
	const esp_partition_type_t SZPEKID_PARTITION_TYPE = 0x40;
	const esp_partition_subtype_t SZPEKID_PARTITION_SUBTYPE = 0;
	const char* SZPEKID_PARTITION_LABEL = "szpekid";
	const size_t SZPEKID_MAX_SIZE = 512; // partition is bigger but we don't need that

	LOG_INFO("Loading SzpekID from (%s, %d, %d)...", SZPEKID_PARTITION_LABEL, SZPEKID_PARTITION_TYPE, SZPEKID_PARTITION_SUBTYPE);

	const esp_partition_t* szpekIdPartition = esp_partition_find_first(SZPEKID_PARTITION_TYPE, SZPEKID_PARTITION_SUBTYPE, SZPEKID_PARTITION_LABEL);
	char* buffer = malloc(SZPEKID_MAX_SIZE);
	esp_partition_read(szpekIdPartition, 0, buffer, SZPEKID_MAX_SIZE);

	cJSON* szpekid = cJSON_Parse(buffer);
	cJSON* szpek_code = cJSON_GetObjectItemCaseSensitive(szpekid, "code");
	strcpy(s_szpekId.code, szpek_code->valuestring);
	cJSON* szpek_secret = cJSON_GetObjectItemCaseSensitive(szpekid, "secret");
	strcpy(s_szpekId.secretBase64, szpek_secret->valuestring);
	cJSON_Delete(szpekid);

	LOG_INFO("SzpekID Code = %s", s_szpekId.code);
	LOG_INFO("SzpekID Secret = %s", s_szpekId.secretBase64);

	free(buffer);

	LOG_INFO("SzpekID loaded!");
}

static void loadWifi(nvs_handle_t nvsHandle)
{
	size_t len;
	esp_err_t eWifi = nvs_get_str(nvsHandle, "wifi_ssid", s_wifi.ssid, &len);
	esp_err_t ePass = nvs_get_str(nvsHandle, "wifi_password", s_wifi.password, &len);

	s_wifiValid = eWifi == ESP_OK && ePass == ESP_OK;
}
