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

static void testBME280();
void user_delay_ms(uint32_t period);
int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);


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
	i2cConfig.master.clk_speed = 100000;
	i2c_param_config(APP_I2C_MASTER, &i2cConfig);
	i2c_driver_install(APP_I2C_MASTER, i2cConfig.mode, 0, 0, 0);

	testBME280();

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


static void testBME280()
{
	struct bme280_dev dev;

	dev.dev_id = BME280_I2C_ADDR_PRIM;
	dev.intf = BME280_I2C_INTF;
	dev.read = user_i2c_read;
	dev.write = user_i2c_write;
	dev.delay_ms = user_delay_ms;

	int8_t rslt = bme280_init(&dev);
	ESP_LOGI("BME280 test", "bme280_init = %d", rslt);

	uint8_t settings_sel;
	struct bme280_data comp_data;

	/* Recommended mode of operation: Indoor navigation */
	dev.settings.osr_h = BME280_OVERSAMPLING_1X;
	dev.settings.osr_p = BME280_OVERSAMPLING_16X;
	dev.settings.osr_t = BME280_OVERSAMPLING_2X;
	dev.settings.filter = BME280_FILTER_COEFF_16;
	dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;
	rslt = bme280_set_sensor_settings(settings_sel, &dev);
	rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);

	ESP_LOGI("BME280 test", "Temperature, Pressure, Humidity");

	dev.delay_ms(200); // delay to measure
	rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
	ESP_LOGI("BME280 test", "%d %d %d", (int)comp_data.temperature, (int)comp_data.pressure, (int)comp_data.humidity);
}

void user_delay_ms(uint32_t period)
{
    vTaskDelay(pdMS_TO_TICKS(period));
}

int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, reg_data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(APP_I2C_MASTER, cmd, pdMS_TO_TICKS(10));

    i2c_cmd_link_delete(cmd);

    return err == ESP_OK ? 0 : -1;
}

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, reg_data, len, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(APP_I2C_MASTER, cmd, 0);

    i2c_cmd_link_delete(cmd);

    return err == ESP_OK ? 0 : -1;
}
