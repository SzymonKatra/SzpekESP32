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

#define CRYPTO_BASE64_LEN(len)  (4 * (((len) / 3) + 1))
#define CRYPTO_BASE64_BUFFER_LEN(len) (CRYPTO_BASE64_LEN(len) + 1)

void cryptoInit(const char* base64SecretKey);

void cryptoSignature(const unsigned char* data, size_t length, unsigned char signature[32]);
void cryptoSignatureBase64(const unsigned char* data, size_t length, unsigned char base64Signature[CRYPTO_BASE64_BUFFER_LEN(32)]);

#endif /* MAIN_CRYPTO_H_ */
