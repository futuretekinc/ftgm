TOPDIR=../..
include ${TOPDIR}/Makefile.in

CLIENT=ftdmc
CLIENT_SRCS=main.c  ftdm_client.c ftdm_client_config.c ftdm_client_cmds.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftdmc -lftm -lconfig -lpthread

LIB_FTDMC=libftdmc
LIB_FTDMC_INCS=ftdm_client.h
LIB_FTDMC_SRCS=ftdm_client.c 
LIB_FTDMC_OBJS=$(LIB_FTDMC_SRCS:.c=.o)

all: $(LIB_FTDMC) $(CLIENT)

$(CLIENT): $(CLIENT_OBJS) $(LIB_FTDMC)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

$(LIB_FTDMC): $(LIB_FTDMC_OBJS)
	$(AR) rcs $(LIB_FTDMC).a $(LIB_FTDMC_OBJS)
	$(CC) -shared -o $(LIB_FTDMC).so.1.0.1 $(LIB_FTDMC_OBJS)

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(SERVER) $(CLIENT) *.a *.o $(LIB_FTDM) $(LIB_FTDMC)
