#include "crypto.h"

#include <mbedtls/sha256.h>
#include <mbedtls/rsa.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <stdio.h>
#include <string.h>
#include "logUtils.h"


#define CRYPTO_SECRET_LEN (32)

static const char* LOG_TAG = "Crypto";

static mbedtls_md_context_t s_mdCtx;
static uint8_t s_key[CRYPTO_SECRET_LEN];

void cryptoInit(const char* base64Secret)
{
    mbedtls_md_init(&s_mdCtx);
    mbedtls_md_setup(&s_mdCtx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);

    size_t olen;
    int result = mbedtls_base64_decode(s_key, CRYPTO_SECRET_LEN, &olen, (const unsigned char*)base64Secret, strlen(base64Secret));
    if (result != 0)
    {
        switch (result)
        {
        case MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL: LOG_ERROR("Secret key is too big. Secret key is 32 bytes encoded in Base64. mbedtls error code = %d", result); break;
        case MBEDTLS_ERR_BASE64_INVALID_CHARACTER: LOG_ERROR("Incorrectly encoded secret key. Secret key is 32 bytes encoded in Base64. mbedtls error code = %d", result); break;
        }
    }
    if (olen != CRYPTO_SECRET_LEN)
    {
        LOG_ERROR("Secret key has wrong size. Must be %d bytes but is %d bytes", CRYPTO_SECRET_LEN, olen);
    }
}

void cryptoSignature(const unsigned char* data, size_t length, unsigned char signature[32])
{
    mbedtls_md_hmac_starts(&s_mdCtx, (const unsigned char*)s_key, CRYPTO_SECRET_LEN);
    mbedtls_md_hmac_update(&s_mdCtx, data, length);
    mbedtls_md_hmac_finish(&s_mdCtx, signature);
}

void cryptoSignatureBase64(const unsigned char* data, size_t length, unsigned char base64Signature[CRYPTO_BASE64_BUFFER_LEN(32)])
{
	unsigned char signature[32];
	cryptoSignature(data, length, signature);

	size_t base64SigLen;
	mbedtls_base64_encode((unsigned char*)base64Signature, CRYPTO_BASE64_BUFFER_LEN(32), &base64SigLen, signature, 32);
}

