ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=azclient
CLIENT_SRCS=main.c \
	ftom_azure_client.c\
	ftom_azure_client_shell_cmds.c


CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_CFLAGS=$(CFLAGS) -std=gnu99 \
				-I/usr/include/azureiot \
				-I/usr/include/azureiot/inc\
				-DARCHITECTURE_x86=1

CLIENT_LDFLAGS=$(LDFLAGS) -L./ \
				-L/usr/lib/azureiot \
				-lftomcl -lftomc -lftom -lftm \
				-liothub_client\
				-liothub_client_mqtt_transport\
				-lumqtt\
				-laziotsharedutil\
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
