CC = /usr/bin/gcc

LDFLAGS=-L.
PERFFLAGS=-O3 -fomit-frame-pointer -march=native
CFLAGS= #-DRPI #For the raspberry pi

SOURCES=main.c performance.c
HEADERS=performance.h

# Which cryptosystem to include?
LIBFLAGS=
ifdef NTRU
	LIBFLAGS += -lntru
	HEADERS += ntru-hrss701/api.h
	CFLAGS += -DNTRU
endif
ifdef NTRUP
	LIBFLAGS += -lntrup
	HEADERS += ntrulpr653/api.h ntrulpr653/crypto_kem.h
	CFLAGS += -DNTRUP
endif
ifdef SABER
	LIBFLAGS += -lsaber
	HEADERS += saber/api.h
	CFLAGS += -DSABER
endif
ifdef KYBER
	LIBFLAGS += -lkyber
	HEADERS += kyber/api.h
	CFLAGS += -DKYBER
endif
ifdef FRODO
	LIBFLAGS += -lfrodo
	HEADERS += FrodoKEM-640/api.h
	CFLAGS += -DFRODO
endif
LIBFLAGS += -lcrypto

DEBUGF=
ifdef DEBUG
	DEBUGF = -g
endif

ifdef TIME
	CFLAGS += -DTIME
endif

ifdef MEMORY
	CFLAGS += -DMEMORY
endif

ifdef RPI
	CFLAGS += -DRPI
endif

$( info $(LIBFLAGS) )

test: $(SOURCES) $(HEADERS)
	$(CC) $(DEBUGF) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $@ $(LIBFLAGS) $(PERFFLAGS)

clean:
	rm test
