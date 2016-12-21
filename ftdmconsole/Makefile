ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=ftdmconsole
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
BIN_LDFLAGS:=$(LDFLAGS) -lftdm -lftm -lcjson -ldl -lpthread -lrt -lm

include $(TOPDIR)/IncludePost.mk
