ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../..
endif

include $(TOPDIR)/IncludePre.mk

BIN_TARGET=ftdmc
BIN_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
BIN_LDFLAGS:=$(LDFLAGS) -lftm -lcjson -ldl -lpthread -lrt -lm

include $(TOPDIR)/IncludePost.mk
