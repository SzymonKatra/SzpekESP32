/*
 * url_encoding.h
 *
 *  Created on: 21 mar 2020
 *      Author: szymo
 */

#ifndef COMPONENTS_URL_ENCODING_URL_ENCODING_H_
#define COMPONENTS_URL_ENCODING_URL_ENCODING_H_

#include <stddef.h>

void urlEncode(const char* input, char* output, size_t size);
void urlDecode(const char* input, char* output, size_t size);

#endif /* COMPONENTS_URL_ENCODING_URL_ENCODING_H_ */
