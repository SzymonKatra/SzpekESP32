/*
 * smog.h
 *
 *  Created on: 15 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_SMOG_H_
#define MAIN_SMOG_H_

#include <stdint.h>

typedef struct
{
	float pm1;
	float pm2_5;
	float pm10;
	uint16_t samplesCount;
} smogResult_t;

void smogInit();
void smogAccumulate(uint16_t pm1, uint16_t pm2_5, uint16_t pm10);
void smogReset();
void smogSummaryAndReset(smogResult_t* result);

#endif /* MAIN_SMOG_H_ */
