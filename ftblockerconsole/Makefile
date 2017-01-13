ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=ftblconsole
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard cmds/*.c *.c))
BIN_CFLAGS:=$(CFLAGS) -I./cmds 
BIN_LDFLAGS:=$(LDFLAGS) -lftblocker -lftm -lcjson -ldl -lpthread -lrt -lm

include $(TOPDIR)/IncludePost.mk
