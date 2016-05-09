TOPDIR=../..
include ${TOPDIR}/Makefile.in

CLIENT=ftdmc
CLIENT_SRCS=main.c  ftdm_client_config.c ftdm_client_cmds.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftdmc -lftm -ldl -lconfig -lpthread

all: $(CLIENT)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(CLIENT) *.a *.o
