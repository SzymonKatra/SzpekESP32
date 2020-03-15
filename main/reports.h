/*
 * reports.h
 *
 *  Created on: 15 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_REPORTS_H_
#define MAIN_REPORTS_H_

#include <stdint.h>
#include <time.h>

typedef struct
{
    float pm1;
    float pm2_5;
    float pm10;
    uint32_t samplesCount;
    time_t timestampFrom;
    time_t timestampTo;
} report_t;

#endif /* MAIN_REPORTS_H_ */
