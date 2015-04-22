CFLAGS=-Wall -ggdb
LDFLAGS=../../lib/libmosquitto.so.1 

.PHONY: all clean

all : mqtt_test

mqtt_test : main.o
	${CC} $^ -o $@ ${LDFLAGS}

main.o : main.c
	${CC} -c $^ -o $@ ${CFLAGS} -I../../lib

clean : 
	-rm -f *.o main.o
