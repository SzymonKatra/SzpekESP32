/*
 * cJSONExt.c
 *
 *  Created on: 5 kwi 2020
 *      Author: szymo
 */


#include "cJSONExt.h"

#include <stdio.h>
#include <string.h>

void cJSONExt_AddDateTimeToObject(cJSON* const object, const char* const name, const time_t timestamp)
{
	struct tm p = *gmtime(&timestamp);
	char buffer[21]; // YYYY-MM-DDTHH:mm:ssZ + null terminator
	sprintf(buffer, "%04hu-%02hhu-%02hhuT%02hhu:%02hhu:%02hhuZ", p.tm_year + 1900, p.tm_mon + 1, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec);
	cJSON_AddStringToObject(object, name, buffer);
}

bool cJSONExt_TryReadString(const cJSON* const jsonObject, const char* const fieldName, char* result)
{
	cJSON* jsonField = cJSON_GetObjectItemCaseSensitive(jsonObject, fieldName);
	if (cJSON_IsString(jsonField) && jsonField->valuestring != NULL)
	{
		strcpy(result, jsonField->valuestring);
		return true;
	}

	return false;
}

bool cJSONExt_TryReadNumber(const cJSON* const jsonObject, const char* const fieldName, double* result)
{
	cJSON* jsonField = cJSON_GetObjectItemCaseSensitive(jsonObject, fieldName);
	if (cJSON_IsNumber(jsonField))
	{
		*result = jsonField->valuedouble;
		return true;
	}

	return false;
}

bool cJSONExt_TryReadTimestamp(const cJSON* const jsonObject, const char* const fieldName, time_t* result)
{
	double tmDouble = 0;
	if (cJSONExt_TryReadNumber(jsonObject, fieldName, &tmDouble))
	{
		*result = (time_t)tmDouble;
		return true;
	}

	return false;
}
