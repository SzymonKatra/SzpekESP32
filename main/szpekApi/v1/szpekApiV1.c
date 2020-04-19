/*
 * szpekApiV1.c
 *
 *  Created on: 4 kwi 2020
 *      Author: szymo
 */

#include "szpekApiV1.h"

#include <stdlib.h>
#include <esp_http_client.h>
#include <cJSON.h>
#include <cJSONExt.h>
#include <url_encoding.h>
#include <string.h>
#include "crypto.h"
#include "logUtils.h"
#include "settings.h"

static const char* LOG_TAG = "SzpekApiV1";

#define API_URL "https://api.szpek.pl"
//#define API_URL "http://192.168.0.151:5001"
#define API_PATH "/sensorApi/v1"

static void createAuthHeader(const unsigned char* data, size_t length, char result[128]);

static int startSignedGET(const char* endpoint, esp_http_client_handle_t* client);
static int performSignedPOST(const char* endpoint, const char* jsonData);
static size_t readResponseString(esp_http_client_handle_t client, char* buffer, size_t length);
static cJSON* readResponseJSON(esp_http_client_handle_t client);

bool szpekApiV1ReportSmog(const szpekApiV1ReportSmog_t* report)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "pm10Value", report->pm10Value);
	cJSON_AddNumberToObject(root, "pm2_5Value", report->pm2_5Value);
	cJSON_AddNumberToObject(root, "pm1Value", report->pm1Value);
	cJSON_AddNumberToObject(root, "samplesCount", report->samplesCount);
	cJSON_AddNumberToObject(root, "timestampFrom", report->timestampFrom);
	cJSON_AddNumberToObject(root, "timestampTo", report->timestampTo);
	char* jsonStr = cJSON_PrintBuffered(root, 512, false);
	cJSON_Delete(root);

	int status = performSignedPOST("/smog", jsonStr);
	free(jsonStr);

	return status == 200;
}

bool szpekApiV1ReportStartup(const char* firmwareName)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "firmwareName", firmwareName);
	char* jsonStr = cJSON_PrintBuffered(root, 128, false);
	cJSON_Delete(root);

	int status = performSignedPOST("/startup", jsonStr);
	free(jsonStr);

	return status == 200;
}

bool szpekApiV1Log(const char* message)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "message", message);
	char* jsonStr = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);

	int status = performSignedPOST("/log", jsonStr);
	free(jsonStr);

	return status == 200;
}

bool szpekApiV1GetRecommendedFirmwareMetadata(szpekApiV1FirmwareMetadata_t* result)
{
	bool success = true;

	esp_http_client_handle_t client;
	int status = startSignedGET("/firmware/recommended", &client);
	if (status == 200)
	{
		cJSON* json = readResponseJSON(client);
		success &= cJSONExt_TryReadString(json, "name", result->name);
		success &= cJSONExt_TryReadTimestamp(json, "releaseTimestamp", &result->releaseTimestamp);
		cJSON_Delete(json);
	}
	else
	{
		success = false;
	}

	esp_http_client_cleanup(client);
	return success;
}

bool szpekApiV1DownloadFirmwareBegin(const char* firmwareName, szpekApiV1FileDownloadState_t* state)
{
	char firmwareNameUrlEncoded[64];
	urlEncode(firmwareName, firmwareNameUrlEncoded, 64);
	char endpoint[128];
	snprintf(endpoint, 128, "/firmware/download/%s", firmwareNameUrlEncoded);

	int status = startSignedGET(endpoint, &state->httpClientHandle);
	state->bytesRead = 0;

	return status == 200;
}

int szpekApiV1DownloadFirmwareRead(szpekApiV1FileDownloadState_t* state, unsigned char* buffer, size_t length)
{
	int bytesRead = esp_http_client_read(state->httpClientHandle, (char*)buffer, length);
	if (bytesRead > 0)
	{
		state->bytesRead += bytesRead;
	}

	return bytesRead;
}

