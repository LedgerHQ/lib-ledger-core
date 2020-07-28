/*
This code is written by kerukuro for cppcrypto library (http://cppcrypto.sourceforge.net/)
and released into public domain.
*/

#ifndef CPPCRYPTO_PORTABILITY_H
#define CPPCRYPTO_PORTABILITY_H

#include <stdint.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <Windows.h>
#define swap_uint64 _byteswap_uint64
#define swap_uint32 _byteswap_ulong
#define swap_uint16 _byteswap_ushort
#define aligned_allocate(a, b) _aligned_malloc(a, b)
#define aligned_deallocate _aligned_free
#define rotater32 _rotr
#define rotatel32 _rotl
#define rotater64 _rotr64
#define rotatel64 _rotl64
#define FASTCALL __fastcall
#define CPPCRYPTO_STATIC_ALIGN(x) __declspec(align(x))
#define zero_memory(a, b) SecureZeroMemory(a, b)

#ifdef CPPCRYPTODLL_EXPORT
#define CPPCRYPTOAPI __declspec(dllexport)
#elif defined(CPPCRYPTODLL)
#define CPPCRYPTOAPI __declspec(dllimport)
#else
#define CPPCRYPTOAPI
#endif
#else
#define CPPCRYPTO_STATIC_ALIGN(x) __attribute__((aligned(x)))
#ifdef __SUNPRO_CC
#define NO_BIND_TO_FUNCTION
#define NO_CPP11_THREADS
static inline uint16_t swap_uint16(uint16_t val)
{
    return ((val & 0xff) << 8) | ((val & 0xff00) >> 8);
}
static inline uint32_t swap_uint32(uint32_t val)
{
    return (((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >> 8) | ((val & 0x0000ff00) << 8) | ((val & 0x000000ff) << 24));
}
static inline uint64_t swap_uint64(uint64_t val)
{
    return (((val & 0xff00000000000000ull) >> 56) |
	((val & 0x00ff000000000000ull) >> 40) |
	((val & 0x0000ff0000000000ull) >> 24) |
	((val & 0x000000ff00000000ull) >> 8) |
	((val & 0x00000000ff000000ull) << 8) |
	((val & 0x0000000000ff0000ull) << 24) |
	((val & 0x000000000000ff00ull) << 40) |
	((val & 0x00000000000000ffull) << 56));
}
#else
#define swap_uint64 __builtin_bswap64
#define swap_uint32 __builtin_bswap32
#define swap_uint16 __builtin_bswap16
#endif
#define FASTCALL
#define CPPCRYPTOAPI

#if defined(__clang__) || defined(__SUNPRO_CC)
static inline uint32_t rotater32(uint32_t x, unsigned n)
{
	return (x >> n) | (x << (32 - n));
}
static inline uint32_t rotatel32(uint32_t x, unsigned n)
{
	return (x << n) | (x >> (32 - n));
}
#else
#include <x86intrin.h>
#define rotater32 _rotr
#define rotatel32 _rotl
#endif

static inline uint64_t rotater64(uint64_t x, unsigned n)
{
	return (x >> n) | (x << (64 - n));
}

static inline uint64_t rotatel64(uint64_t x, unsigned n)
{
	return (x << n) | (x >> (64 - n));
}

#ifdef __linux__
#include <boost/align/aligned_alloc.hpp>
#define aligned_allocate(a, b) boost::alignment::aligned_alloc(b, a)
#define aligned_deallocate free
#else
#ifdef __MINGW32__
#define NO_CPP11_THREADS
#define aligned_allocate(a, b) _aligned_malloc(a, b)
#define aligned_deallocate _aligned_free
#else
static inline void* aligned_allocate(size_t a, size_t b)
{
	void* aPtr;
	if (posix_memalign (&aPtr, b, a))
		aPtr = NULL;
	return aPtr;
}
#define aligned_deallocate free
#endif
#endif


#if defined(__APPLE__) && defined(__MACH__)
#define zero_memory(a, b) memset_s(a, b, 0, b)
//#elif defined(__linux__)
//#define zero_memory(a, b) memzero_explicit(a, b)
#else
static inline void zero_memory(void *v, size_t n) {
	volatile unsigned char *p = (volatile unsigned char *)v;
	while (n--) {
		*p++ = 0;
	}
}
#endif


#endif

#endif

