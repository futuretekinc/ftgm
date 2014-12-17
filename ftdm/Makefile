#CC=armv5-linux-gcc
#AR=armv5-linux-ar
#CFLAGS=-c -fPIC -Wall -DDEBUG -I../devtools/include
#LDFLAGS=-L../devtools/lib -lconfig -lftsom
CC=gcc
CFLAGS=-c -Wall -DDEBUG -I/usr/local/include -I../include
LDFLAGS= -L/usr/local/lib -L. -lconfig  -lftdm -lsqlite3

TARGET=ftdm
TARGET_SRCS=main.c 
TARGET_OBJS=$(TARGET_SRCS:.c=.o)

LIB_TARGET=libftdm.so.1.0.1
LIB_INCS= libftdm.h	sqlite_if.h
LIB_SRCS= libftdm.c sqlite_if.c
LIB_OBJS=$(LIB_SRCS:.c=.o)


all: $(LIB_TARGET) $(TARGET)

$(TARGET): $(TARGET_OBJS)
	$(CC) -o $@ $(TARGET_OBJS) $(LDFLAGS)

$(LIB_TARGET): $(LIB_OBJS)
	$(AR) rcs lib$(TARGET).a $(LIB_OBJS)
	$(CC) -shared -Wl,-soname,lib$(TARGET).so.1 -o lib$(TARGET).so.1.0.1 $(LIB_OBJS)

main.o: main.c libftdm.o libftdm.h
	$(CC) $(CFLAGS) $< -o $@

libftdm.o: libftdm.c libftdm.h
	$(CC) $(CFLAGS) $< -o $@

.c.o: 
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(TARGET) *.o
