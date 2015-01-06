#CC=armv5-linux-gcc
#AR=armv5-linux-ar
#CFLAGS=-c -fPIC -Wall -DDEBUG -I../devtools/include
#LDFLAGS=-L../devtools/lib -lconfig -lftsom
CC=gcc
CFLAGS=-c -Wall -DDEBUG -I/usr/local/include -I../include
LDFLAGS= -L/usr/local/lib -L. -lconfig  -lftdm -lsqlite3 -lpthread 

CONSOLE=ftdm_console
CONSOLE_SRCS=ftdm_console.c ftdm_cmd.c
CONSOLE_OBJS=$(CONSOLE_SRCS:.c=.o)
CONSOLE_LDFLAGS=-L/usr/local/lib -L. -lconfig  -lftdm -lsqlite3 -lpthread 

SERVER=ftdm_server
SERVER_SRCS=ftdm_server_main.c ftdm_server.c sqlite_if.c
SERVER_OBJS=$(SERVER_SRCS:.c=.o)
SERVER_LDFLAGS=-L/usr/local/lib -L. -L../lib -lconfig -lftdm -lftm -lsqlite3 -lpthread 

CLIENT=ftdm_client
CLIENT_SRCS=ftdm_client_main.c  ftdm_client.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS= -L/usr/local/lib -L. -L../lib -lconfig  -lsqlite3 -lpthread -lftdmc -lftm

LIB_FTDM=libftdm
LIB_FTDM_INCS= ftdm.h sqlite_if.h
LIB_FTDM_SRCS= ftdm.c ftdm_server.c sqlite_if.c ftdm_device.c ftdm_ep.c
LIB_FTDM_OBJS=$(LIB_FTDM_SRCS:.c=.o)

LIB_FTDMC=libftdmc
LIB_FTDMC_INCS=ftdm_client.h
LIB_FTDMC_SRCS=ftdm_client.c 
LIB_FTDMC_OBJS=$(LIB_FTDMC_SRCS:.c=.o)

all: $(LIB_FTDM) $(LIB_FTDMC) $(CONSOLE) $(SERVER) $(CLIENT)

$(CONSOLE): $(CONSOLE_OBJS)
	$(CC) -o $@ $(CONSOLE_OBJS) $(CONSOLE_LDFLAGS)

$(SERVER): $(SERVER_OBJS)
	$(CC) -o $@ $(SERVER_OBJS) $(SERVER_LDFLAGS)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

$(LIB_FTDM): $(LIB_FTDM_OBJS)
	$(AR) rcs $(LIB_FTDM).a $(LIB_FTDM_OBJS)
	$(CC) -shared -Wl -o $(LIB_FTDM).so.1.0.1 $(LIB_FTDM_OBJS) $(LDFLAGS)

$(LIB_FTDMC): $(LIB_FTDMC_OBJS)
	$(AR) rcs $(LIB_FTDMC).a $(LIB_FTDMC_OBJS)
	$(CC) -shared -Wl -o $(LIB_FTDMC).so.1.0.1 $(LIB_FTDMC_OBJS) $(LDFLAGS)

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(SERVER) $(CLIENT) *.a *.o
