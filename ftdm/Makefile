#CC=armv5-linux-gcc
#AR=armv5-linux-ar
#CFLAGS=-c -fPIC -Wall -DDEBUG -I../devtools/include
#LDFLAGS=-L../devtools/lib -lconfig -lftsom
CC=gcc
CFLAGS=-c -Wall -DDEBUG -I/usr/local/include -I../include
LDFLAGS= -L/usr/local/lib -L. -lconfig  -lftdm -lsqlite3 -lpthread

TARGET=ftdm
TARGET_SRCS=ftdm_server.c ftdm_cmd.c ftdm.c
TARGET_OBJS=$(TARGET_SRCS:.c=.o)

CLIENT=ftdmc
CLIENT_SRCS=ftdmc.c ftdm_client.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)

LIB_TARGET=libftdm.so.1.0.1
LIB_INCS= ftdm.h sqlite_if.h
LIB_SRCS= ftdm.c sqlite_if.c
LIB_OBJS=$(LIB_SRCS:.c=.o)


all: $(LIB_TARGET) $(TARGET) $(CLIENT)

$(TARGET): $(TARGET_OBJS)
	$(CC) -o $@ $(TARGET_OBJS) $(LDFLAGS)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(LDFLAGS)

$(LIB_TARGET): $(LIB_OBJS)
	$(AR) rcs lib$(TARGET).a $(LIB_OBJS)
	$(CC) -shared -Wl -o lib$(TARGET).so.1.0.1 $(LIB_OBJS) $(LDFLAGS)

ftdm_server.o: ftdm_server.c ftdm.o ftdm.h
	$(CC) $(CFLAGS) $< -o $@

ftdm.o: ftdm.c ftdm.h
	$(CC) $(CFLAGS) $< -o $@

.c.o: 
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(TARGET) *.o
