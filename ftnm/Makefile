#CC=armv5-linux-gcc
#AR=armv5-linux-ar
#CFLAGS=-c -fPIC -Wall -DDEBUG -I../devtools/include
#LDFLAGS=-L../devtools/lib -lconfig -lftsom
CC=gcc
CFLAGS=-c -Wall -DDEBUG -I/usr/local/include -I../include -I../ftdm
LDFLAGS= -L/usr/local/lib -L. 

SERVER=ftnm_server
SERVER_SRCS=ftnm_server_main.c ftnm_debug.c
SERVER_OBJS=$(SERVER_SRCS:.c=.o)
SERVER_LDFLAGS=-L/usr/local/lib -L. -lftnm -lconfig -lsqlite3 -lpthread  -lnetsnmp

LIB_FTNM=libftnm
LIB_FTNM_INCS= ftnm.h 
LIB_FTNM_SRCS= ftnm.c ftnm_server.c ftnm_object.c simclist.c ftnm_snmp_client.c ftnm_config.c
LIB_FTNM_OBJS=$(LIB_FTNM_SRCS:.c=.o)

all: $(LIB_FTNM) $(SERVER)

$(CONSOLE): $(CONSOLE_OBJS)
	$(CC) -o $@ $(CONSOLE_OBJS) $(CONSOLE_LDFLAGS)

$(SERVER): $(SERVER_OBJS) $(LIB_FTNM)
	$(CC) -o $@ $(SERVER_OBJS) $(SERVER_LDFLAGS)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

$(LIB_FTNM): $(LIB_FTNM_OBJS)
	$(AR) rcs $(LIB_FTNM).a $(LIB_FTNM_OBJS)
	$(CC) -shared -Wl -o $(LIB_FTNM).so.1.0.1 $(LIB_FTNM_OBJS) $(LDFLAGS)

$(LIB_FTNMC): $(LIB_FTNMC_OBJS)
	$(AR) rcs $(LIB_FTNMC).a $(LIB_FTNMC_OBJS)
	$(CC) -shared -Wl -o $(LIB_FTNMC).so.1.0.1 $(LIB_FTNMC_OBJS) $(LDFLAGS)

.c.o:  $(SERVER_SRC) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(SERVER) $(CLIENT) *.a *.o
