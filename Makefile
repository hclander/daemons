DEBUG=-g3
CFLAGS=-std=gnu99 $(DEBUG)
#LDFLAGS=

#LDLIBS=-lmysqlclient -lpq -lodbc
LDLIBS=-lmysqlclient

LIB_SOURCES=lib/db.c lib/db.h lib/file.c lib/file.h lib/log.c lib/log.h lib/tools.c lib/tools.h lib/mem.c lib/mem.h lib/hashinttable.c lib/hashinttable.h

COMMON_SOURCES= mydb.c mydb.h frames.c frames.h framedecoder_list.c framedecoder_list.h $(LIB_SOURCES)

RXD_SOURCES=rxd.c rxd.h  $(COMMON_SOURCES)
GPSD_SOURCES=gpsd.c gpsd.h $(COMMON_SOURCES)
TXS_SOURCES=txs.c $(COMMON_SOURCES)

TXS_PRJ_SOURCES=txs_prj.c frames.c frames.h framedecoder_list.c framedecoder_list.h \
		lib/tools.c lib/tools.h lib/hashinttable.c lib/hashinttable.h lib/mem.c lib/mem.h \
		lib/bytebuffer.c lib/bytebuffer.h
		
TEST_SOURCES=test.c lib/bytebuffer.c lib/bytebuffer.h $(COMMON_SOURCES)		
		
PRJ_LIBS=-lproject -lproj

all: rxd gpsd txs txs_prj test

rxd: $(RXD_SOURCES)

gpsd: $(GPSD_SOURCES)

txs: $(TXS_SOURCES)

txs_prj: $(TXS_PRJ_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(PRJ_LIBS)

test: $(TEST_SOURCES)

clean:
	rm rxd gpsd txs txs_prj
