/*
 * taskFirmwareOTA.c
 *
 *  Created on: 5 kwi 2020
 *      Author: szymo
 */

#include "taskFirmwareOTA.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_ota_ops.h>
#include <esp_https_ota.h>
#include <esp_partition.h>
#include <string.h>
#include "logUtils.h"
#include "app.h"
#include "szpekApi/v1/szpekApiV1.h"
#include "timeTriggers.h"

#define BUFFER_SIZE 4096

static void hourPassedHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static bool needsFirmwareUpdate(szpekApiV1FirmwareMetadata_t* firmwareMetadata);
static bool updateFirmware(const char* firmwareName, const esp_partition_t** newBootPartition);

void taskFirmwareOTA(void* p)
{
	appRegisterEventHandler(TIME_TRIGGERS_EVENT, TIME_TRIGGERS_EVENT_HOUR_PASSED, hourPassedHandler, NULL);

	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		szpekApiV1FirmwareMetadata_t firmwareMetadata;
		if (needsFirmwareUpdate(&firmwareMetadata))
		{
			LOG_TASK_INFO("Updating firmware started!");
			const esp_partition_t* newBootPartition = NULL;
			if (updateFirmware(firmwareMetadata.name, &newBootPartition))
			{
				LOG_TASK_INFO("Setting OTA boot partition: %d", newBootPartition->subtype);
				ESP_ERROR_CHECK(esp_ota_set_boot_partition(newBootPartition));
				LOG_TASK_INFO("Restarting in 3 seconds!");
				vTaskDelay(pdMS_TO_TICKS(3000));
				esp_restart();
			}
		}
		else
		{
			LOG_TASK_INFO("Firmware is up to date!");
		}
	}
}

static void hourPassedHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	timeTriggersEventHourData_t* hourData = (timeTriggersEventHourData_t*)event_data;
	if (*hourData == 0)
	{
		xTaskNotifyGive(appGetTasks()->firmwareOTA);
	}
}

static bool needsFirmwareUpdate(szpekApiV1FirmwareMetadata_t* firmwareMetadata)
{
	LOG_TASK_INFO("Checking recommended firmware...");
	if (szpekApiV1GetRecommendedFirmwareMetadata(firmwareMetadata))
	{
		LOG_TASK_INFO("Current firmware: %s. Recommended firmware: %s", APP_FIRMWARE_NAME, firmwareMetadata->name);
		return strcmp(APP_FIRMWARE_NAME, firmwareMetadata->name) != 0;
	}
	else
	{
		LOG_TASK_ERROR("An error occurred while checking recommended firmware");
	}

	return false;
}

static bool updateFirmware(const char* firmwareName, const esp_partition_t** newBootPartition)
{
	esp_err_t err;
	unsigned char* buffer = NULL;

	LOG_TASK_INFO("Updating to firmware %s started", firmwareName);
	const esp_partition_t* targetPartition = esp_ota_get_next_update_partition(NULL);
	if (targetPartition == NULL)
	{
		LOG_TASK_ERROR("Partition for OTA not found!");
		return false;
	}

	LOG_TASK_INFO("OTA partition found - %d at 0x%x", targetPartition->subtype, targetPartition->address);

	// THIS IS HACK FOR FAILING esp_ota_begin when erasing entire partition at once
	esp_ota_handle_t otaHandle;
	const int ERASE_CHUNK_SIZE = 16384;
	for (int i = 0; i < targetPartition->size / ERASE_CHUNK_SIZE; i++)
	{
		LOG_TASK_INFO("Erasing OTA partition... (0x%x)", i * ERASE_CHUNK_SIZE);
		esp_partition_erase_range(targetPartition, i * ERASE_CHUNK_SIZE, ERASE_CHUNK_SIZE);
		vTaskDelay(pdMS_TO_TICKS(10));
	}
	ESP_ERROR_CHECK(esp_ota_begin(targetPartition, 1, &otaHandle));
	// should be just esp_ota_begin(targetPartition, OTA_SIZE_UNKNOWN, &otaHandle);

	LOG_TASK_INFO("Starting firmware %s download...", firmwareName);
	szpekApiV1FileDownloadState_t downloadState;
	if (!szpekApiV1DownloadFirmwareBegin(firmwareName, &downloadState))
	{
		LOG_TASK_ERROR("An error occurred while starting to download firmware %s", firmwareName);
		szpekApiV1DownloadFirmwareEnd(&downloadState);
		return false;
	}

	buffer = malloc(BUFFER_SIZE);
	configASSERT(buffer);

	int bytesRead;
	bool otaError = false;
	do
	{
		bytesRead = szpekApiV1DownloadFirmwareRead(&downloadState, buffer, BUFFER_SIZE);
		LOG_TASK_INFO("Firmware download progress - %d bytes", downloadState.bytesRead);
		if (bytesRead > 0)
		{
			err = esp_ota_write(otaHandle, buffer, bytesRead);
			if (err != ESP_OK)
			{
				LOG_TASK_ERROR("esp_ota_write failed: %d/%s", err, esp_err_to_name(err));
				otaError = true;
				break;
			}
		}
	}
	while (bytesRead > 0);

	free(buffer);
	szpekApiV1DownloadFirmwareEnd(&downloadState);

	if (otaError)
	{
		esp_ota_end(otaHandle); // free up resources
		return false;
	}

	if (bytesRead != 0)
	{
		LOG_TASK_INFO("An error occurred while downloading firmware");
		return false;
	}


	LOG_TASK_INFO("Firmware downloaded successfully");

	err = esp_ota_end(otaHandle);
	if (err == ESP_OK)
	{
		LOG_TASK_INFO("Firmware validated successfully!");
		*newBootPartition = targetPartition;
	}
	else
	{
		LOG_TASK_ERROR("Firmware validation failed! Error: %d/%s", err, esp_err_to_name(err));
	}

	return err == ESP_OK;

}
