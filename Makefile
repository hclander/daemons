DEBUG=-g3
CFLAGS=-std=c99 -D_GNU_SOURCE $(DEBUG)
#LDFLAGS=

LDLIBS=-lmysqlclient -lpq -lodbc

LIB_SOURCES=lib/db.c lib/db.h lib/file.c lib/file.h lib/log.c lib/log.h lib/tools.c lib/tools.h lib/hashinttable.c lib/hashinttable.h

COMMON_SOURCES= mydb.c mydb.h frames.c frames.h $(LIB_SOURCES)

RXD_SOURCES=rxd.c rxd.h  $(COMMON_SOURCES)
GPSD_SOURCES=gpsd.c gpsd.h $(COMMON_SOURCES)
TXS_SOURCES=txs.c $(COMMON_SOURCES)

TXS_PRJ_SOURCES=txs_prj.c frames.c frames.h lib/tools.c lib/tools.h 
PRJ_LIBS=-lproject -lproj

all: rxd gpsd txs txs_prj

rxd: $(RXD_SOURCES)

gpsd: $(GPSD_SOURCES)

txs: $(TXS_SOURCES)

txs_prj: $(TXS_PRJ_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(PRJ_LIBS)

clean:
	rm rxd gpsd txs
