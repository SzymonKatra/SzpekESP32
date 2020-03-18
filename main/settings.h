/*
 * settings.h
 *
 *  Created on: 18 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_SETTINGS_H_
#define MAIN_SETTINGS_H_

#include <stddef.h>

void settingsInit();

const char* settingsGetSensorCode();
const char* settingsGetSensorBase64Secret();

#endif /* MAIN_SETTINGS_H_ */
