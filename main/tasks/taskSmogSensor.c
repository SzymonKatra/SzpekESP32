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

void taskSmogSensor(void* p)
{
	pmsParams_t params;
	params.model = PMS_7003;
	params.dataReadFun = &readFromUart;

	while (1)
	{
		pmsData_t result;
		pmsRead(&params, &result);
		smogAccumulate(result.pm1Outdoor, result.pm2_5Outdoor, result.pm10Outdoor);
		//LOG_TASK_INFO("%d, PM1: %d, PM2.5: %d, PM10: %d", errcode, result.pm1Outdoor, result.pm2_5Outdoor, result.pm10Outdoor);
	}
}

static int readFromUart(uint8_t* buffer, size_t len)
{
	return uart_read_bytes(APP_PMS_UART_PORT, buffer, len, 2000 / portTICK_PERIOD_MS);
}
