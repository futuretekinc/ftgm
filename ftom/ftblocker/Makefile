ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=ftblocker
CLIENT_SRCS=main.c \
	ftom_blocker.c\
	ftom_blocker_cb.c\
	ftom_cloud_client.c\
	ftom_cloud_client_wrapper.c\
	ftom_blocker_shell_cmds.c


CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_CFLAGS=$(CFLAGS) -std=gnu99 \
				-I/usr/include/azureiot \
				-I/usr/include/azureiot/inc\
				-I$(TOPDIR)/ftom/azclient \
				-I$(TOPDIR)/ftom/netclient \
				-DARCHITECTURE_x86=1

CLIENT_LDFLAGS=$(LDFLAGS) -L./ \
				-L$(TOPDIR)/ftom/azclient \
				-L$(TOPDIR)/ftom/netclient \
				-L/usr/lib/azureiot \
				-lnetclient \
				-lazclient \
				-ltpclient \
				-lftomcl -lftomc -lftom -lftm \
				-liothub_client\
				-liothub_client_mqtt_transport\
				-lumqtt\
				-laziotsharedutil\
				-lmosquitto\
				-lpthread\
				-lcurl\
				-lssl\
				-lcrypto\
				-luuid\
				-lcjson\
				-lm\
				-ldl\
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
