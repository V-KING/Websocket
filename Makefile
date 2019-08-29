# Edit by liubaolong@20190828
# make pc
# make libwebsocket_arm CROSS_COMPILE=/gcc_path/arm-linux-

CROSS_COMPILE ?= /home/hhh/petalinux_tool/opt/pkg/petalinux/2018.3/tools/linux-i386/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-

PLATFORM?=ARM
ifeq ($(PLATFORM) , PC)
CROSS_COMPILE :=
CFLAGS = -DTEST_MAIN_PC
endif

CC = $(CROSS_COMPILE)gcc
CXX= $(CROSS_COMPILE)g++
# CC 		= gcc
# CXX 		= g++

CFLAGS += -g -std=gnu9x -w -DRUPIFY -DINVG_RELEASE
# CFLAGS += -g -std=gnu9x -w -DRUPIFY

INCL 	= Handshake.c
OBJECTS = Errors.o Datastructures.o Communicate.o sha1.o md5.o base64.o utf8.o
EXEC 	= Websocket

.PHONY: Websocket clean valgrind 

all: Websocket

Websocket: $(OBJECTS) 
	$(CC) $(CFLAGS) $(INCL) $(OBJECTS) -lpthread $(EXEC).c -o $(EXEC) -std=c99

clean:
	rm -f $(EXEC) *.o libwebsocket.a

run:
	./$(EXEC)

valgrind:
	valgrind --leak-check=full --log-file="valgrind.log" --track-origins=yes --show-reachable=yes ./$(EXEC)

base64.o: base64.c base64.h
	$(CC) $(CFLAGS) -c base64.c

md5.o: md5.c md5.h
	$(CC) $(CFLAGS) -c md5.c

utf8.o: utf8.c utf8.h
	$(CC) $(CFLAGS) -c utf8.c

sha1.o: sha1.c sha1.h
	$(CC) $(CFLAGS) -c sha1.c

Communicate.o: Communicate.c Communicate.h Datastructures.h
	$(CC) $(CFLAGS) -c Communicate.c

Datastructures.o: Datastructures.c Datastructures.h
	$(CC) $(CFLAGS) -c Datastructures.c

Errors.o: Errors.c Errors.h Datastructures.h
	$(CC) $(CFLAGS) -c Errors.c

libwebsocket_arm:
	make clean
	make -f Makefile_for_lib.mk CROSS_COMPILE=$(CROSS_COMPILE) V=1
libwebsocket_pc:
	make clean
	make -f Makefile_for_lib.mk V=1
test:
	echo $(CC)
pc:
	make all PLATFORM=PC