ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=ftdm
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard shell/*.c *.c))
BIN_LDFLAGS:=$(LDFLAGS) -lftdm -lftm -lsqlite3 -lcjson -lpthread -ldl -lrt -lm

SUBDIR=lib

include $(TOPDIR)/IncludePost.mk
