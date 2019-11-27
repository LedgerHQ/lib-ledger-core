/*
 * Copyright 2016-2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Derived from the BLAKE2 reference implementation written by Samuel Neves.
 * Copyright 2012, Samuel Neves <sneves@dei.uc.pt>
 * More information about the BLAKE2 hash function and its implementations
 * can be found at https://blake2.net.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../../include/openssl/e_os2.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define BLAKE2S_BLOCKBYTES    64
#define BLAKE2S_OUTBYTES      32
#define BLAKE2S_KEYBYTES      32
#define BLAKE2S_SALTBYTES     8
#define BLAKE2S_PERSONALBYTES 8

#define BLAKE2B_BLOCKBYTES    128
#define BLAKE2B_OUTBYTES      64
#define BLAKE2B_KEYBYTES      64
#define BLAKE2B_SALTBYTES     16
#define BLAKE2B_PERSONALBYTES 16

struct blake2s_param_st {
    uint8_t  digest_length; /* 1 */
    uint8_t  key_length;    /* 2 */
    uint8_t  fanout;        /* 3 */
    uint8_t  depth;         /* 4 */
    uint8_t  leaf_length[4];/* 8 */
    uint8_t  node_offset[6];/* 14 */
    uint8_t  node_depth;    /* 15 */
    uint8_t  inner_length;  /* 16 */
    uint8_t  salt[BLAKE2S_SALTBYTES]; /* 24 */
    uint8_t  personal[BLAKE2S_PERSONALBYTES];  /* 32 */
};

typedef struct blake2s_param_st BLAKE2S_PARAM;

struct blake2s_ctx_st {
    uint32_t h[8];
    uint32_t t[2];
    uint32_t f[2];
    uint8_t  buf[BLAKE2S_BLOCKBYTES];
    size_t   buflen;
    size_t   outlen;
};

struct blake2b_param_st {
    uint8_t  digest_length; /* 1 */
    uint8_t  key_length;    /* 2 */
    uint8_t  fanout;        /* 3 */
    uint8_t  depth;         /* 4 */
    uint8_t  leaf_length[4];/* 8 */
    uint8_t  node_offset[8];/* 16 */
    uint8_t  node_depth;    /* 17 */
    uint8_t  inner_length;  /* 18 */
    uint8_t  reserved[14];  /* 32 */
    uint8_t  salt[BLAKE2B_SALTBYTES]; /* 48 */
    uint8_t  personal[BLAKE2B_PERSONALBYTES];  /* 64 */
};

typedef struct blake2b_param_st BLAKE2B_PARAM;

struct blake2b_ctx_st {
    uint64_t h[8];
    uint64_t t[2];
    uint64_t f[2];
    uint8_t  buf[BLAKE2B_BLOCKBYTES];
    size_t   buflen;
    size_t   outlen;
};

#define BLAKE2B_DIGEST_LENGTH 64
#define BLAKE2S_DIGEST_LENGTH 32

typedef struct blake2s_ctx_st BLAKE2S_CTX;
typedef struct blake2b_ctx_st BLAKE2B_CTX;

int BLAKE2b_Init(BLAKE2B_CTX *c, const BLAKE2B_PARAM *P);
int BLAKE2b_Init_default(BLAKE2B_CTX *c, size_t outlen);
int BLAKE2b_Init_key(BLAKE2B_CTX *c, const BLAKE2B_PARAM *P, const void *key);
int BLAKE2b_Update(BLAKE2B_CTX *c, const void *data, size_t datalen);
int BLAKE2b_Final(unsigned char *md, BLAKE2B_CTX *c);

/*
 * These setters are internal and do not check the validity of their parameters.
 * See blake2b_mac_ctrl for validation logic.
 */

void blake2b_param_init(BLAKE2B_PARAM *P);
void blake2b_param_set_digest_length(BLAKE2B_PARAM *P, uint8_t outlen);
void blake2b_param_set_key_length(BLAKE2B_PARAM *P, uint8_t keylen);
void blake2b_param_set_personal(BLAKE2B_PARAM *P, const uint8_t *personal, size_t length);
void blake2b_param_set_salt(BLAKE2B_PARAM *P, const uint8_t *salt, size_t length);

