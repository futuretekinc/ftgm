ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=ftomc
CLIENT_SRCS=main.c\
			ftom_client_cmds.c\
			cmd_client.c\
			cmd_node.c\
			cmd_ep.c\
			cmd_debug.c\
			cmd_quit.c\
			cmd_trace.c\
			cmd_discovery.c

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftomc -lftom -lftm -lcjson -lpthread  -lrt -lm

all: $(CLIENT) 

$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

	$(CC) $(CFLAGS) $< -o $@

install: $(CLIENT)
	install -d $(EXEC_PREFIX)
	install -t $(EXEC_PREFIX) $(CLIENT)

clean:
	rm $(CLIENT) *.a *.o
