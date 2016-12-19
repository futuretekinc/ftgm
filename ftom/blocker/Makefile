ifeq ($(TOPDIR),)
TOPDIR=$(shell pwd)/../..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=ftblocker
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
BIN_CFLAGS:=$(CFLAGS) -std=gnu99 \
		-I/usr/include/azureiot \
		-I/usr/include/azureiot/inc\
		-DARCHITECTURE_x86=1

BIN_LDFLAGS:=$(LDFLAGS) -L./ \
		-L/usr/lib/azureiot \
		-lftomclient\
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

include $(TOPDIR)/IncludePost.mk