int BLAKE2s_Init(BLAKE2S_CTX *c, const BLAKE2S_PARAM *P);
int BLAKE2s_Init_key(BLAKE2S_CTX *c, const BLAKE2S_PARAM *P, const void *key);
int BLAKE2s_Update(BLAKE2S_CTX *c, const void *data, size_t datalen);
int BLAKE2s_Final(unsigned char *md, BLAKE2S_CTX *c);

void blake2s_param_init(BLAKE2S_PARAM *P);
void blake2s_param_set_digest_length(BLAKE2S_PARAM *P, uint8_t outlen);
void blake2s_param_set_key_length(BLAKE2S_PARAM *P, uint8_t keylen);
void blake2s_param_set_personal(BLAKE2S_PARAM *P, const uint8_t *personal, size_t length);
void blake2s_param_set_salt(BLAKE2S_PARAM *P, const uint8_t *salt, size_t length);

static ossl_inline uint32_t load32(const uint8_t *src)
{
    const union {
        long one;
        char little;
    } is_endian = { 1 };

    if (is_endian.little) {
        uint32_t w;
        memcpy(&w, src, sizeof(w));
        return w;
    } else {
        uint32_t w = ((uint32_t)src[0])
        | ((uint32_t)src[1] <<  8)
        | ((uint32_t)src[2] << 16)
        | ((uint32_t)src[3] << 24);
        return w;
    }
}

static ossl_inline uint64_t load64(const uint8_t *src)
{
    const union {
        long one;
        char little;
    } is_endian = { 1 };

    if (is_endian.little) {
        uint64_t w;
        memcpy(&w, src, sizeof(w));
        return w;
    } else {
        uint64_t w = ((uint64_t)src[0])
        | ((uint64_t)src[1] <<  8)
        | ((uint64_t)src[2] << 16)
        | ((uint64_t)src[3] << 24)
        | ((uint64_t)src[4] << 32)
        | ((uint64_t)src[5] << 40)
        | ((uint64_t)src[6] << 48)
        | ((uint64_t)src[7] << 56);
        return w;
    }
}

static ossl_inline void store32(uint8_t *dst, uint32_t w)
{
    const union {
        long one;
        char little;
    } is_endian = { 1 };

    if (is_endian.little) {
        memcpy(dst, &w, sizeof(w));
    } else {
        uint8_t *p = (uint8_t *)dst;
        int i;

        for (i = 0; i < 4; i++)
            p[i] = (uint8_t)(w >> (8 * i));
    }
}

static ossl_inline void store64(uint8_t *dst, uint64_t w)
{
    const union {
        long one;
        char little;
    } is_endian = { 1 };

    if (is_endian.little) {
        memcpy(dst, &w, sizeof(w));
    } else {
        uint8_t *p = (uint8_t *)dst;
        int i;

        for (i = 0; i < 8; i++)
            p[i] = (uint8_t)(w >> (8 * i));
    }
}

static ossl_inline uint64_t load48(const uint8_t *src)
{
    uint64_t w = ((uint64_t)src[0])
    | ((uint64_t)src[1] <<  8)
    | ((uint64_t)src[2] << 16)
    | ((uint64_t)src[3] << 24)
    | ((uint64_t)src[4] << 32)
    | ((uint64_t)src[5] << 40);
    return w;
}

static ossl_inline void store48(uint8_t *dst, uint64_t w)
{
    uint8_t *p = (uint8_t *)dst;
    p[0] = (uint8_t)w;
    p[1] = (uint8_t)(w>>8);
    p[2] = (uint8_t)(w>>16);
    p[3] = (uint8_t)(w>>24);
    p[4] = (uint8_t)(w>>32);
    p[5] = (uint8_t)(w>>40);
}

static ossl_inline uint32_t rotr32(const uint32_t w, const unsigned int c)
{
    return (w >> c) | (w << (32 - c));
}

static ossl_inline uint64_t rotr64(const uint64_t w, const unsigned int c)
{
    return (w >> c) | (w << (64 - c));
}

#ifdef  __cplusplus
}
#endif