/*
 * settings.h
 *
 *  Created on: 18 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_SETTINGS_H_
#define MAIN_SETTINGS_H_

#include <stddef.h>

typedef struct
{
	char code[32];
	char secretBase64[45];
} settingsSzpekId_t;

void settingsInit();

const settingsSzpekId_t* settingsGetSzpekId();

#endif /* MAIN_SETTINGS_H_ */
