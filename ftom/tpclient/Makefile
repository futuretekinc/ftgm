ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

LIBTPCLIENT=libtpclient
LIBTPCLIENT_SRCS=ftom_tp_client.c ftom_tp_restapi.c ftom_tp_restapi_gateway.c
LIBTPCLIENT_OBJS=$(LIBTPCLIENT_SRCS:.c=.o)

CLIENT=tpclient
CLIENT_SRCS=main.c \
			ftom_tp_client_shell_cmds.c

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_LDFLAGS=$(LDFLAGS) -L./ -ltpclient -lftom -lftomcl -lftomc -lftm -lmosquitto -lcrypto -lssl -lcurl -lcjson -lpthread  -lm -lrt

all: $(LIBTPCLIENT) $(CLIENT)

$(CLIENT): $(CLIENT_OBJS) $(LIBTPCLIENT)
	$(CC) -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

$(LIBTPCLIENT): $(LIBTPCLIENT_OBJS)
	$(AR) rcs $(LIBTPCLIENT).a $(LIBTPCLIENT_OBJS)
	$(CC) -shared -o $(LIBTPCLIENT).so.1.0.1 $(LIBTPCLIENT_OBJS) $(LDFLAGS)

.c.o:  $(LIBTPCLIENT_SRCS) $(CLIENT_SRCS) *.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(LIBTPCLIENT) $(CLIENT) *.a *.o

install: $(LIBTPCLIENT) $(CLIENT)
	install -d $(EXEC_PREFIX)	
	install -t $(EXEC_PREFIX) $(CLIENT)
	install -d $(LIB_PREFIX)
	install -t $(LIB_PREFIX) $(LIBTPCLIENT).a
	install -t $(LIB_PREFIX) $(LIBTPCLIENT).so.1.0.1
	install -d $(CONF_PREFIX)
	install -t $(CONF_PREFIX) $(CLIENT).conf
	for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done
