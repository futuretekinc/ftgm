ifeq ($(TOPDIR),)
TOPDIR=../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=azclient
CLIENT_SRCS=main.c \
	ftom_azure_client.c\
	ftom_azure_client_shell_cmds.c

CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)
CLIENT_CFLAGS=$(CFLAGS) -std=gnu99 -I$(TOPDIR)/../azure-iot-sdks/c/iothub_client/inc -I$(TOPDIR)/../azure-iot-sdks/c/c-utility/inc
CLIENT_LDFLAGS=$(LDFLAGS) -L./ \
				-L$(TOPDIR)/../azure-iot-sdks/c/build_all/iothub_client \
				-L$(TOPDIR)/../azure-iot-sdks/c/build_all/c-utility \
				-L$(TOPDIR)/../azure-iot-sdks/c/build_all/umqtt \
				-liothub_client -liothub_client_mqtt_transport -lumqtt -laziotsharedutil \
				-lftomcl -lftomc -lftom -lftm \
				-lmosquitto -lcrypto -lssl -lcurl -lcjson -lpthread  -lm -lrt 

all: $(CLIENT)

$(CLIENT): $(CLIENT_OBJS) 
	$(CC)  -o $@ $(CLIENT_OBJS) $(CLIENT_LDFLAGS)

.c.o:  $(LIBTPCLIENT_SRCS) $(CLIENT_SRCS) *.h
	$(CC) $(CLIENT_CFLAGS) $< -o $@

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
