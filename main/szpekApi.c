/*
 * szpekApi.c
 *
 *  Created on: 18 mar 2020
 *      Author: szymo
 */

#include "szpekApi.h"

#include <stdlib.h>
#include <mbedtls/base64.h>
#include <esp_http_client.h>
#include <cJSON.h>
#include "crypto.h"
#include <string.h>
#include "logUtils.h"

static const char* LOG_TAG = "SzpekApi";

#define SZPEKAPI_BASE_URL "https://api.szpek.pl/api"

static void szpekApiJSONPack(const char* data, size_t length, char* result);
static void cJSONHelper_AddDateTimeToObject(cJSON* const object, const char* const name, const time_t timestamp);

bool szpekApiSensorsMicro(const szpekApiSensorsMicroData_t* data)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "pm1Value", data->pm1Value);
	cJSON_AddNumberToObject(root, "pm25Value", data->pm25Value);
	cJSON_AddNumberToObject(root, "pm10Value", data->pm10Value);
	cJSON_AddNumberToObject(root, "samplesQuantity", data->samplesQuantity);
	cJSONHelper_AddDateTimeToObject(root, "periodFrom", data->periodFrom);
	cJSONHelper_AddDateTimeToObject(root, "periodTo", data->periodTo);
	cJSON_AddStringToObject(root, "sensorCode", data->sensorCode);
	char* jsonStr = cJSON_PrintBuffered(root, 512, false);
	cJSON_Delete(root);

	char* jsonPack = malloc(1024);
	szpekApiJSONPack(jsonStr, strlen(jsonStr), jsonPack);
	free(jsonStr);
	jsonStr = NULL;

	esp_http_client_config_t config = {
			.url = SZPEKAPI_BASE_URL"/SensorsMicro"
		};
	esp_http_client_handle_t client = esp_http_client_init(&config);
	esp_http_client_set_header(client, "Content-Type", "application/json");
	esp_http_client_set_method(client, HTTP_METHOD_POST);
	esp_http_client_set_post_field(client, jsonPack, strlen(jsonPack));

	esp_err_t err = esp_http_client_perform(client);
	LOG_INFO("perform = %d", err);
	LOG_INFO("response code = %d", esp_http_client_get_status_code(client));
	bool success = err == ESP_OK && esp_http_client_get_status_code(client) == 200;

	esp_http_client_cleanup(client);
	free(jsonPack);

	return success;
}

static void cJSONHelper_AddDateTimeToObject(cJSON* const object, const char* const name, const time_t timestamp)
{
	struct tm p = *gmtime(&timestamp);
	char buffer[21]; // YYYY-MM-DDTHH:mm:ssZ + null terminator
	sprintf(buffer, "%04hu-%02hhu-%02hhuT%02hhu:%02hhu:%02hhuZ", p.tm_year + 1900, p.tm_mon + 1, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec);
	cJSON_AddStringToObject(object, name, buffer);
}

static void szpekApiJSONPack(const char* data, size_t length, char* result)
{
    char* base64Data = (char*)malloc(CRYPTO_BASE64_BUFFER_LEN(length));
    size_t base64DataLen;
    mbedtls_base64_encode((unsigned char*)base64Data, CRYPTO_BASE64_BUFFER_LEN(length), &base64DataLen, (const unsigned char*)data, length);

    unsigned char signature[32];
    cryptoSignature((const unsigned char*)base64Data, base64DataLen, signature);

    char base64Signature[CRYPTO_BASE64_BUFFER_LEN(32)];
    size_t base64SigLen;
    mbedtls_base64_encode((unsigned char*)base64Signature, CRYPTO_BASE64_BUFFER_LEN(32), &base64SigLen, signature, 32);

    sprintf(result, "{ \"payload\": \"%s\", \"signature\": \"%s\" }", base64Data, base64Signature);

    free(base64Data);
}
