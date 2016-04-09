TOPDIR=../..
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
CLIENT_LDFLAGS=$(LDFLAGS) -lftomc -lftm -lconfig -lpthread 

LIB_FTOMC=libftomc
LIB_FTOMC_INCS= ftom_client.h 
LIB_FTOMC_SRCS= ftom_client.c 
LIB_FTOMC_OBJS=$(LIB_FTOMC_SRCS:.c=.o)

all: tags $(CLIENT) $(LIB_FTOMC) 

tags:
	ctags -R
	
$(CLIENT): $(CLIENT_OBJS) $(LIB_FTOMC)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

$(LIB_FTOMC): $(LIB_FTOMC_OBJS)
	$(AR) rcs $(LIB_FTOMC).a $(LIB_FTOMC_OBJS)
	$(CC) -shared -o $(LIB_FTOMC).so.1.0.1 $(LIB_FTOMC_OBJS) $(LDFLAGS)

.c.o:  $(CLIENT_SRC) $(LIB_FTOMC_OBJS) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(CLIENT) *.a *.o
