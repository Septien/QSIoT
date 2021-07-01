/**
 * Test the performance of the different mechanisms in CPU usage or RAM usage, accorging to the
 * selected option. When compiling with the Makefile, use the following options
 *      -Set TIME=1 for measuring the CPU usage.
 *      -Set MEMORY=1 for measuring the RAM usage.
 *      -Set RPI=1 if the tests are to be done on a RPI device. 
 * Select an appropiate mechanism for performance measurment:
 *      -Set NTRU=1 for selecting NTRUhps2048509.
 *      -Set NTRUP=1 for selecting NTRULPr653. 
 *      -Set SABER=1 for selecting LightSaber.
 *      -Set KYBER=1 for selecting Kyber512.
 *      -Set FRODO=1 for selecting FrodoKEM-640.
 * For this program to work, it is assumed that a static library from the selected mechanism is present 
 * in the same location as this file, and the api.h file is present in folders described bellow.
 * When measuring the CPU performance, you should pass a csv file when running the program. In this file, 
 * the values of the execution times will be stored.
 * 
 * When measuring CPU usage, use the following command:
 *  make test KEM=1 TIME=1 [RPI=1]
 *  ./test output.csv
 * When measuring RAM usage, use the following command:
 *  make test KEM=1 MEMORY=1 DEBUG=1
 *  valgrind --tool=massif --stacks=yes --time-unit=B --massiff-out-file=outputfile test
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

// Crypto libraries
#ifdef NTRU
#include "ntru-hps2048509/api.h"
#endif

#ifdef NTRUP
#include "ntrulpr653/api.h"
#include "ntrulpr653/crypto_kem.h"
#endif

#ifdef SABER
#include "lightsaber/api.h"
#endif

#ifdef  KYBER
#include "kyber512/api.h"
#endif

#ifdef FRODO
#include "FrodoKEM-640/api.h"
#endif

#include "performance.h"

#ifdef RPI
#define uint64_t u_int64_t
#endif

void computeMean(int N, struct values **means, struct values **keygen, struct values **dec, struct values **enc)
{
    means[0]->cycles = means[0]->time = 0;
    means[1]->cycles = means[1]->time = 0;
    means[2]->cycles = means[2]->time = 0;
    for (int i = 0; i < N; i++)
    {
        means[0]->cycles += keygen[i]->cycles;
        means[0]->time += keygen[i]->time;
        means[1]->cycles += enc[i]->cycles;
        means[1]->time += enc[i]->time;
        means[2]->cycles += dec[i]->cycles;
        means[2]->time += dec[i]->time;
    }
    means[0]->cycles /= N;
    means[0]->time /= N;
    means[1]->cycles /= N;
    means[1]->time /= N;
    means[2]->cycles /= N;
    means[2]->time /= N;
}

void measureTimeKEM(int N, struct values **means, struct values **keygen, struct values **dec, struct values **enc)
{
    // For measuring time
    int i;

    // For the scheme
    unsigned char pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES], ss[CRYPTO_BYTES], ct[CRYPTO_CIPHERTEXTBYTES];
    struct values *keygenA = NULL, *encA = NULL, *decA = NULL;

    for (i = 0; i < N; i++)
    {
#ifdef TIME
        keygenA = keygen[i];
        encA = enc[i];
        decA = dec[i];
#endif
        // Key generation
        testKeyGen(crypto_kem_keypair, pk, sk, keygenA);
        // Encapsulation
        testEnc(crypto_kem_enc, ct, ss, pk, encA);
        // Decapsulation
        testDec(crypto_kem_dec, ss, ct, sk, decA);
    }

#ifdef TIME
    computeMean(N, means, keygen, dec, enc);
#endif
}

void makeTest(int N, struct values **means, struct values **keygen, struct values **dec, struct values **enc, char *file)
{
    measureTimeKEM(N, means, keygen, dec, enc);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
#ifdef TIME
        printf("Provide the name for the output file: output.csv\n");
#endif
        return 0;
    }

    struct values **keygen = NULL, **enc = NULL, **dec = NULL, **means = NULL;
    int N = 1, i, j;
    char *file = NULL;

#ifdef TIME
    N = 2000;
    keygen = (struct values **)malloc(N * sizeof(struct values *));
    enc = (struct values **)malloc(N * sizeof(struct values *));
    dec = (struct values **)malloc(N * sizeof(struct values *));
    means = (struct values **)malloc(3 * sizeof(struct values *));

    /* Allocate memory for each entry */
    for (j = 0; j < N; j++)
    {
        keygen[j] = (struct values *) malloc (sizeof(struct values));
        enc[j] = (struct values *) malloc (sizeof(struct values));
        dec[j] = (struct values *) malloc (sizeof(struct values));
    }
    means[0] = (struct values *)malloc(sizeof(struct values));
    means[1] = (struct values *)malloc(sizeof(struct values));
    means[2] = (struct values *)malloc(sizeof(struct values));
#endif

    makeTest(N, means, keygen, dec, enc, file);

#ifdef TIME
    printf("Mean for the KeyGen function:\n\t%f\t%f\n", means[0]->cycles, means[0]->time);
    printf("Mean for the Enc function:\n\t%f\t%f\n", means[1]->cycles, means[1]->time);
    printf("Mean for the Dec function:\n\t%f\t%f\n", means[2]->cycles, means[2]->time);

    FILE *pFile;
    pFile = fopen(argv[1], "w");

    fprintf(pFile, "KeyGen (uS), Enc (uS), Dec (uS)\n");
    for (i = 0; i < N - 1; i++)
        fprintf(pFile, "%f,", keygen[i]->time);
    fprintf(pFile, "%f\n", keygen[N-1]->time);

    for (i = 0; i < N - 1; i++)
        fprintf(pFile, "%f,", enc[i]->time);
    fprintf(pFile, "%f\n", enc[N-1]->time);

    for (i = 0; i < N - 1; i++)
        fprintf(pFile, "%f,", dec[i]->time);
    fprintf(pFile, "%f\n", dec[N-1]->time);

    fprintf(pFile, "KeyGen (cycles), Enc (cycles), Dec (cycles)\n");
    for (i = 0; i < N - 1; i++)
        fprintf(pFile, "%f,", keygen[i]->cycles);
    fprintf(pFile, "%f\n", keygen[N-1]->cycles);

    for (i = 0; i < N - 1; i++)
        fprintf(pFile, "%f,", enc[i]->cycles);
    fprintf(pFile, "%f\n", enc[N-1]->cycles);

    for (i = 0; i < N - 1; i++)
        fprintf(pFile, "%f,", dec[i]->cycles);
    fprintf(pFile, "%f\n", dec[N-1]->cycles);

    for (j = 0; j < N; j++)
    {
        free(keygen[j]);
        free(enc[j]);
        free(dec[j]);
    }
    free(keygen);
    free(enc);
    free(dec);
    fclose(pFile);
#endif
    return 0;
}