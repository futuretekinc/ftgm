TARGET=ftmqc
CFLAGS=-Wall -ggdb -I../ftm -I../ftnm -I../ftdm -I../mosquitto-1.4.1/lib 
LDFLAGS=-L../ftnm -L../ftm -L../libconfig-1.4.9/lib./libs -lftnmc -lftm -lconfig -lpthread ../mosquitto-1.4.1/lib/libmosquitto.so.1  
SRCS=main.c ftm_mqtt_client.c
OBJS=$(SRCS:.c=.o)

.PHONY: all clean


all : ${TARGET}

${TARGET} : ${OBJS}
	${CC} $^ -o $@ ${LDFLAGS}

.c.o : *.c *.h
	${CC} -c $^ -o $@ ${CFLAGS}

clean : 
	-rm -f *.o main.o
