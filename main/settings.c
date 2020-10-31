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
#include <cJSONExt.h>
#include <nvs.h>
#include "logUtils.h"

static const char* LOG_TAG = "Settings";

static settingsSzpekId_t s_szpekId;
static bool s_wifiValid;
static settingsWifi_t s_wifi;

static void loadSzpekId();
static void loadWifi(nvs_handle_t nvsHandle);

static bool nvsGetStrSafe(nvs_handle_t nvsHandle, const char* key, char* outValue, size_t maxLength);

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
		LOG_ERROR("Error (%s) opening NVS handle for settings!", esp_err_to_name(err));
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
		ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_set_str(nvsHandle, "wifi_ssid", s_wifi.ssid));
		ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_set_str(nvsHandle, "wifi_password", s_wifi.password));
		ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_commit(nvsHandle));
		nvs_close(nvsHandle);
	}
	else
	{
		LOG_ERROR("An error occurred while opening NVS handle (%s)!", esp_err_to_name(err));
	}
}

esp_log_level_t settingsGetRemoteLogLevel()
{
	return ESP_LOG_WARN;
}

static void loadSzpekId()
{
	nvs_handle_t nvsHandle;
	esp_err_t err = nvs_open("szpekid", NVS_READONLY, &nvsHandle);
	if (err != ESP_OK)
	{
		LOG_ERROR("Error (%s) opening NVS handle for szpekid!", esp_err_to_name(err));
	}

	nvsGetStrSafe(nvsHandle, "code", s_szpekId.code, sizeof(s_szpekId.code));
	nvsGetStrSafe(nvsHandle, "secretBase64", s_szpekId.secretBase64, sizeof(s_szpekId.secretBase64));
	nvsGetStrSafe(nvsHandle, "board", s_szpekId.board, sizeof(s_szpekId.board));
	nvsGetStrSafe(nvsHandle, "model", s_szpekId.model, sizeof(s_szpekId.model));

	LOG_INFO("SzpekID Code = %s", s_szpekId.code);
	LOG_INFO("SzpekID SecretBase64 = %s", s_szpekId.secretBase64);
	LOG_INFO("SzpekID Board = %s", s_szpekId.board);
	LOG_INFO("SzpekID Model = %s", s_szpekId.model);

	nvs_close(nvsHandle);
}

static void loadWifi(nvs_handle_t nvsHandle)
{
	size_t lenSsid = NETWORK_SSID_SIZE;
	esp_err_t eSsid = nvs_get_str(nvsHandle, "wifi_ssid", s_wifi.ssid, &lenSsid);
	if (eSsid != ESP_OK || eSsid != ESP_ERR_NVS_NOT_FOUND)
	{
		ESP_ERROR_CHECK_WITHOUT_ABORT(eSsid);
	}

	size_t lenPass = NETWORK_PASSWORD_SIZE;
	esp_err_t ePass = nvs_get_str(nvsHandle, "wifi_password", s_wifi.password, &lenPass);
	if (ePass != ESP_OK || ePass != ESP_ERR_NVS_NOT_FOUND)
	{
		ESP_ERROR_CHECK_WITHOUT_ABORT(eSsid);
	}

	s_wifiValid = eSsid == ESP_OK && ePass == ESP_OK;
}

static bool nvsGetStrSafe(nvs_handle_t nvsHandle, const char* key, char* outValue, size_t maxLength)
{
	esp_err_t err = nvs_get_str(nvsHandle, key, outValue, &maxLength);
	if (err != ESP_OK)
	{
		if (err == ESP_ERR_NVS_NOT_FOUND)
		{
			LOG_ERROR("Key %s not found!", key);
		}
		else
		{
			LOG_ERROR("Unknown error occured while reading NVS (%d)/(%s)", err, esp_err_to_name(err));
		}

		outValue = "";
		return false;
	}

	return true;
}
