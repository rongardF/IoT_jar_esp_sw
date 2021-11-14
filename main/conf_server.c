#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "conf_server.h"
#include "wifi_iot.h"

struct netdata {
	uint8_t SSID[32];
	uint8_t PASSWORD[64];
};

void conf_server(int32_t socket)
{
	uint32_t payload_len, word_len;
	uint8_t recv_buffer[100] = {0};
	struct netdata new_conf;

	if (recv(socket, &payload_len, sizeof(payload_len), 0) < 0)
	{
		ESP_LOGE("CONF_SERVER", "Failed to receive payload size value\r\n");
	}

	payload_len = ntohl(payload_len);

	ESP_LOGI("CONF_SERVER", "Payload size %u\r\n",payload_len);

	if (recv(socket, &word_len, sizeof(word_len), 0) < 0)
	{
		ESP_LOGE("CONF_SERVER", "Failed to receive word size value\r\n");
	}

	word_len = ntohl(word_len);

	ESP_LOGI("CONF_SERVER", "Word size %u\r\n",word_len);

	if (recv(socket, recv_buffer, word_len, 0) < 0)
	{
		ESP_LOGE("CONF_SERVER", "Failed to receive SSID word\r\n");
	}

	for (int i=0; i<=word_len; i++)
	{
		new_conf.SSID[i] = recv_buffer[i];  // this does not need to have nthol for endianess conversion because only single byte!
		if (i == word_len)
		{
			new_conf.SSID[i] = 0;  //null character to signal the end of string (\0)
		}
	}

	if (recv(socket, &word_len, sizeof(word_len), 0) < 0)
	{
		ESP_LOGE("CONF_SERVER", "Failed to receive word size value\r\n");
	}

	word_len = ntohl(word_len);

	ESP_LOGI("CONF_SERVER", "Word size %u\r\n",word_len);

	if (recv(socket, recv_buffer, word_len, 0) < 0)
	{
		ESP_LOGE("CONF_SERVER", "Failed to receive PASSWORD word\r\n");
	}

	for (int i=0; i <= word_len; i++)
	{
		new_conf.PASSWORD[i] = recv_buffer[i]; // this does not need to have nthol for endianess conversion because only single byte!
		if (i == word_len)
		{
			new_conf.PASSWORD[i] = 0;  //null character to signal the end of string (\0)
		}
	}

	ESP_LOGI("CONF_SERVER", "SSID: %s\r\n",(char*) new_conf.SSID);
	ESP_LOGI("CONF_SERVER", "Password: %s\r\n",(char*) new_conf.PASSWORD);

	while (1)
	{
		vTaskDelay(5000 / portTICK_RATE_MS);
	}
}

// this creates a server which accepts connection from the user to configure the device
void listener ()
{
	struct sockaddr_in server_addr, client_addr;
	int32_t sockfd, user_socket;

	if (( sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		// log and print the error
		ESP_LOGE("CONF_SERVER", "Server listener failed to get the socket handler\r\n");
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; // convert IP address string to long format and assign it into IP address variable in structure
	server_addr.sin_port = htons(CONF_PORT_NUM);

	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		// log and print the error
		ESP_LOGE("CONF_SERVER", "Failed to bind to socket\r\n");
	}

	if (listen(sockfd, 1) < 0) // listen for incoming connections; only one connections can be in a queue
	{
		// log and print the error
		ESP_LOGE("CONF_SERVER", "Listen task failed\r\n");
	}

	while (1)
	{
		if ((user_socket = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t*) &client_addr))<0)
		{
			// log and print the error
			ESP_LOGE("CONF_SERVER", "Failed to retrieve socket handler for incoming connection\r\n");
		}

		// create a task which communicates with client application to receive home network information and commands
		xTaskCreate(conf_server, "conf_server", 4096, user_socket, 5, NULL);

		// wait until some event happens - eventGroupWaitBits here

		// check if STA disconnected or client app closed connection - wait for a new connection if configuration was not finished

		// check if all configuration information received and have communication with MQTT broker - delete both tasks then and re-configurefor STA only

	}



//	ESP_LOGI("CONF_SERVER", "Connection received\r\n");
//
//	if(recv(user_socket, message, 200, 0) < 0)
//	{
//		ESP_LOGE("CONF_SERVER", "Failed to receive a message\r\n");
//	}
//	strcpy(home_ssid,message);
//
//	ESP_LOGI("CONF_SERVER", "SSID received: %s\r\n",home_ssid);
//
//	if(recv(user_socket, message, 200, 0) < 0)
//	{
//		ESP_LOGE("CONF_SERVER", "Failed to receive a message\r\n");
//	}
//	strcpy(home_passwd,message);
//
//	ESP_LOGI("CONF_SERVER", "Password received: %s\r\n",home_passwd);
}


