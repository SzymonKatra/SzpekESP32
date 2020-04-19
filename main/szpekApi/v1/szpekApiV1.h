/*
 * szpekApiV1.h
 *
 *  Created on: 4 kwi 2020
 *      Author: szymo
 */

#ifndef MAIN_SZPEKAPI_V1_SZPEKAPIV1_H_
#define MAIN_SZPEKAPI_V1_SZPEKAPIV1_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <esp_http_client.h>

typedef struct
{
	esp_http_client_handle_t httpClientHandle;
	size_t bytesRead;
} szpekApiV1FileDownloadState_t;

typedef struct
{
	char name[51];
	time_t releaseTimestamp;
} szpekApiV1FirmwareMetadata_t;

typedef struct
{
	float pm10Value;
	float pm2_5Value;
	float pm1Value;
	size_t samplesCount;
	time_t timestampFrom;
	time_t timestampTo;
} szpekApiV1ReportSmog_t;

bool szpekApiV1ReportSmog(const szpekApiV1ReportSmog_t* report);

bool szpekApiV1ReportStartup(const char* firmwareName);

bool szpekApiV1Log(const char* message);

bool szpekApiV1GetRecommendedFirmwareMetadata(szpekApiV1FirmwareMetadata_t* result);

bool szpekApiV1DownloadFirmwareBegin(const char* firmwareName, szpekApiV1FileDownloadState_t* state);
int szpekApiV1DownloadFirmwareRead(szpekApiV1FileDownloadState_t* state, unsigned char* buffer, size_t length);
void szpekApiV1DownloadFirmwareEnd(szpekApiV1FileDownloadState_t* state);

#endif /* MAIN_SZPEKAPI_V1_SZPEKAPIV1_H_ */
