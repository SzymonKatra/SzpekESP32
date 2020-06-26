/*
 * taskWeatherSensor.c
 *
 *  Created on: 26 cze 2020
 *      Author: szymo
 */


#include "taskWeatherSensor.h"
#include "logUtils.h"
#include "app.h"
#include "bme280.h"
#include "weather.h"

static bool initBME280(struct bme280_dev* dev);

static void user_delay_ms(uint32_t period);
static int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
static int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

void taskWeatherSensor(void* p)
{
	struct bme280_dev dev;

	while (!initBME280(&dev)) {
		vTaskDelay(pdMS_TO_TICKS(10000));
	}

	uint32_t req_delay = bme280_cal_meas_delay(&dev.settings);
	LOG_TASK_INFO("Measure delay = %d", req_delay);

	uint8_t rslt;
	struct bme280_data bmeData;

	while (1)
	{
		rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
		if (rslt != 0)
		{
			LOG_TASK_ERROR("BME280 error! (%d)", rslt);

			weatherInvalidate();
			vTaskDelay(pdMS_TO_TICKS(10000));
		}

		dev.delay_ms(req_delay * 2); // delay to measure

		rslt = bme280_get_sensor_data(BME280_ALL, &bmeData, &dev);
		if (rslt != 0)
		{
			LOG_TASK_ERROR("BME280 error! (%d)", rslt);

			weatherInvalidate();
			vTaskDelay(pdMS_TO_TICKS(10000));
		}

		//LOG_TASK_INFO("Temp: %d, Press: %d, Hum: %d", (int)bmeData.temperature, (int)(bmeData.pressure / 100), (int)bmeData.humidity);
		weatherUpdate(bmeData.temperature, bmeData.pressure / 100.0, bmeData.humidity);

		vTaskDelay(pdMS_TO_TICKS(60000));
	}
}

static bool initBME280(struct bme280_dev* dev)
{
	dev->dev_id = BME280_I2C_ADDR_PRIM;
	dev->intf = BME280_I2C_INTF;
	dev->read = user_i2c_read;
	dev->write = user_i2c_write;
	dev->delay_ms = user_delay_ms;

	int8_t rslt = bme280_init(dev);
	if (rslt != 0)
	{
		LOG_TASK_ERROR("Error while initializing BME280! BME280 driver error code = %d", rslt);
		return false;
	}

	dev->settings.osr_h = BME280_OVERSAMPLING_1X;
	dev->settings.osr_p = BME280_OVERSAMPLING_1X;
	dev->settings.osr_t = BME280_OVERSAMPLING_1X;
	dev->settings.filter = BME280_FILTER_COEFF_OFF;
	dev->settings.standby_time = BME280_STANDBY_TIME_1000_MS;

	uint8_t settings_sel = 0;
	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;
	rslt = bme280_set_sensor_settings(settings_sel, dev);

	if (rslt != 0)
	{
		LOG_TASK_ERROR("Error while setting sensor settings for BME280! BME280 driver error code = %d", rslt);
		return false;
	}

	LOG_TASK_INFO("BME280 initialized successfully!");
	return true;
}

static void user_delay_ms(uint32_t period)
{
    vTaskDelay(pdMS_TO_TICKS(period));
}

static int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
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

static int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
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
