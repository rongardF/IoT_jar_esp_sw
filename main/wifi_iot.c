#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_event.h"
#include "freertos/event_groups.h"

#include "wifi_iot.h"

// configure WiFi interface for AP first
static 	wifi_config_t ap_wifi_config = {
        .ap = {
            .ssid = AP_WIFI_SSID,
            .ssid_len = strlen(AP_WIFI_SSID),
            .password = AP_WIFI_PASS,
            .max_connection = AP_MAX_CONNECTIONS,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

static 	wifi_config_t sta_wifi_config;

// event loop handler for wifi events - this is essentially a callback function that gets called when a WiFi related event occurs
void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
	if (event_id == WIFI_EVENT_AP_STACONNECTED)
	{  // station connected
		wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
		ESP_LOGI("WIFI_EVENT_HANDLER", "Station connected\r\n");
		xEventGroupSetBits(s_sta_ap_event_group, STA_CONNECTED);
	}
	else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
	{ // station disconnected
		wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
		ESP_LOGI("WIFI_EVENT_HANDLER", "Station dis-connected\r\n");
	}
	else if (event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "Station start\r\n");
		esp_wifi_connect();
	}
	else if (event_id == WIFI_EVENT_STA_STOP)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "Station stopped\r\n");
	}
	else if (event_id == WIFI_EVENT_STA_CONNECTED)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "Station connected\r\n");
	}
	else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "Station dis-connected\r\n");
	}
	else if (event_id == IP_EVENT_STA_GOT_IP)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "Station got IP\r\n");
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
	}
	else if (event_id == IP_EVENT_STA_LOST_IP)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "Station lost IP\r\n");
	}
	else if (event_id == IP_EVENT_AP_STAIPASSIGNED)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "IP assigned in STA-AP mode IP\r\n");
	}
	else if (event_id == WIFI_EVENT_AP_STOP)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "AP stopped\r\n");
	}
	else if (event_id == WIFI_EVENT_AP_START)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "AP started\r\n");
	}
	else if (event_id == WIFI_EVENT_AP_START)
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "AP started\r\n");
	}
	else
	{
		ESP_LOGI("WIFI_EVENT_HANDLER", "Some other event\r\n");
	}
}

void wifi_init_sta_ap()
{
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_create_default()); // create a default event loop used by system - THIS SHOULD PROBBALY BE IN MAIN BECAUSE THIS IS NOT ONLY USED BY WIFI

	s_sta_ap_event_group = xEventGroupCreate();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // create a WIFI configuration structure with default values
	ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // initialize WIFI and check for errors

	// register event handlers to default event loop - function to be executed when some event happens (e.g. WiFi connect)
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_wifi_config));
//	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_wifi_config));
	ESP_LOGI("WIFI_INIT_STA_AP", "Starting WiFi interface\r\n");
	ESP_ERROR_CHECK(esp_wifi_start());
}


void reconfigure_wifi(uint8_t * ssid, uint8_t * password)
{
	memcpy(sta_wifi_config.sta.ssid,ssid,sizeof(sta_wifi_config.sta.ssid));
	memcpy(sta_wifi_config.sta.password,password,sizeof(sta_wifi_config.sta.password));
	sta_wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_wifi_config));
}

