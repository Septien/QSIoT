#CC = clang
CC = gcc
LDFLAGS = -lcrypto
AR = ar rcs

SOURCESLIB = pack_unpack.c poly.c rng.c fips202.c verify.c cbd.c SABER_indcpa.c kem.c
HEADERS = SABER_params.h pack_unpack.h poly.h rng.h fips202.h verify.h cbd.h SABER_indcpa.h kem.h 
FLAGSPIC = -c -Wall -march=native -mtune=native -O3 -fomit-frame-pointer -fwrapv

.PHONY: clean, libsaber

libsaber: saberlib
	$(AR) libsaber.a *.o

saberlib: $(SOURCESLIB) $(HEADERS)
	$(CC) $(FLAGSPIC) $(SOURCESLIB)

clean:
	-rm *.o
