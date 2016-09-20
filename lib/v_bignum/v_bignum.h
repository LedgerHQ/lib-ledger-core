/*
 * $Id$
 *
 * ***** BEGIN BSD LICENSE BLOCK *****
 *
 * Copyright (c) 2005-2008, The Uni-Verse Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END BSD LICENSE BLOCK *****
 *
 */

/*
 * Verse routines for big integer operations.
 * Handy in heavy encryption done during connect.
*/

#ifndef LEDGER_V_BIGNUM_H
#define LEDGER_V_BIGNUM_H

#include <limits.h>

#include "v_randgen.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------------------------- */

typedef unsigned short VBigDig;    /* Type used to hold one digit of a bignum. */
typedef unsigned int VBigDigs;    /* Should hold precisely two digits. */

#define    V_BIGBITS    (CHAR_BIT * sizeof (VBigDig))

/* Use this macro to initialize big number variables, like so:
 * VBigDig BIGNUM(foo, 128), BIGNUM(bar, 256);
 * Creates automatic variables 'foo' of 128 bits, and 'bar' of 256.
 *
 * Note that 'bits' must be a multiple of V_BIGBITS, completely
 * arbitrary number sizes are not supported by this module.
*/
#define    VBIGNUM(n, bits)    n[1 + (bits / V_BIGBITS)] = { bits / V_BIGBITS }

/* ----------------------------------------------------------------------------------------- */

/* Import/export numbers from raw bits. The number x must have been allocated
 * with the desired number of bits to read/write.
*/
void v_bignum_raw_import(VBigDig *x, const void *bits);

void v_bignum_raw_export(const VBigDig *x, void *bits);

/* Initializers. */
void v_bignum_set_zero(VBigDig *x);

void v_bignum_set_one(VBigDig *x);

void v_bignum_set_digit(VBigDig *x, VBigDig y);

void v_bignum_set_string(VBigDig *x, const char *string);    /* Decimal. */
void v_bignum_set_string_hex(VBigDig *x, const char *string);

void v_bignum_set_bignum(VBigDig *x, const VBigDig *y);

/* x = <bits> most significant <bits> bits of <y>, starting at <msb>. Right-
 * adjusted in x, so that e.g. y=0xcafebabec001 msb=47 bits=16 gives x=0xcafe.
*/
void v_bignum_set_bignum_part(VBigDig *x, const VBigDig *y,
                              unsigned int msb, unsigned int bits);

void v_bignum_set_random(VBigDig *x, VRandGen *gen);

/* Handy during debugging. */
void v_bignum_print_hex(const VBigDig *x);

void v_bignum_print_hex_lf(const VBigDig *x);

/* Bit operators. */
void v_bignum_not(VBigDig *x);

int v_bignum_bit_test(const VBigDig *x, unsigned int bit);

void v_bignum_bit_set(VBigDig *x, unsigned int bit);

int v_bignum_bit_msb(const VBigDig *x);

int v_bignum_bit_size(const VBigDig *x);

void v_bignum_bit_shift_left(VBigDig *x, unsigned int count);

void v_bignum_bit_shift_left_1(VBigDig *x);

void v_bignum_bit_shift_right(VBigDig *x, unsigned int count);

/* Comparators. */
int v_bignum_eq_zero(const VBigDig *x);            /* x == 0. */
int v_bignum_eq_one(const VBigDig *x);            /* x == 1. */
int v_bignum_eq(const VBigDig *x, const VBigDig *y);    /* x == y. */
int v_bignum_gte(const VBigDig *x, const VBigDig *y);    /* x >= y. */

/* Number vs single-digit arithmetic. */
void v_bignum_add_digit(VBigDig *x, VBigDig y);    /* x += y. */
void v_bignum_sub_digit(VBigDig *x, VBigDig y);    /* x -= y. */
void v_bignum_mul_digit(VBigDig *x, VBigDig y);    /* x *= y. */

/* Arithmetic. */
void v_bignum_add(VBigDig *x, const VBigDig *y);    /* x += y. */
void v_bignum_sub(VBigDig *x, const VBigDig *y);    /* x -= y. */
void v_bignum_mul(VBigDig *x, const VBigDig *y);    /* x *= y. */
void v_bignum_div(VBigDig *x, const VBigDig *y, VBigDig *remainder);

void v_bignum_mod(VBigDig *x, const VBigDig *y);    /* x %= y. */

/* Barrett reducer for fast x % m computation. Requires precalcing step. */
const VBigDig *v_bignum_reduce_begin(const VBigDig *m);

void v_bignum_reduce(VBigDig *x, const VBigDig *m, const VBigDig *mu);

void v_bignum_reduce_end(const VBigDig *mu);

/* Compute x *= x, assuming x only uses half of its actual size. */
void v_bignum_square_half(VBigDig *x);

/* Compute pow(x, y, n) == (x raised to the y:th power) modulo n. */
void v_bignum_pow_mod(VBigDig *x, const VBigDig *y, const VBigDig *n);

#ifdef __cplusplus
}
#endif

#endif