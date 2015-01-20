#CC=armv5-linux-gcc
#AR=armv5-linux-ar
#CFLAGS=-c -fPIC -Wall -DDEBUG -I../include -I../devtools/include -I../../ftm-50s/rootfs/apps/libconfig-1.4.9/lib -I../ftdm
#LDFLAGS=-L../devtools/lib  -L. -L../lib -L../../ftm-50s/rootfs/apps/libconfig-1.4.9/lib/.libs -L../ftdm
CC=gcc
CFLAGS=-c -Wall -DDEBUG \
		-I/usr/local/include \
		-I../include -I../ftdm \
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1
LDFLAGS= -L/usr/local/lib -L. -L../lib -L../ftdm

CLIENT=ftnm_client
CLIENT_SRCS=ftnm_client_main.c ftnm_client_config.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftnmc -lftm -lconfig -lpthread 

FTNM=ftnm
FTNM_SRCS=ftnm_main.c ftnm.c
FTNM_OBJS=$(FTNM_SRCS:.c=.o)
FTNM_LDFLAGS=$(LDFLAGS) -lftnm -lftdmc -lftm -lconfig -lpthread  -lnetsnmp

LIB_FTNM=libftnm
LIB_FTNM_INCS= ftnm.h 
LIB_FTNM_SRCS= ftnm.c \
			   ftnm_server.c\
			   ftnm_node.c\
			   ftnm_ep.c\
			   ftnm_ep_class.c\
			   ftnm_config.c\
			   ftnm_node_snmpc.c\
			   ftnm_cmds.c
LIB_FTNM_OBJS=$(LIB_FTNM_SRCS:.c=.o)

LIB_FTNMC=libftnmc
LIB_FTNMC_INCS= ftnm_client.h 
LIB_FTNMC_SRCS= ftnm_client.c 
LIB_FTNMC_OBJS=$(LIB_FTNMC_SRCS:.c=.o)

all: $(LIB_FTNM) $(FTNM) $(LIB_FTNMC) $(CLIENT)

$(CONSOLE): $(CONSOLE_OBJS)
	$(CC) -o $@ $(CONSOLE_OBJS) $(CONSOLE_LDFLAGS)

$(FTNM): $(FTNM_OBJS) $(LIB_FTNM)
	$(CC) -o $@ $(FTNM_OBJS) $(FTNM_LDFLAGS)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

$(LIB_FTNM): $(LIB_FTNM_OBJS)
	$(AR) rcs $(LIB_FTNM).a $(LIB_FTNM_OBJS)
	$(CC) -shared -o $(LIB_FTNM).so.1.0.1 $(LIB_FTNM_OBJS) $(LDFLAGS)

$(LIB_FTNMC): $(LIB_FTNMC_OBJS)
	$(AR) rcs $(LIB_FTNMC).a $(LIB_FTNMC_OBJS)
	$(CC) -shared -o $(LIB_FTNMC).so.1.0.1 $(LIB_FTNMC_OBJS) $(LDFLAGS)

.c.o:  $(SERVER_SRC) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(SERVER) $(CLIENT) *.a *.o
