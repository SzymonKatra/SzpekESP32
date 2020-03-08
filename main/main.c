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
#include "pmsX003.h"
#include <driver/uart.h>
#include "driver/gpio.h"

void smogSensorTask()
{
	while (1)
	{
		pmsX003Data_t result;
		pmsX003Result_t errcode = pmsX003_read(&result);
		printf("%d, PM1: %d, PM2.5: %d, PM10: %d\n", errcode, result.pm1Outdoor, result.pm2_5Outdoor, result.pm10Outdoor);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void app_main()
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

	pmsX003_init(PMS3003, UART_NUM_2, 0, PMSX003_MODE_ACTIVE);

	xTaskCreate(&smogSensorTask, "task", 2048, NULL, 3, NULL);



    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    //esp_restart();
}
