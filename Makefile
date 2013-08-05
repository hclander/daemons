DEBUG=-g3
CFLAGS=-std=c99 -D_GNU_SOURCE $(DEBUG)
LDFLAGS=-lmysqlclient -lpq -lodbc

LIB_SOURCES=lib/db.c lib/db.h lib/file.c lib/file.h lib/log.c lib/log.h lib/tools.c lib/tools.h

COMMON_SOURCES= mydb.c mydb.h frames.c frames.h $(LIB_SOURCES)

RXD_SOURCES=rxd.c rxd.h  $(COMMON_SOURCES)
GPSD_SOURCES=gpsd.c gpsd.h $(COMMON_SOURCES)
TXS_SOURCES=txs.c $(COMMON_SOURCES)

all: rxd gpsd txs

rxd: $(RXD_SOURCES)

gpsd: $(GPSD_SOURCES)

txs: $(TXS_SOURCES)

clean:
	rm rxd gpsd
