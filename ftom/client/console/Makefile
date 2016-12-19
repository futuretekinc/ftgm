ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=ftomclient
CLIENT_SRCS=main.c\
			ftom_client_cmds.c\
			cmd_client.c\
			cmd_node.c\
			cmd_ep.c\
			cmd_trigger.c\
			cmd_debug.c\
			cmd_quit.c\
			cmd_trace.c\
			cmd_discovery.c

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_CFLAGS=$(CFLAGS) -I$(TOPDIR)/ftom/netclient
CLIENT_LDFLAGS=$(LDFLAGS) -L$(TOPDIR)/ftom/netclient -lnetclient -lftomclient -lftom -lftm -lcjson -lpthread  -lrt -lm

all: $(CLIENT) 

$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

.c.o:  $(CLIENT_SRCS) *.h
	$(CC) $(CLIENT_CFLAGS) -c $< -o $@

install: $(CLIENT)
	install -d $(EXEC_PREFIX)
	install -t $(EXEC_PREFIX) $(CLIENT)

clean:
	rm $(CLIENT) *.a *.o
