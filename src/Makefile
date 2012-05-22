# Makefile for httpsqs
CC=g++
CFLAGS=-L/usr/local/libevent-2.0.10-stable/lib/ -levent -I./leveldb/include -L./leveldb -lleveldb -I/usr/local/libevent-2.0.10-stable/include/ -lz  -lrt -lpthread -lm -lc -g 

llserver: llserver.c
	$(CC) -o llserver llserver.c handler.c utils.c llthread.c mc_init.c dbhelper.c hstring.c dlinklist.c linklist.c $(CFLAGS)
	@echo ""x
	@echo "llserver build complete."
	@echo ""	

clean: llserver
	rm -f llserver

install: llserver
	install $(INSTALL_FLAGS) -m 4755 -o root llserver $(DESTDIR)/usr/bin
