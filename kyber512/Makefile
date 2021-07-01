CC = gcc
AR = ar rcs

SOURCESLIB = verify.c symmetric-fips202.c sha512.c sha256.c rng.c reduce.c randombytes.c polyvec.c poly.c ntt.c kex.c kem.c indcpa.c fips202.c cbd.c aes256ctr.c 
HEADERS = verify.h symmetric.h sha2.h rng.h reduce.h randombytes.h polyvec.h poly.h params.h ntt.h kex.h indcpa.h fips202.h cbd.h api.h aes256ctr.h
FLAGSPIC = -c -Wall -march=native -mtune=native -O3 -fomit-frame-pointer -fwrapv

.PHONY: clean, libkyber

libkyber: kyberlib
	$(AR) libkyber.a *.o 

kyberlib: $(SOURCESLIB) $(HEADERS)
	$(CC) $(FLAGSPIC) $(SOURCESLIB) -fpic

clean:
	-rm *.o
