#CC=armv5-linux-gcc
#AR=armv5-linux-ar
#CFLAGS=-c -fPIC -Wall -DDEBUG -I../devtools/include
#LDFLAGS=-L../devtools/lib -lconfig -lftsom
CC=gcc
CFLAGS=-c -Wall -DDEBUG -I/usr/local/include -I../include
LDFLAGS=-L/usr/local/lib -L. -L../lib

SERVER=ftdm
SERVER_SRCS=ftdm_main.c ftdm_server.c ftdm_console.c
SERVER_OBJS=$(SERVER_SRCS:.c=.o)
SERVER_LDFLAGS=$(LDFLAGS) -lftdm -lftm -lconfig -lpthread -lsqlite3

CLIENT=ftdm_client
CLIENT_SRCS=ftdm_client_main.c  ftdm_client.c ftdm_client_config.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftdmc -lftm -lconfig 

LIB_FTDM=libftdm
LIB_FTDM_INCS= ftdm.h ftdm_config.h ftdm_server.h ftdm_sqlite.h ftdm_node_info.h ftdm_ep_info.h ftdm_ep_class_info.h
LIB_FTDM_SRCS= ftdm.c ftdm_config.c ftdm_server.c ftdm_sqlite.c ftdm_node_info.c ftdm_ep_info.c ftdm_ep_class_info.c
LIB_FTDM_OBJS=$(LIB_FTDM_SRCS:.c=.o)

LIB_FTDMC=libftdmc
LIB_FTDMC_INCS=ftdm_client.h
LIB_FTDMC_SRCS=ftdm_client.c 
LIB_FTDMC_OBJS=$(LIB_FTDMC_SRCS:.c=.o)

all: $(LIB_FTDM) $(LIB_FTDMC) $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_OBJS) $(LIBFTDM)
	$(CC) -o $@ $(SERVER_OBJS) $(SERVER_LDFLAGS)

$(CLIENT): $(CLIENT_OBJS) $(LIBFTDMC)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

$(LIB_FTDM): $(LIB_FTDM_OBJS)
	$(AR) rcs $(LIB_FTDM).a $(LIB_FTDM_OBJS)
	$(CC) -shared -Wl -o $(LIB_FTDM).so.1.0.1 $(LIB_FTDM_OBJS) 

$(LIB_FTDMC): $(LIB_FTDMC_OBJS)
	$(AR) rcs $(LIB_FTDMC).a $(LIB_FTDMC_OBJS)
	$(CC) -shared -Wl -o $(LIB_FTDMC).so.1.0.1 $(LIB_FTDMC_OBJS)

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(SERVER) $(CLIENT) *.a *.o
