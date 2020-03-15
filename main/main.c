/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <driver/uart.h>
#include "driver/gpio.h"
#include "pms_driver.h"
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>

#include "app.h"
#include "network.h"
#include "logUtils.h"

/*
int readFromUart(uint8_t* buffer, size_t len)
{
	return uart_read_bytes(UART_NUM_2, buffer, len, 2000 / portTICK_PERIOD_MS);
}

void smogSensorTask()
{
	pmsParams_t params;
	params.model = PMS_7003;
	params.dataReadFun = &readFromUart;

	while (1)
	{
		pmsData_t result;
		pmsError_t errcode = pmsRead(&params, &result);
		printf("%d, PM1: %d, PM2.5: %d, PM10: %d\n", errcode, result.pm1Outdoor, result.pm2_5Outdoor, result.pm10Outdoor);
	}
}
*/
static void initPeripherals();
static void initNVS();

void app_main()
{
	initPeripherals();
	initNVS();
	tcpip_adapter_init();

	ESP_ERROR_CHECK(esp_event_loop_create_default());

	appInit();
}

static void initNVS()
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
}

static void initPeripherals()
{
	uart_config_t uartPms3003Config = {
		        .baud_rate = 9600,
		        .data_bits = UART_DATA_8_BITS,
		        .parity = UART_PARITY_DISABLE,
		        .stop_bits = UART_STOP_BITS_1,
		        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
		    };

	uart_param_config(UART_NUM_2, &uartPms3003Config);
	uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(UART_NUM_2, 128 * 2, 0, 0, NULL, 0);
}
