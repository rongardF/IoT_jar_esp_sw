
/*
TODO:

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

#include "types.h"
#include "gpio_control.h"
#include "wifi_iot.h"
#include "conf_server.h"
#include "mqtt_iot.h"
#include "mqtt_client.h"

#define UART_BUF_SIZE (1024)	// UART buffer size

void setup_UART()
{
	// UART configuration
	uart_config_t uart_config;	// define configuration data structure
	uart_config.baud_rate = 115200; 	// set baud rate
	uart_config.data_bits = UART_DATA_8_BITS;	// set UART byte size
	uart_config.parity = UART_PARITY_DISABLE;	// UART parity chekc disabled
	uart_config.stop_bits = UART_STOP_BITS_1;	// 1 stop bit
	uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;	// flowcontrol disabled
	uart_param_config(UART_NUM_0, &uart_config); // configure uart interface
	uart_driver_install(UART_NUM_0, UART_BUF_SIZE * 2, 0, 0, NULL, 0);  // install UART driver to FreeRTOS op system
}

void app_main(void)
{
	int rcu_conf_flag = 1;
	esp_err_t ret;
	int listener_running = 0;

	setup_UART(); // setup UART first for printing logs and testing UART
	vTaskDelay(5000 / portTICK_RATE_MS);
	ESP_LOGI("MAIN", "Starting application\r\n");

//	setup_GPIO(GPIO_MODE_INPUT, CONF_FLAG, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_DISABLE);  // disable GPIO for resetting VL6180 device
//	rcu_conf_flag = gpio_get_level(CONF_FLAG);

	if (rcu_conf_flag == 0)  // RCU is not active, run normal process if device is configured
	{
		// initialize NVS for writing-reading from flash
		ret = nvs_flash_init();
		if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
			// NVS partition was truncated and needs to be erased
			// Retry nvs_flash_init
			ESP_ERROR_CHECK(nvs_flash_erase());
			ret = nvs_flash_init();
		}

		nvs_handle flash_handle;
		ret = nvs_open("storage", NVS_READWRITE, &flash_handle);
	}
	else if (rcu_conf_flag == 1)  // RCU is active, go into configuring mode
	{
		// initialize ESP as AP+STA with default IP and port
		wifi_init_ap();

		while (1)
		{
			// wait until we have a station connected to AP
			EventBits_t bits = xEventGroupWaitBits(s_sta_ap_event_group,
					STA_CONNECTED,
					pdTRUE,
					pdFALSE,
					portMAX_DELAY);

			if ((bits & STA_CONNECTED) && (listener_running == 0))
			{
				// create a task to receive connection
				xTaskCreate(listener, "listener", 1024, NULL, 5, NULL);
				listener_running = 1;
			}
			else if (bits & STA_CONNECTED)
			{
				// create a MQTT task
				xTaskCreate(mqtt_task, "mqtt", 4096, NULL, 5, NULL);
			}
			else
			{
				ESP_LOGI("MAIN", "Timeout\r\n");
			}
		}
	}
	else
	{
		ESP_LOGE("MAIN", "Error: invalid value for rcu_conf_flag \r\n");
	}
}


