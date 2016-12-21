ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=ftomconsole

BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

BIN_CFLAGS=$(CFLAGS)\
		-I$(TOPDIR)/ftom/netclient

BIN_LDFLAGS=$(LDFLAGS)\
		-L$(TOPDIR)/ftom/netclient\
		-lftomclient\
		-lftom\
		-lftm\
		-lcjson\
		-lpthread\
		-lrt\
		-lm

include $(TOPDIR)/IncludePost.mk
