/*
Copyright(C) thecodeway.com
*/
#include <cy_crypt.h>
#include "cyr_adler32.h"

/* adler32.c -- compute the Adler-32 checksum of a data stream
* Copyright (C) 1995-2011 Mark Adler
* For conditions of distribution and use, see copyright notice in zlib.h
*/
namespace cyclone
{
#define BASE 65521      /* largest prime smaller than 65536 */
#define NMAX 5552

	/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {adler += (uint32_t)(buf)[i]; sum2 += adler;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

	/* use NO_DIVIDE if your processor does not do division in hardware --
	try it both ways to see which is faster */
#ifdef NO_DIVIDE
	/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
	(thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
	do {
	\
	unsigned long tmp = a >> 16; \
	a &= 0xffffUL; \
	a += (tmp << 4) - tmp; \
	} while (0)
#  define MOD28(a) \
	do {
	\
	CHOP(a); \
	if (a >= BASE) a -= BASE; \
	} while (0)
#  define MOD(a) \
	do {
	\
	CHOP(a); \
	MOD28(a); \
	} while (0)
#  define MOD63(a) \
	do { /* this assumes a is not negative */ \
	z_off64_t tmp = a >> 32; \
	a &= 0xffffffffL; \
	a += (tmp << 8) - (tmp << 5) + tmp; \
	tmp = a >> 16; \
	a &= 0xffffL; \
	a += (tmp << 4) - tmp; \
	tmp = a >> 16; \
	a &= 0xffffL; \
	a += (tmp << 4) - tmp; \
	if (a >= BASE) a -= BASE; \
	} while (0)
#else
#  define MOD(a) a %= BASE
#  define MOD28(a) a %= BASE
#  define MOD63(a) a %= BASE
#endif
uint32_t adler32(uint32_t adler, const uint8_t* buf, size_t len)
{
	uint32_t sum2;
    unsigned n;
	
	/* initial Adler-32 value */
	if (buf == 0 || len == 0)
		return INITIAL_ADLER;
	
	/* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    if (len == 1) {
        adler += (uint32_t)buf[0];
        if (adler >= BASE)
            adler -= BASE;
        sum2 += adler;
        if (sum2 >= BASE)
            sum2 -= BASE;
        return adler | (sum2 << 16);
    }

    /* in case short lengths are provided, keep it somewhat fast */
    if (len < 16) {
        while (len--) {
			adler += (uint32_t)(*buf++);
            sum2 += adler;
        }
        if (adler >= BASE)
            adler -= BASE;
        MOD28(sum2);            /* only added so many BASE's */
        return adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    while (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        do {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } while (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    if (len) {                  /* avoid modulos if none remaining */
        while (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        while (len--) {
			adler += (uint32_t)(*buf++);
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    return adler | (sum2 << 16);
}

}
