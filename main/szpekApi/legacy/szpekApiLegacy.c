/*
 * szpekApi.c
 *
 *  Created on: 18 mar 2020
 *      Author: szymo
 */

#include "szpekApiLegacy.h"

#include <stdlib.h>
#include <mbedtls/base64.h>
#include <esp_http_client.h>
#include <cJSON.h>
#include <cJSONExt.h>
#include "crypto.h"
#include <string.h>
#include "logUtils.h"

static const char* LOG_TAG = "SzpekApiLegacy";

#define SZPEKAPI_BASE_URL "https://api.szpek.pl/api"

static void szpekApiJSONPack(const char* data, size_t length, char* result);

static int performSignedPOST(const char* url, const char* jsonData);

bool szpekApiLegacySensorsMicro(const szpekApiLegacySensorsMicroContract_t* data)
{
	cJSON* root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "pm1Value", data->pm1Value);
	cJSON_AddNumberToObject(root, "pm25Value", data->pm25Value);
	cJSON_AddNumberToObject(root, "pm10Value", data->pm10Value);
	cJSON_AddNumberToObject(root, "samplesQuantity", data->samplesQuantity);
	cJSONExt_AddDateTimeToObject(root, "periodFrom", data->periodFrom);
	cJSONExt_AddDateTimeToObject(root, "periodTo", data->periodTo);
	cJSON_AddStringToObject(root, "sensorCode", data->sensorCode);
	char* jsonStr = cJSON_PrintBuffered(root, 512, false);
	cJSON_Delete(root);

	int status = performSignedPOST(SZPEKAPI_BASE_URL"/SensorsMicro", jsonStr);
	free(jsonStr);

	return status == 200;
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

static int performSignedPOST(const char* url, const char* jsonData)
{
	size_t len = strlen(jsonData);
	char* jsonPack = malloc(CRYPTO_BASE64_BUFFER_LEN(len) + 128);
	szpekApiJSONPack(jsonData, strlen(jsonData), jsonPack);

	esp_http_client_config_t config = { .url = url, .method = HTTP_METHOD_POST };
	esp_http_client_handle_t client = esp_http_client_init(&config);
	esp_http_client_set_header(client, "Content-Type", "application/json");
	esp_http_client_set_post_field(client, jsonPack, strlen(jsonPack));

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
	free(jsonPack);

	return status;
}
