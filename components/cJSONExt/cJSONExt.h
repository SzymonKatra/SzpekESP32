/*
 * cJSONExt.h
 *
 *  Created on: 5 kwi 2020
 *      Author: szymo
 */

#ifndef COMPONENTS_CJSONEXT_CJSONEXT_H_
#define COMPONENTS_CJSONEXT_CJSONEXT_H_

#include <cJSON.h>
#include <time.h>
#include <stdbool.h>

void cJSONExt_AddDateTimeToObject(cJSON* const object, const char* const name, const time_t timestamp);

bool cJSONExt_TryReadString(const cJSON* const jsonObject, const char* const fieldName, char* result);
bool cJSONExt_TryReadNumber(const cJSON* const jsonObject, const char* const fieldName, double* result);
bool cJSONExt_TryReadTimestamp(const cJSON* const jsonObject, const char* const fieldName, time_t* result);


#endif /* COMPONENTS_CJSONEXT_CJSONEXT_H_ */