void szpekApiV1DownloadFirmwareEnd(szpekApiV1FileDownloadState_t* state)
{
	esp_http_client_cleanup(state->httpClientHandle);
}

static void createAuthHeader(const unsigned char* data, size_t length, char result[128])
{
	char base64Signature[CRYPTO_BASE64_BUFFER_LEN(32)];
	cryptoSignatureBase64(data, length, base64Signature);

	char sensorCodeEncoded[48];
	urlEncode(settingsGetSzpekId()->code, sensorCodeEncoded, 48);

	snprintf(result, 128, "SZPEK-HMAC-SHA256 %s:%s", sensorCodeEncoded, base64Signature);
}

static int startSignedGET(const char* endpoint, esp_http_client_handle_t* client)
{
	char url[128];
	snprintf(url, 128, "%s%s%s", "GET", API_PATH, endpoint);

	char authHeader[128];
	createAuthHeader((const unsigned char*)url, strlen(url), authHeader);

	snprintf(url, 128, "%s%s%s", API_URL, API_PATH, endpoint);

	esp_http_client_config_t config = { .url = url, .method = HTTP_METHOD_GET };
	*client = esp_http_client_init(&config);
	esp_http_client_set_header(*client, "Authorization", authHeader);

	LOG_INFO("Performing HTTP GET to %s ...", config.url);

	esp_err_t err = esp_http_client_open(*client, 0);
	if (err == ESP_OK)
	{
		if (esp_http_client_fetch_headers(*client) < 0)
		{
			err = ESP_FAIL;
		}
	}
	int status;
	if (err == ESP_OK)
	{
		status = esp_http_client_get_status_code(*client);
		LOG_INFO("Successful HTTP GET to %s, status = %d", config.url, status);
	}
	else
	{
		LOG_ERROR("Failed HTTP GET to %s failed: %d/%s", config.url, err, esp_err_to_name(err));
		status = -1;
	}

	return status;
}

static int performSignedPOST(const char* endpoint, const char* jsonData)
{
	size_t jsonDataLen = strlen(jsonData);

	char url[128];
	snprintf(url, 128, "%s%s%s", API_URL, API_PATH, endpoint);

	char authHeader[128];
	createAuthHeader((const unsigned char*)jsonData, strlen(jsonData), authHeader);

	esp_http_client_config_t config = { .url = url, .method = HTTP_METHOD_POST };
	esp_http_client_handle_t client = esp_http_client_init(&config);
	esp_http_client_set_header(client, "Authorization", authHeader);
	esp_http_client_set_header(client, "Content-Type", "application/json");
	esp_http_client_set_post_field(client, jsonData, jsonDataLen);

	LOG_INFO("Performing HTTP POST to %s ...", config.url);
	esp_err_t err = esp_http_client_perform(client);
	int status;
	if (err == ESP_OK)
	{
		status = esp_http_client_get_status_code(client);
		LOG_INFO("Successful HTTP POST to %s, status = %d", config.url, status);
	}
	else
	{
		LOG_ERROR("Failed HTTP POST to %s failed: %s", config.url, esp_err_to_name(err));
		status = -1;
	}

	esp_http_client_cleanup(client);

	return status;
}

static size_t readResponseString(esp_http_client_handle_t client, char* buffer, size_t length)
{
	length--; // leave space for null-terminator

	size_t pos = 0;
	size_t remaining = length - pos;
	int bytesRead;
	do
	{
		bytesRead = esp_http_client_read(client, buffer + pos, remaining);
		if (bytesRead >= 0)
		{
			pos += bytesRead;
			remaining = length - pos;
		}
	}
	while (remaining > 0 && bytesRead > 0);

	buffer[pos] = 0;

	return pos;
}

static cJSON* readResponseJSON(esp_http_client_handle_t client)
{
	char* buffer = malloc(512);
	readResponseString(client, buffer, 512);
	cJSON* json = cJSON_Parse((char*)buffer);
	free(buffer);
	return json;
}
