ifeq ($(TOPDIR),)
TOPDIR=$(shell pwd)/../..
endif

include $(TOPDIR)/Makefile.in

CLIENT=ftblocker
SRCS=main.c \
	ftom_blocker.c\
	ftom_blocker_cb.c\
	ftom_blocker_server_client_wrapper.c\
	ftom_blocker_cloud_client_wrapper.c\
	ftom_blocker_shell_cmds.c


OBJS=$(SRCS:.c=.o)
CFLAGS:=$(CFLAGS) -std=gnu99 \
		-I/usr/include/azureiot \
		-I/usr/include/azureiot/inc\
		-DARCHITECTURE_x86=1

LDFLAGS:=$(LDFLAGS) -L./ \
		-L/usr/lib/azureiot \
		-lftomclient \
		-lcloudclient \
		-lmqttclient \
		-lftomclient \
		-lftom \
		-lftm \
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

$(CLIENT): $(OBJS) 
	$(CC)  -o $@ $(OBJS) $(LDFLAGS)

.c.o:  $(SRCS) *.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT) *.a *.o

install: $(CLIENT)
	install -d $(EXEC_PREFIX)	
	install -t $(EXEC_PREFIX) $(CLIENT)
	install -d $(CONF_PREFIX)
	install -t $(CONF_PREFIX) $(CLIENT).conf
	for dir in $(SUBDIR); do\
		make -C $$dir install;\
	done
