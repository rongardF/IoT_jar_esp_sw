
/*
TODO:
- Create a timer which waits TIMEOUT (default = 300s) in AP mode and if not STA connect then it configures ESP for deep-sleep and enters deep-sleep
- Create a "server" that waits for connection on some specific socket and IP and which creates an interface for user-configuration.


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

#include "mqtt_client.h"

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

#define UART_BUF_SIZE (1024)	// UART buffer size
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define EXAMPLE_ESP_WIFI_SSID "Telia-8D67F2"
#define EXAMPLE_ESP_WIFI_PASS "HUPKG3QHOETDK6"

#define MQTT_BROKER_IP "192.168.1.129"
#define MQTT_BROKER_PORT 9999

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

static void connection_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 5) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI("CONNECTION_EVENT_HANDLER", "Retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, BIT1);
        }
        ESP_LOGI("CONNECTION_EVENT_HANDLER","Connection to the AP failed");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("CONNECTION_EVENT_HANDLER", "got IP:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, BIT0);
    }
}

static void mqtt_event_handler_cb (esp_mqtt_event_handle_t event)
{
	esp_mqtt_client_handle_t client = event->client;
	int msg_id;
	// your_context_t *context = event->context;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_CONNECTED");
			msg_id = esp_mqtt_client_publish(client, "main", "TEST", 0, 1, 0);
			ESP_LOGI("MQTT_EVENT_HANDLER", "Sent publish successful, msg_id=%d", msg_id);
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_DISCONNECTED");
			break;
//		case MQTT_EVENT_SUBSCRIBED:
//			ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
//			msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
//			ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
//			break;
//		case MQTT_EVENT_UNSUBSCRIBED:
//			ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
//			break;
		case MQTT_EVENT_PUBLISHED:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_DATA:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_DATA");
			printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
			printf("DATA=%.*s\r\n", event->data_len, event->data);
			break;
		case MQTT_EVENT_ERROR:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_ERROR");
			break;
		case MQTT_EVENT_BEFORE_CONNECT:
			ESP_LOGI("MQTT_EVENT_HANDLER", "MQTT_EVENT_BEFORE_CONNECT");
			break;
		default:
			ESP_LOGI("MQTT_EVENT_HANDLER", "Other event id:%d", event->event_id);
			break;
	}
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD("MQTT_EVENT_HANDLER", "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void app_main(void)
{
	int rcu_conf_flag = 1;
	esp_err_t ret;

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
		wifi_init_sta_ap();
		EventBits_t bits = xEventGroupWaitBits(s_sta_ap_event_group,
				STA_CONNECTED,
				pdTRUE,
				pdFALSE,
				portMAX_DELAY);

		if (bits & STA_CONNECTED)
		{
			ESP_LOGI("MAIN", "Station has connected\r\n");
		}
		else
		{
			ESP_LOGI("MAIN", "Timeout\r\n");
		}
	}
	else
	{
		ESP_LOGE("MAIN", "Error: invalid value for rcu_conf_flag \r\n");
	}

//	ESP_ERROR_CHECK(nvs_flash_init());
//
//	s_wifi_event_group = xEventGroupCreate();
//
//	tcpip_adapter_init();
//
//	ESP_ERROR_CHECK(esp_event_loop_create_default());
//
//	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
//
//	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &connection_event_handler, NULL));
//	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connection_event_handler, NULL));
//
//	wifi_config_t wifi_config = {
//		.sta = {
//			.ssid = EXAMPLE_ESP_WIFI_SSID,
//			.password = EXAMPLE_ESP_WIFI_PASS
//		},
//	};
//
//	/* Setting a password implies station will connect to all security modes including WEP/WPA.
//		* However these modes are deprecated and not advisable to be used. Incase your Access point
//		* doesn't support WPA2, these mode can be enabled by commenting below line */
//
//	if (strlen((char *)wifi_config.sta.password)) {
//		wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
//	}
//
//	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
//	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
//	ESP_ERROR_CHECK(esp_wifi_start() );
//
//	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
//	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by connection_event_handler() (see above) */
//	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
//			WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
//			pdFALSE,
//			pdFALSE,
//			portMAX_DELAY);
//
//	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
//	 * happened. */
//	if (bits & WIFI_CONNECTED_BIT) {
//		ESP_LOGI("MAIN", "connected to AP with SSID:%s password:%s",
//				 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
//	} else if (bits & WIFI_FAIL_BIT) {
//		ESP_LOGI("MAIN", "Failed to connect to SSID:%s, password:%s",
//				 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
//	} else {
//		ESP_LOGE("MAIN", "UNEXPECTED EVENT");
//	}
//
//	ESP_LOGI("MAIN","Creating MQTT config structure");
//
//	esp_mqtt_client_config_t mqtt_cfg = {
//	        .host = MQTT_BROKER_IP,
//			.port = MQTT_BROKER_PORT,
//			.transport= MQTT_TRANSPORT_OVER_TCP
//	    };
//
//	ESP_LOGI("MAIN","Creating MQTT handle");
//	esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
//	ESP_LOGI("MAIN","Registering MQTT events to event loop");
//	esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
//	ESP_LOGI("MAIN","Starting MQTT");
//	esp_mqtt_client_start(client);
//	ESP_LOGI("MAIN","Main has finished");

//	ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
//	ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
//	vEventGroupDelete(s_wifi_event_group);

// THIS POINT FORWARD IS OLD CODE FOR IOT JAR
//	esp_err_t err;
//	int32_t conf_mode_en;
//	int32_t conf_valid;
//
//	// initialize GPIO to check what is the status of configuration signal
//	gpio_config_t gpio_conf_struct;
//	err = initialize_gpio(&gpio_conf_struct, CONF_FLAG, GPIO_MODE_INPUT, GPIO_PULLDOWN_DISABLE , GPIO_PULLUP_DISABLE);
//
//	// check the status of configuration signal
//	conf_mode_en = gpio_get_level(CONF_FLAG);
//
//	// initialize NVS for writing-reading from flash
//	err = nvs_flash_init();
//	if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
//		// NVS partition was truncated and needs to be erased
//		// Retry nvs_flash_init
//		ESP_ERROR_CHECK(nvs_flash_erase());
//		err = nvs_flash_init();
//	}
//
//	nvs_handle flash_handle;
//	err = nvs_open("storage", NVS_READWRITE, &flash_handle);
//
//	// read from flash if conf is valid
//	err = nvs_get_i32(&flash_handle, "configured", &conf_valid);  // what value is returned if this key-value pair does not exist in flash???
//
//	if ((conf_valid == 0) || (conf_mode_en == 1)){ // go into configuration mode
//		wifi_init_soft_ap();
////		vTaskDelay(NO_ACTIVITY_TIMEOUT_MS / portTICK_RATE_MS);
////		if (no_sta_connected == TRUE){
////			enter_deep_sleep();
////		}
//
//	} else { // go into normal operating mode
//
//	}
}


