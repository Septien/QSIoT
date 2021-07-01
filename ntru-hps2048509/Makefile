CC=/usr/bin/gcc
LDFLAGS=-lcrypto
AR = ar rcs

SOURCES = crypto_sort.c fips202.c kem.c owcpa.c pack3.c packq.c poly.c sample.c verify.c rng.c
HEADERS = api.h crypto_sort.h fips202.h kem.h poly.h owcpa.h params.h sample.h verify.h rng.h

FLAGSPIC = -c -Wall -march=native -mtune=native -O3 -fomit-frame-pointer -fwrapv

.PHONY: clean, libntru

libntru: ntrulib
	$(AR) -o libntru.a *.o

ntrulib: $(SOURCES) $(HEADERS)
	$(CC) $(FLAGSPIC) $(SOURCES) -fpic

clean:
	-rm *.o
