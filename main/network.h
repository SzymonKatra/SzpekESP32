/*
 * network.h
 *
 *  Created on: 14 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_NETWORK_H_
#define MAIN_NETWORK_H_

#include <stdbool.h>
#include <esp_event.h>

typedef enum
{
	NETWORK_OK,
	NETWORK_ERR_TOO_LONG_SSID,
	NETWORK_ERR_TOO_LONG_PASSWORD
} networkError_t;

ESP_EVENT_DECLARE_BASE(NETWORK_EVENT);

typedef enum
{
	NETWORK_EVENT_CONNECTION_ESTABLISHED,
	NETWORK_EVENT_CONNECTION_LOST
} networkEvent_t;

void networkInit();
networkError_t networkSTAConnection(const char* ssid, const char* password);
void networkWaitForEstablish();
bool networkIsEstablished();

#endif /* MAIN_NETWORK_H_ */
