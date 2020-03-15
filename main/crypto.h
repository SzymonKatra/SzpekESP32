/*
 * crypt.h
 *
 *  Created on: 15 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_CRYPTO_H_
#define MAIN_CRYPTO_H_

#include <stdint.h>
#include <stdlib.h>

void cryptoInit(const char* base64SecretKey);

void cryptoSignature(const unsigned char* data, size_t length, unsigned char signature[32]);
void cryptoJSONPack(const char* data, size_t length, char* result);

#endif /* MAIN_CRYPTO_H_ */
