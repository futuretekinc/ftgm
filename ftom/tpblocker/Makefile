ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=tpblocker
CLIENT_SRCS=main.c \
	ftom_tp_blocker.c\
	ftom_tp_blocker_shell_cmds.c


CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_CFLAGS=$(CFLAGS) -std=gnu99 \
				-I$(TOPDIR)/ftom/tpclient \
				-I$(TOPDIR)/ftom/netclient \
				-DARCHITECTURE_x86=1

CLIENT_LDFLAGS=$(LDFLAGS) -L./ \
				-L$(TOPDIR)/ftom/tpclient \
				-L$(TOPDIR)/ftom/netclient \
				-ltpclient \
				-lnetclient \
				-lftomcl -lftomc -lftom -lftm \
				-lmosquitto\
				-lpthread\
				-lcurl\
				-lssl\
				-lcrypto\
				-luuid\
				-lcjson\
				-lm\
				-lrt 

all: $(CLIENT)

$(CLIENT): $(CLIENT_OBJS) 
	$(CC)  -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

.c.o:  $(LIBTPCLIENT_SRCS) $(CLIENT_SRCS) *.h
	$(CC) $(CLIENT_CFLAGS) -c $< -o $@

clean:
	rm -f $(LIBTPCLIENT) $(CLIENT) *.a *.o

install: $(LIBTPCLIENT) $(CLIENT)
	install -d $(EXEC_PREFIX)	
	install -t $(EXEC_PREFIX) $(CLIENT)
	install -d $(CONF_PREFIX)
	install -t $(CONF_PREFIX) $(CLIENT).conf
	for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done
