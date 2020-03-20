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

extern const size_t NETWORK_SSID_SIZE;
extern const size_t NETWORK_PASSWORD_SIZE;

typedef enum
{
	NETWORK_OK
} networkError_t;

ESP_EVENT_DECLARE_BASE(NETWORK_EVENT);

typedef enum
{
	NETWORK_EVENT_CONNECTION_ESTABLISHED,
	NETWORK_EVENT_CONNECTION_LOST,
	NETWORK_EVENT_HOTSPOT_STARTED,
	NETWORK_EVENT_HOTSPOT_STOPPED
} networkEvent_t;

void networkInit();
networkError_t networkSTAConnection(const char ssid[NETWORK_SSID_SIZE], const char password[NETWORK_PASSWORD_SIZE]);
networkError_t networkHotspot(const char ssid[NETWORK_SSID_SIZE], const char password[NETWORK_PASSWORD_SIZE]);
void networkStop();
void networkWaitForEstablish();
bool networkIsEstablished();

#endif /* MAIN_NETWORK_H_ */
