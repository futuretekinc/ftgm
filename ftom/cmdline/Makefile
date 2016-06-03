ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

#CFLAGS=$(CFLAGS)\
		-D__malloc_and_calloc_defined=1 \
		-D__need_malloc_and_calloc=1
CLIENT=ftomc
CLIENT_SRCS=main.c

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -L./ -lftomcl -lftomc -lftm -lconfig -lpthread  -lrt

LIB_FTOMCL=libftomcl
LIB_FTOMCL_SRCS= ftom_client_cmdline.c\
			cmd_cgi.c\
			cmd_node.c\
			cmd_ep.c\
			cmd_data.c

LIB_FTOMCL_OBJS=$(LIB_FTOMCL_SRCS:.c=.o)

all: tags $(LIB_FTOMCL) $(CLIENT) 

tags:
	ctags -R
	
$(CLIENT): $(CLIENT_OBJS) $(LIB_FTOMC)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

$(LIB_FTOMCL): $(LIB_FTOMCL_OBJS)
	$(AR) rcs lib$(LIB_FTOMCL).a $(LIB_FTOMCL_OBJS)
	$(CC) -shared -o $(LIB_FTOMCL).so.1.0.1 $(LIB_FTOMCL_OBJS) $(LDFLAGS)

.c.o:  $(CLIENT_SRC) $(LIB_FTOMCL_OBJS) *.h
	$(CC) $(CFLAGS) $< -o $@

install: $(CLIENT) $(LIB_FTOMCL)
	install -d $(EXEC_PREFIX)
	install -t $(EXEC_PREFIX) $(CLIENT)
	install -d $(LIB_PREFIX)
	install -t $(LIB_PREFIX) $(LIB_FTOMCL).a
	install -t $(LIB_PREFIX) $(LIB_FTOMCL).so.1.0.1

clean:
	rm $(CLIENT) *.a *.o
