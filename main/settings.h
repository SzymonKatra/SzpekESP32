/*
 * settings.h
 *
 *  Created on: 18 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_SETTINGS_H_
#define MAIN_SETTINGS_H_

#include <stddef.h>
#include "network.h"
#include <esp_log.h>

typedef struct
{
	char code[32];
	char secretBase64[45];
	char board[32];
	char model[64];
} settingsSzpekId_t;

typedef struct
{
	char ssid[NETWORK_SSID_SIZE];
	char password[NETWORK_PASSWORD_SIZE];
} settingsWifi_t;

void settingsInit();

const settingsSzpekId_t* settingsGetSzpekId();
const settingsWifi_t* settingsGetWifi();
void settingsSetWifi(const settingsWifi_t* wifiSettings);

esp_log_level_t settingsGetRemoteLogLevel();

#endif /* MAIN_SETTINGS_H_ */
