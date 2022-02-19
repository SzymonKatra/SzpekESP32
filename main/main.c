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
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>

#include "app.h"
#include "network.h"
#include "logUtils.h"

#include <bme280.h>

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
	uart_config_t uartPmsConfig =
	{
		.baud_rate = 9600,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};

	uart_param_config(APP_PMS_UART_PORT, &uartPmsConfig);
	uart_set_pin(APP_PMS_UART_PORT, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(APP_PMS_UART_PORT, 128 * 2, 0, 0, NULL, 0);

	i2c_config_t i2cConfig;
	i2cConfig.mode = I2C_MODE_MASTER;
	i2cConfig.sda_io_num = GPIO_NUM_21;
	i2cConfig.sda_pullup_en = GPIO_PULLUP_ENABLE;
	i2cConfig.scl_io_num = GPIO_NUM_22;
	i2cConfig.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2cConfig.clk_flags = 0;
	i2cConfig.master.clk_speed = 100000;
	i2c_param_config(APP_I2C_MASTER, &i2cConfig);
	i2c_driver_install(APP_I2C_MASTER, i2cConfig.mode, 0, 0, 0);

	gpio_config_t ledsConfig =
	{
		.pin_bit_mask = (1ULL << APP_LED_GREEN_GPIO) | (1ULL << APP_LED_ORANGE_GPIO) | (1ULL << APP_LED_RED_GPIO),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&ledsConfig);

	gpio_config_t buttonsConfig =
	{
		.pin_bit_mask = (1ULL << APP_BUTTON_GPIO),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&buttonsConfig);
}
