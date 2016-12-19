ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/..
endif

include $(TOPDIR)/IncludePre.mk

LIB_TARGET=libftm.a 
LIB_OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
CFLAGS:=$(CFLAGS) -DUSER_LITTLE_ENDIAN
LIB_CFLAGS=$(CFLAGS)

include $(TOPDIR)/IncludePost.mk
