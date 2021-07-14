
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

static EventGroupHandle_t s_wifi_event_group;

void enter_deep_sleep(){

}

// initialize GPIO - this function can be called multiple times to configure different GPIOs with different configuration
esp_err_t initialize_gpio(gpio_config_t *io_cfg, uint32_t pbm, gpio_mode_t gpio_mode, gpio_pulldown_t pde, gpio_pullup_t pue)
{
	io_cfg->intr_type = GPIO_INTR_DISABLE;	// disable interrupt
	io_cfg->mode = gpio_mode;	// set as input mode
	io_cfg->pin_bit_mask = pbm;	// bit mask of the pins that you want to set
	io_cfg->pull_down_en = pde;	 // disable pull-down mode
	io_cfg->pull_up_en = pue;	 // disable pull-up mode

	return (gpio_config(io_cfg));	// configure GPIO with the given settings, return the the result
}

// event loop handler for wifi events - this is essentially a callback function that gets called when a WiFi related event occurs
void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
	 if (event_id == WIFI_EVENT_AP_STACONNECTED) {  // station connected
		wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;

	} else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) { // station disconnected
		wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;

	}
}

void wifi_init_soft_ap()
{
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_create_default()); // create a default event loop used by system (WiFi, tcp etc.) to bring up WiFi

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // create a WIFI configuration structure with default values
	ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // initialize WIFI and check for errors

	// register event handlers to default event loop - function to be executed when some event happens (e.g. WiFi connect)
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

	// configure the AP
	wifi_config_t wifi_config = {
	        .ap = {
	            .ssid = EXAMPLE_ESP_WIFI_SSID,
	            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
	            .password = EXAMPLE_ESP_WIFI_PASS,
	            .max_connection = EXAMPLE_MAX_STA_CONN,
	            .authmode = WIFI_AUTH_WPA_WPA2_PSK
	        },
	    };

	// if no password is given then it is open network (no password is required)
	if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
}

// this creates a server which accepts connection from the user to configure the device
void conf_server (){
	struct sockaddr_in addr;
	int32_t sockfd, user_socket;

	if (( sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ // create a socket handler
		// log and print the error
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(CONF_IP_ADDR); // convert IP address string to long format and assign it into IP address variable in structure
	addr.sin_port = htons(CONF_PORT_NUM);

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0){
		// log and print the error
	}

	if (listen(sockfd, 3) < 0){  // listen for incoming connections; up to three connections can be in a queue
		// log and print the error
	}

	if ((user_socket = accept(sockfd, (struct sockaddr *)&addr, (socklen_t*) &addr))<0){
		// log and print the error
	}


}

void app_main(void)
{
	esp_err_t err;
	int32_t conf_mode_en;
	int32_t conf_valid;

	// initialize GPIO to check what is the status of configuration signal
	gpio_config_t gpio_conf_struct;
	err = initialize_gpio(&gpio_conf_struct, CONF_FLAG, GPIO_MODE_INPUT, GPIO_PULLDOWN_DISABLE , GPIO_PULLUP_DISABLE);

	// check the status of configuration signal
	conf_mode_en = gpio_get_level(CONF_FLAG);

	// initialize NVS for writing-reading from flash
	err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}

	nvs_handle flash_handle;
	err = nvs_open("storage", NVS_READWRITE, &flash_handle);

	// read from flash if conf is valid
	err = nvs_get_i32(&flash_handle, "configured", &conf_valid);  // what value is returned if this key-value pair does not exist in flash???

	if ((conf_valid == 0) || (conf_mode_en == 1)){ // go into configuration mode
		wifi_init_soft_ap();
//		vTaskDelay(NO_ACTIVITY_TIMEOUT_MS / portTICK_RATE_MS);
//		if (no_sta_connected == TRUE){
//			enter_deep_sleep();
//		}

	} else { // go into normal operating mode

	}
}


