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
#include "logUtils.h"

static const char* LOG_TAG = "Settings";

static settingsSzpekId_t s_szpekId;

static void loadSzpekId();

void settingsInit()
{
	loadSzpekId();
}

const settingsSzpekId_t* settingsGetSzpekId()
{
	return &s_szpekId;
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
