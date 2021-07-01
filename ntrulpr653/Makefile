CC = gcc
AR = ar rcs

SOURCESLIB = uint32_sort.c uint32.c sha512.c kem.c int32.c Encode.c Decode.c aes256ctr.c nist/rng.c
HEADERS = uint64.h uint32.h uint16.h sha512.h randombytes.h paramsmenu.h params.h int8.h int32.h int16.h Encode.h Decode.h crypto_kem_ntrulpr653.h crypto_kem.h api.h aes256ctr.h nist/rng.h
FLAGSPIC = -c -Wall -march=native -mtune=native -O3 -fomit-frame-pointer -fwrapv

.PHONY: clean, libntrup

libntrup: ntruplib
	$(AR) -o libntrup.a *.o

ntruplib: $(SOURCESLIB) $(HEADERS)
	$(CC) $(FLAGSPIC) $(SOURCESLIB) -fpic

clean:
	-rm *.o
