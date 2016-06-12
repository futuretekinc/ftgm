ifeq ($(TOPDIR),)
TOPDIR=$(shell pwd)/../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=ftdmc
CLIENT_SRCS=main.c  ftdm_client_config.c ftdm_client_cmds.c
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftdmc -lftm -lcjson -ldl -lpthread -lrt -lm

all: $(CLIENT)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

.c.o:  *.c *.h
	$(CC) $(CFLAGS) $< -o $@

install: $(CLIENT)
	install -d $(EXEC_PREFIX)
	install -t $(EXEC_PREFIX) $(CLIENT)

clean:
	rm -f $(CLIENT) *.a *.o
