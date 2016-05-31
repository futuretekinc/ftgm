include $(TOPDIR)/Makefile.in

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1
CLIENT=ftomc
CLIENT_SRCS=main.c\
			ftom_client_config.c\
			ftom_client_cmds.c\
			cmd_node.c\
			cmd_ep.c\
			cmd_debug.c\
			cmd_quit.c\
			cmd_discovery.c

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -lftomc -lftm -lconfig -lpthread  -lrt

all: tags $(CLIENT) 
	echo $(TOPDIR)

tags:
	ctags -R
	
$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

	$(CC) $(CFLAGS) $< -o $@

install: $(CLIENT)
	install -d $(EXEC_PREFIX)
	install -t $(EXEC_PREFIX) $(CLIENT)

clean:
	rm $(CLIENT) *.a *.o
