#include "performance.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef RPI
#include <linux/module.h>
#include <linux/kernel.h>

#define uint64_t u_int64_t
#endif

/*
*   Get the current number of cycles.
*/
FUNC rdtsc()
{
#ifndef RPI
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t) hi << 32 | lo);
#else
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    return (uint64_t)(time.tv_sec*1e9 + time.tv_nsec);
#endif
}

void testKeyGen(int (*keygen)(unsigned char *, unsigned char*), unsigned char *pk, unsigned char *sk, struct values *keygenA)
{
#ifdef TIME
    double low, high;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    low = (double) rdtsc();
#endif
    keygen(pk, sk);
#ifdef TIME
    high = (double) rdtsc();
    gettimeofday(&end, NULL);
    keygenA -> cycles = high - low;
    keygenA -> time = (double) (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
#endif
}

void testEnc(int (*enc)(unsigned char*, unsigned char*, const unsigned char*), unsigned char *ct, unsigned char *ss, unsigned char *pk, struct values *encA)
{
#ifdef TIME
    double low, high;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    low = (double) rdtsc();
#endif
    enc(ct, ss, pk);
#ifdef TIME
    high = (double) rdtsc();
    gettimeofday(&end, NULL);
    encA -> cycles = high - low;
    encA -> time = (double) (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
#endif
}

void testDec(int (*dec)(unsigned char*, const unsigned char *, const unsigned char*), unsigned char *ss, unsigned char *ct, unsigned char *sk, struct values *decA)
{
#ifdef TIME
    double low, high;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    low = (double) rdtsc();
#endif
    dec(ss, ct, sk);
#ifdef TIME
    high = (double) rdtsc();
    gettimeofday(&end, NULL);
    decA -> cycles = high - low;
    decA -> time = (double) (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec);
#endif
}
