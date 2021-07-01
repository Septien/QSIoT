#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

#ifndef RPI
    #ifdef WIN32
    #include <intrin.h>
    #else
    #include <x86intrin.h>
    #endif
    #define uint64_t u_int64_t
    #define FUNC uint64_t
#else
#include <linux/module.h>
#include <linux/kernel.h>
#define uint64_t u_int64_t
#define FUNC static inline uint64_t
#endif

struct values {
    double time, cycles;
};

FUNC rdtsc();
void testKeyGen(int (*keygen)(unsigned char *, unsigned char*), unsigned char *pk, unsigned char *sk, struct values *keygenA);
void testEnc(int (*enc)(unsigned char*, unsigned char*, const unsigned char*), unsigned char *ct, unsigned char *ss, unsigned char *pk, struct values *encA);
void testDec(int (*dec)(unsigned char*, const unsigned char *, const unsigned char*), unsigned char *ss, unsigned char *ct, unsigned char *sk, struct values *decA);
#endif //PERFORMANCE_H