/*
 * taskSmogSensor.c
 *
 *  Created on: 15 mar 2020
 *      Author: szymo
 */

#include "taskSmogSensor.h"
#include "logUtils.h"
#include "smog.h"
#include "app.h"
#include "pms_driver.h"

static int readFromUart(uint8_t* buffer, size_t len);

static void pmsTest();

void taskSmogSensor(void* p)
{
	pmsParams_t params;
	params.model = PMS_7003;
	params.dataReadFun = &readFromUart;

	LOG_TASK_INFO("Testing PMS7003...");
	pmsTest(&params);
	LOG_TASK_INFO("Testing PMS7003 finished");

	while (1)
	{
		pmsData_t result;
		pmsError_t err = pmsRead(&params, &result);
		if (err == PMS_OK)
		{
			smogAccumulate(result.pm1Outdoor, result.pm2_5Outdoor, result.pm10Outdoor);
		}
		else
		{
			LOG_TASK_ERROR("An error occurred while reading data from PMS7003! Error = %d", err);
		}
		//LOG_TASK_INFO("%d, PM1: %d, PM2.5: %d, PM10: %d", err, result.pm1Outdoor, result.pm2_5Outdoor, result.pm10Outdoor);
	}
}

static int readFromUart(uint8_t* buffer, size_t len)
{
	return uart_read_bytes(APP_PMS_UART_PORT, buffer, len, 2000 / portTICK_PERIOD_MS);
}

static void pmsTest(const pmsParams_t* params)
{
	pmsData_t result;
	pmsError_t err = PMS_ERR_DATA_READ;
	while (err == PMS_ERR_DATA_READ)
	{
		err = pmsRead(params, &result);
		if (err == PMS_ERR_DATA_READ)
		{
			LOG_TASK_ERROR("Cannot read bytes from PMS7003!");
		}
	}

	while (err != PMS_OK)
	{
		for (size_t i = 0; i < 10; i ++)
		{
			err = pmsRead(params, &result);
			if (err == PMS_OK) break;
		}

		if (err != PMS_OK)
		{
			LOG_TASK_ERROR("Receiving invalid data from PMS7003, last error %d!", err);
		}
	}

	LOG_TASK_INFO("PMS7003 first read success! PM1: %d, PM2.5: %d, PM10: %d", result.pm1Outdoor, result.pm2_5Outdoor, result.pm10Outdoor);
}
